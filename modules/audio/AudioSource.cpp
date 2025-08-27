#include "base/Base.h"
#include "audio.h"
#include "scene/Node.h"
//#include "AudioBuffer.h"
#include "AudioController.h"
#include "AudioSource.h"
#include "platform/Toolkit.h"

#include "stb_vorbis.h"

namespace mgp
{

AudioSource::AudioSource(AudioController* ctrl) 
    : _state(INITIAL), _looped(false), _gain(1.0f), _pitch(1.0f), _audioController(ctrl), _sound(NULL)
{
    _sound = (ma_sound*)malloc(sizeof(ma_sound));
}

AudioSource::~AudioSource()
{
    ma_sound_uninit(_sound);
    free(_sound);
    _sound = NULL;
}

UPtr<AudioSource> AudioSource::create(const char* url, bool streamed)
{
    AudioController* audioController = AudioController::cur();
    GP_ASSERT(audioController);
    AudioSource* audioSource = new AudioSource(audioController);

    ma_result result = ma_sound_init_from_file(audioController->_engine, url, 0, NULL, NULL, audioSource->_sound);
    if (result != MA_SUCCESS) {
        GP_ERROR("Failed to load audio source %s.", url);
        delete audioSource;
        return UPtr<AudioSource>();
    }
    
    return UPtr<AudioSource>(audioSource);
}

AudioSource::State AudioSource::getState() const
{
    if (!ma_sound_is_playing(_sound)) {
        if (_state == PLAYING) {
            return PAUSED;
        }
    }

    return _state;
}

bool AudioSource::isStreamed() const
{
    return false;
}

void AudioSource::play()
{
    ma_sound_start(_sound);
    _state = PLAYING;

    // Add the source to the controller's list of currently playing sources.
    _audioController->addPlayingSource(this);
}

void AudioSource::pause()
{
    ma_sound_stop(_sound);
    _state = PAUSED;

    // Remove the source from the controller's set of currently playing sources
    // if the source is being paused by the user and not the controller itself.
    _audioController->removePlayingSource(this);
}

void AudioSource::resume()
{
    if (getState() == PAUSED)
    {
        play();
    }
}

void AudioSource::stop()
{
    ma_sound_stop(_sound);
    ma_sound_seek_to_pcm_frame(_sound, 0);
    _state = STOPPED;

    // Remove the source from the controller's set of currently playing sources.
    _audioController->removePlayingSource(this);
}

void AudioSource::rewind()
{
    ma_sound_seek_to_pcm_frame(_sound, 0);
}

bool AudioSource::isLooped() const
{
    return _looped;
}

void AudioSource::setLooped(bool looped)
{
    ma_sound_set_looping(_sound, looped);
    _looped = looped;
}

float AudioSource::getGain() const
{
    return _gain;
}

void AudioSource::setGain(float gain)
{
    ma_sound_set_min_gain(_sound, gain);
    ma_sound_set_max_gain(_sound, gain);
    _gain = gain;
}

float AudioSource::getPitch() const
{
    return _pitch;
}

void AudioSource::setPitch(float pitch)
{
    ma_sound_set_pitch(_sound, pitch);
    _pitch = pitch;
}

const Vector3& AudioSource::getVelocity() const
{
    return _velocity;
}

void AudioSource::setVelocity(const Vector3& velocity)
{
    ma_sound_set_velocity(_sound, velocity.x, velocity.y, velocity.z);
    _velocity = velocity;
}

void AudioSource::setVelocity(float x, float y, float z)
{
    setVelocity(Vector3(x, y, z));
}

Node* AudioSource::getNode() const
{
    return _node;
}

void AudioSource::setNode(Node* node)
{
    if (_node != node)
    {
        // Disconnect our current transform.
        if (_node)
        {
            _node->removeListener(this);
        }

        // Connect the new node.
        _node = node;

        if (_node)
        {
            _node->addListener(this);
            // Update the audio source position.
            transformChanged(_node, 0);
        }
    }
}

void AudioSource::transformChanged(Transform* transform, long cookie)
{
    if (_node)
    {
        Vector3 translation = _node->getTranslationWorld();
        ma_sound_set_position(_sound, translation.x, translation.y, translation.z);
    }
}

AudioSource* AudioSource::clone(NodeCloneContext& context)
{
    AudioSource* audioClone = new AudioSource(_audioController);
    ma_sound_init_copy(_audioController->_engine, _sound, 0, NULL, audioClone->_sound);
  
    audioClone->setLooped(isLooped());
    audioClone->setGain(getGain());
    audioClone->setPitch(getPitch());
    audioClone->setVelocity(getVelocity());
    if (Node* node = getNode())
    {
        Node* clonedNode = context.findClonedNode(node);
        if (clonedNode)
        {
            audioClone->setNode(clonedNode);
        }
    }
    return audioClone;
}

}
