#include "base/Base.h"
#include "audio.h"
#include "AudioController.h"
#include "scene/AudioListener.h"
#include "AudioSource.h"

#include <algorithm>
#include <functional>

namespace mgp
{

static AudioController* g_cur;

AudioController::AudioController() 
: _engine(NULL), _pausingSource(NULL)
{
    g_cur = this;
}

AudioController::~AudioController()
{
    g_cur = NULL;
}

AudioController* AudioController::cur()
{
    return g_cur;
}

bool AudioController::isValid()
{
    return _engine;
}

void AudioController::initialize()
{
    ma_result result;
    ma_engine* pEngine = (ma_engine*)malloc(sizeof(*pEngine));

    result = ma_engine_init(NULL, pEngine);
    if (result != MA_SUCCESS) {
        GP_ERROR("Failed to initialize the audio engine. Error: %d\n", result);
        free(pEngine);
        return;
    }
    _engine = pEngine;
}

void AudioController::finalize()
{
    if (!_engine)
        return;
    ma_engine_uninit(_engine);
    free(_engine);
    _engine = NULL;
}

void AudioController::pause()
{
    if (!_engine)
        return;
    std::set<AudioSource*>::iterator itr = _playingSources.begin();

    // For each source that is playing, pause it.
    AudioSource* source = NULL;
    while (itr != _playingSources.end())
    {
        GP_ASSERT(*itr);
        source = *itr;
        _pausingSource = source;
        source->pause();
        _pausingSource = NULL;
        itr++;
    }
}

void AudioController::resume()
{   
    if (!_engine)
        return;

    std::set<AudioSource*>::iterator itr = _playingSources.begin();

    // For each source that is playing, resume it.
    AudioSource* source = NULL;
    while (itr != _playingSources.end())
    {
        GP_ASSERT(*itr);
        source = *itr;
        source->resume();
        itr++;
    }
}

void AudioController::update(float elapsedTime)
{
    if (!_engine)
        return;
    AudioListener* listener = AudioListener::getInstance();
    if (listener)
    {
        ma_engine_set_gain_db(_engine, listener->getGain());
        const Float* orien = listener->getOrientation();
        ma_engine_listener_set_direction(_engine, 0, orien[0], orien[1], orien[3]);
        ma_engine_listener_set_world_up(_engine, 0, orien[4], orien[5], orien[6]);

        ma_engine_listener_set_velocity(_engine, 0, listener->getVelocity().x, listener->getVelocity().y, listener->getVelocity().z);
        ma_engine_listener_set_position(_engine, 0, listener->getPosition().x, listener->getPosition().y, listener->getPosition().z);
    }
}

void AudioController::addPlayingSource(AudioSource* source)
{
    if (_playingSources.find(source) == _playingSources.end())
    {
        _playingSources.insert(source);
    }
}

void AudioController::removePlayingSource(AudioSource* source)
{
    if (_pausingSource != source)
    {
        std::set<AudioSource*>::iterator iter = _playingSources.find(source);
        if (iter != _playingSources.end())
        {
            _playingSources.erase(iter);
        }
    } 
}

}
