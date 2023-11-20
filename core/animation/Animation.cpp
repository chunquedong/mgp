#include "base/Base.h"
#include "Animation.h"
#include "AnimationController.h"
#include "AnimationClip.h"
#include "AnimationTarget.h"
#include "platform/Toolkit.h"
#include "scene/Transform.h"
#include "base/Properties.h"
#include "scene/Node.h"

#define ANIMATION_INDEFINITE_STR "INDEFINITE"
#define ANIMATION_DEFAULT_CLIP 0
#define ANIMATION_ROTATE_OFFSET 0
#define ANIMATION_SRT_OFFSET 3

namespace mgp
{

Animation::Animation(const char* id, AnimationTarget* target, int propertyId, unsigned int keyCount, unsigned int* keyTimes, float* keyValues, unsigned int type)
    : _controller(AnimationController::cur()), _id(id), _duration(0L), _defaultClip(NULL), _clips(NULL)
{
    createChannel(target, propertyId, keyCount, keyTimes, keyValues, type);

    // Release the animation because a newly created animation has a ref count of 1 and the channels hold the ref to animation.
    release();
    GP_ASSERT(getRefCount() == 1);
}

Animation::Animation(const char* id, AnimationTarget* target, int propertyId, unsigned int keyCount, unsigned int* keyTimes, float* keyValues, float* keyInValue, float* keyOutValue, unsigned int type)
    : _controller(AnimationController::cur()), _id(id), _duration(0L), _defaultClip(NULL), _clips(NULL)
{
    createChannel(target, propertyId, keyCount, keyTimes, keyValues, keyInValue, keyOutValue, type);
    // Release the animation because a newly created animation has a ref count of 1 and the channels hold the ref to animation.
    release();
    GP_ASSERT(getRefCount() == 1);
}

Animation::Animation(const char* id)
    : _controller(AnimationController::cur()), _id(id), _duration(0L), _defaultClip(NULL), _clips(NULL)
{
}

Animation::~Animation()
{
    _channels.clear();

    if (_defaultClip)
    {
        if (_defaultClip->isClipStateBitSet(AnimationClip::CLIP_IS_PLAYING_BIT))
        {
            GP_ASSERT(_controller);
            _controller->unschedule(_defaultClip);
        }
        SAFE_RELEASE(_defaultClip);
    }

    if (_clips)
    {
        std::vector<AnimationClip*>::iterator clipIter = _clips->begin();

        while (clipIter != _clips->end())
        {
            AnimationClip* clip = *clipIter;
            GP_ASSERT(clip);
            if (clip->isClipStateBitSet(AnimationClip::CLIP_IS_PLAYING_BIT))
            {
                GP_ASSERT(_controller);
                _controller->unschedule(clip);
            }
            SAFE_RELEASE(clip);
            clipIter++;
        }
        _clips->clear();
    }
    SAFE_DELETE(_clips);
}


const char* Animation::getName() const
{
    return _id.c_str();
}

unsigned long Animation::getDuration() const
{
    return _duration;
}

void Animation::createClips(const char* url)
{
    UPtr<Properties> properties = Properties::create(url);
    GP_ASSERT(properties.get());

    Properties* pAnimation = (strlen(properties->getNamespace()) > 0) ? properties.get() : properties->getNextNamespace();
    GP_ASSERT(pAnimation);

    int frameCount = pAnimation->getInt("frameCount");
    if (frameCount <= 0)
        GP_ERROR("The animation's frame count must be greater than 0.");

    createClips(pAnimation, (unsigned int)frameCount);

    //SAFE_DELETE(properties);
}

AnimationClip* Animation::createClip(const char* id, unsigned long begin, unsigned long end)
{
    AnimationClip* clip = new AnimationClip(id, this, begin, end);
    addClip(clip);
    return clip;
}

AnimationClip* Animation::getClip(const char* id)
{
    // If id is NULL return the default clip.
    if (id == NULL)
    {
        if (_defaultClip == NULL)
            createDefaultClip();

        return _defaultClip;
    }
    else
    {
        return findClip(id);
    }
}

AnimationClip* Animation::getClip(unsigned int index) const
{
    if (_clips)
        return _clips->at(index);

    return NULL;
}

unsigned int Animation::getClipCount() const
{
    return _clips ? (unsigned int)_clips->size() : 0;
}

void Animation::play(const char* clipId)
{
    // If id is NULL, play the default clip.
    if (clipId == NULL)
    {
        if (_defaultClip == NULL)
            createDefaultClip();

        _defaultClip->play();
    }
    else
    {
        // Find animation clip and play.
        AnimationClip* clip = findClip(clipId);
        if (clip != NULL)
            clip->play();
    }
}

void Animation::stop(const char* clipId)
{
    // If id is NULL, play the default clip.
    if (clipId == NULL)
    {
        if (_defaultClip)
            _defaultClip->stop();
    }
    else
    {
        // Find animation clip and play.
        AnimationClip* clip = findClip(clipId);
        if (clip != NULL)
            clip->stop();
    }
}

void Animation::pause(const char * clipId)
{
    if (clipId == NULL)
    {
        if (_defaultClip)
            _defaultClip->pause();
    }
    else
    {
        AnimationClip* clip = findClip(clipId);
        if (clip != NULL)
            clip->pause();
    }
}

bool Animation::targets(AnimationTarget* target) const
{
    for (std::vector<AnimationChannel*>::const_iterator itr = _channels.begin(); itr != _channels.end(); ++itr)
    {
        GP_ASSERT(*itr);
        if ((*itr)->getTarget() == target)
        {
            return true;
        }
    }
    return false;
}


void Animation::createDefaultClip()
{
    _defaultClip = new AnimationClip("default_clip", this, 0.0f, _duration);
}

void Animation::createClips(Properties* animationProperties, unsigned int frameCount)
{
    GP_ASSERT(animationProperties);

    Properties* pClip = animationProperties->getNextNamespace();

    while (pClip != NULL && std::strcmp(pClip->getNamespace(), "clip") == 0)
    {
        int begin = pClip->getInt("begin");
        int end = pClip->getInt("end");

        AnimationClip* clip = createClip(pClip->getId(), ((float)begin / frameCount) * _duration, ((float)end / frameCount) * _duration);

        const char* repeat = pClip->getString("repeatCount");
        if (repeat)
        {
            if (strcmp(repeat, ANIMATION_INDEFINITE_STR) == 0)
            {
                clip->setRepeatCount(AnimationClip::REPEAT_INDEFINITE);
            }
            else
            {
                float value;
                sscanf(repeat, "%f", &value);
                clip->setRepeatCount(value);
            }
        }

        const char* speed = pClip->getString("speed");
        if (speed)
        {
            float value;
            sscanf(speed, "%f", &value);
            clip->setSpeed(value);
        }

        clip->setLoopBlendTime(pClip->getFloat("loopBlendTime")); // returns zero if not specified

        pClip = animationProperties->getNextNamespace();
    }
}

void Animation::addClip(AnimationClip* clip)
{
    if (_clips == NULL)
        _clips = new std::vector<AnimationClip*>;

    GP_ASSERT(clip);
    _clips->push_back(clip);
}

AnimationClip* Animation::findClip(const char* id) const
{
    if (_clips)
    {
        size_t clipCount = _clips->size();
        for (size_t i = 0; i < clipCount; i++)
        {
            AnimationClip* clip = _clips->at(i);
            GP_ASSERT(clip);
            if (clip->_id.compare(id) == 0)
            {
                return clip;
            }
        }
    }
    return NULL;
}


void Animation::addChannel(AnimationChannel* channel)
{
    GP_ASSERT(channel);
    _channels.push_back(channel);

    if (channel->getDuration() > _duration)
        _duration = channel->getDuration();
}

void Animation::removeChannel(AnimationChannel* channel)
{
    std::vector<AnimationChannel*>::iterator itr = _channels.begin();
    while (itr != _channels.end())
    {
        AnimationChannel* chan = *itr;
        if (channel == chan)
        {
            _channels.erase(itr);
            return;
        }
        else
        {
            itr++;
        }
    }
}

void Animation::setTransformRotationOffset(Curve* curve, unsigned int propertyId)
{
    GP_ASSERT(curve);

    switch (propertyId)
    {
    case Transform::ANIMATE_ROTATE:
    case Transform::ANIMATE_ROTATE_TRANSLATE:
        curve->setQuaternionOffset(ANIMATION_ROTATE_OFFSET);
        return;
    case Transform::ANIMATE_SCALE_ROTATE:
    case Transform::ANIMATE_SCALE_ROTATE_TRANSLATE:
        curve->setQuaternionOffset(ANIMATION_SRT_OFFSET);
        return;
    }

    return;
}

Animation* Animation::clone(AnimationChannel* channel, AnimationTarget* target)
{
    GP_ASSERT(channel);

    Animation* animation = new Animation(getName());

    AnimationChannel* channelCopy = new KeyframeChannel(*dynamic_cast<KeyframeChannel*>(channel), animation, target);
    animation->addChannel(channelCopy);
    // Release the animation because a newly created animation has a ref count of 1 and the channels hold the ref to animation.
    animation->release();
    GP_ASSERT(animation->getRefCount() == 1);

    // Clone the clips
    
    if (_defaultClip)
    {
        animation->_defaultClip = _defaultClip->clone(animation);
    }
    
    if (_clips)
    {
        for (std::vector<AnimationClip*>::iterator it = _clips->begin(); it != _clips->end(); ++it)
        {
            AnimationClip* newClip = (*it)->clone(animation);
            animation->addClip(newClip);
        }
    }
    return animation;
}

void Animation::write(Stream* file) {
    file->writeStr(_id);
    file->writeUInt64(_duration);

    int channelCount = 0;
    for (AnimationChannel* ac : _channels) {
        KeyframeChannel* c = dynamic_cast<KeyframeChannel*>(ac);
        if (!c) continue;
        ++channelCount;
    }
    file->writeUInt16(channelCount);

    for (AnimationChannel* ac : _channels) {
        KeyframeChannel* c = dynamic_cast<KeyframeChannel*>(ac);
        if (!c) continue;
        Node* node = dynamic_cast<Node*>(c->_target);
        if (node != NULL) {
            file->writeStr(node->getName());
        }
        else {
            file->writeStr("");
        }

        file->writeUInt16(c->_propertyId);
        file->writeUInt64(c->_duration);

        
        file->writeUInt32(c->_curve->getPointCount());
        file->writeUInt8(c->_curve->_componentCount);
        for (int i = 0; i < c->_curve->getPointCount(); i++) {
            file->writeFloat(c->_curve->_points->time);
            file->writeFloat(*c->_curve->_points->value);
            file->writeUInt8(c->_curve->_points->type);
        }
    }
}
bool Animation::read(Stream* file) {
    std::string id = file->readStr();
    Animation* a = this;
    a->_id = id;
    a->_duration = file->readUInt64();
    int channelCount = file->readUInt16();
    for (int i = 0; i < channelCount; ++i) {
        KeyframeChannel* c = new KeyframeChannel();
        c->_targetId = file->readStr();
        c->_propertyId = file->readUInt16();
        c->_duration = file->readUInt64();
        
        int keyCount = file->readUInt32();
        int propertyComponentCount = file->readUInt16();
        int propertyComponentSize = file->readUInt16();
        Curve* curve = Curve::create(keyCount, propertyComponentCount).take();
        c->_curve = curve;
        for (int i = 0; i < keyCount; i++) {
            float time = file->readFloat();
            float value = file->readFloat();
            int type = file->readUInt8();
            curve->setPoint(i, time, &value, (Curve::InterpolationType)type);
        }
        a->_channels.push_back(c);
    }
    return true;
}

void Animation::update(float percentComplete, unsigned int clipStart, unsigned int clipEnd, unsigned int loopBlendTime, float blendWeight) {
    size_t channelCount = _channels.size();
    float percentageStart = (float)clipStart / (float)_duration;
    float percentageEnd = (float)clipEnd / (float)_duration;
    float percentageBlend = (float)loopBlendTime / (float)_duration;

    for (size_t i = 0; i < channelCount; i++)
    {
        AnimationChannel* channel = _channels[i];
        channel->update(percentComplete, percentageStart, percentageEnd, percentageBlend, blendWeight);
    }
}

AnimationChannel* Animation::createChannel(AnimationTarget* target, int propertyId, unsigned int keyCount, unsigned int* keyTimes, float* keyValues, unsigned int type)
{
    GP_ASSERT(target);
    GP_ASSERT(keyTimes);
    GP_ASSERT(keyValues);

    unsigned int propertyComponentCount = target->getAnimationPropertyComponentCount(propertyId);
    GP_ASSERT(propertyComponentCount > 0);

    UPtr<Curve> curve = Curve::create(keyCount, propertyComponentCount);
    GP_ASSERT(curve.get());
    if (target->_targetType == AnimationTarget::TRANSFORM)
        setTransformRotationOffset(curve.get(), propertyId);

    unsigned int lowest = keyTimes[0];
    unsigned long duration = keyTimes[keyCount - 1] - lowest;

    float* normalizedKeyTimes = new float[keyCount];

    normalizedKeyTimes[0] = 0.0f;
    curve->setPoint(0, normalizedKeyTimes[0], keyValues, (Curve::InterpolationType)type);

    unsigned int pointOffset = propertyComponentCount;
    unsigned int i = 1;
    for (; i < keyCount - 1; i++)
    {
        normalizedKeyTimes[i] = (float)(keyTimes[i] - lowest) / (float)duration;
        curve->setPoint(i, normalizedKeyTimes[i], (keyValues + pointOffset), (Curve::InterpolationType)type);
        pointOffset += propertyComponentCount;
    }
    if (keyCount > 1) {
        i = keyCount - 1;
        normalizedKeyTimes[i] = 1.0f;
        curve->setPoint(i, normalizedKeyTimes[i], keyValues + pointOffset, (Curve::InterpolationType)type);
    }

    SAFE_DELETE_ARRAY(normalizedKeyTimes);

    AnimationChannel* channel = new KeyframeChannel(this, target, propertyId, curve.get(), duration);
    //curve->release();
    addChannel(channel);
    return channel;
}

AnimationChannel* Animation::createChannel(AnimationTarget* target, int propertyId, unsigned int keyCount, unsigned int* keyTimes, float* keyValues, float* keyInValue, float* keyOutValue, unsigned int type)
{
    GP_ASSERT(target);
    GP_ASSERT(keyTimes);
    GP_ASSERT(keyValues);

    unsigned int propertyComponentCount = target->getAnimationPropertyComponentCount(propertyId);
    GP_ASSERT(propertyComponentCount > 0);

    UPtr<Curve> curve = Curve::create(keyCount, propertyComponentCount);
    GP_ASSERT(curve.get());
    if (target->_targetType == AnimationTarget::TRANSFORM)
        setTransformRotationOffset(curve.get(), propertyId);

    unsigned long lowest = keyTimes[0];
    unsigned long duration = keyTimes[keyCount - 1] - lowest;

    float* normalizedKeyTimes = new float[keyCount];

    normalizedKeyTimes[0] = 0.0f;
    curve->setPoint(0, normalizedKeyTimes[0], keyValues, (Curve::InterpolationType)type, keyInValue, keyOutValue);

    unsigned int pointOffset = propertyComponentCount;
    unsigned int i = 1;
    for (; i < keyCount - 1; i++)
    {
        normalizedKeyTimes[i] = (float)(keyTimes[i] - lowest) / (float)duration;
        curve->setPoint(i, normalizedKeyTimes[i], (keyValues + pointOffset), (Curve::InterpolationType)type, (keyInValue + pointOffset), (keyOutValue + pointOffset));
        pointOffset += propertyComponentCount;
    }
    i = keyCount - 1;
    normalizedKeyTimes[i] = 1.0f;
    curve->setPoint(i, normalizedKeyTimes[i], keyValues + pointOffset, (Curve::InterpolationType)type, keyInValue + pointOffset, keyOutValue + pointOffset);

    SAFE_DELETE_ARRAY(normalizedKeyTimes);

    AnimationChannel* channel = new KeyframeChannel(this, target, propertyId, curve.get(), duration);
    //curve->release();
    addChannel(channel);
    return channel;
}


KeyframeChannel::KeyframeChannel()
    : _animation(NULL), _target(NULL), _propertyId(0), _curve(NULL), _duration(0), _value(NULL)
{

}

KeyframeChannel::KeyframeChannel(Animation* animation, AnimationTarget* target, int propertyId, Curve* curve, unsigned long duration)
    : _animation(animation), _target(target), _propertyId(propertyId), _curve(curve), _duration(duration), _value(NULL)
{
    GP_ASSERT(_animation);
    GP_ASSERT(_target);
    GP_ASSERT(_curve);

    // get property component count, and ensure the property exists on the AnimationTarget by getting the property component count.
    GP_ASSERT(_target->getAnimationPropertyComponentCount(propertyId));
    _curve->addRef();
    _target->addChannel(this);
    _animation->addRef();
}

KeyframeChannel::KeyframeChannel(const KeyframeChannel& copy, Animation* animation, AnimationTarget* target)
    : _animation(animation), _target(target), _propertyId(copy._propertyId), _curve(copy._curve), _duration(copy._duration), _value(NULL)
{
    GP_ASSERT(_curve);
    GP_ASSERT(_target);
    GP_ASSERT(_animation);

    _curve->addRef();
    _target->addChannel(this);
    _animation->addRef();
}

KeyframeChannel::~KeyframeChannel()
{
    SAFE_RELEASE(_curve);
    SAFE_RELEASE(_animation);
    SAFE_DELETE(_value);
}

Curve* KeyframeChannel::getCurve() const
{
    return _curve;
}


void KeyframeChannel::update(float percentComplete, float clipStart, float clipEnd, float loopBlendTime, float blendWeight) {

    GP_ASSERT(this);
    AnimationTarget* target = _target;
    GP_ASSERT(target);

    if (!_value) {
        _value = new AnimationValue(getCurve()->getComponentCount());
    }

    // Evaluate the point on Curve
    GP_ASSERT(this->getCurve());
    this->getCurve()->evaluate(percentComplete, clipStart, clipEnd, loopBlendTime, _value->_value);

    // Set the animation value on the target property.
    target->setAnimationPropertyValue(this->_propertyId, _value, blendWeight);
}




}
