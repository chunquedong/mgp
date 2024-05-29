#include "base/Base.h"
#include "audio.h"
#include "AudioController.h"
#include "scene/AudioListener.h"
#include "AudioBuffer.h"
#include "AudioSource.h"

#include <algorithm>
#include <functional>

namespace mgp
{

static AudioController* g_cur;

AudioController::AudioController() 
: _alcDevice(NULL), _alcContext(NULL), _pausingSource(NULL), _streamingThreadActive(true)
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
    return _alcDevice && _alcContext;
}

void AudioController::initialize()
{
    _alcDevice = alcOpenDevice(NULL);
    if (!_alcDevice)
    {
        GP_WARN("Unable to open OpenAL device.\n");
        return;
    }
    
    _alcContext = alcCreateContext(_alcDevice, NULL);
    ALCenum alcErr = alcGetError(_alcDevice);
    if (!_alcContext || alcErr != ALC_NO_ERROR)
    {
        alcCloseDevice(_alcDevice);
        GP_ERROR("Unable to create OpenAL context. Error: %d\n", alcErr);
        return;
    }
    
    alcMakeContextCurrent(_alcContext);
    alcErr = alcGetError(_alcDevice);
    if (alcErr != ALC_NO_ERROR)
    {
        GP_ERROR("Unable to make OpenAL context current. Error: %d\n", alcErr);
    }
    _streamingMutex.reset(new std::mutex());
}

void AudioController::finalize()
{
    if (!_alcDevice)
        return;
    GP_ASSERT(_streamingSources.empty());
    if (_streamingThread.get())
    {
        _streamingThreadActive = false;
        _streamingThread->join();
        _streamingThread.reset(NULL);
    }

    alcMakeContextCurrent(NULL);
    if (_alcContext)
    {
        alcDestroyContext(_alcContext);
        _alcContext = NULL;
    }
    if (_alcDevice)
    {
        alcCloseDevice(_alcDevice);
        _alcDevice = NULL;
    }
}

void AudioController::pause()
{
    if (!_alcDevice)
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
#ifdef ALC_SOFT_pause_device
    alcDevicePauseSOFT(_alcDevice);
#endif
}

void AudioController::resume()
{   
    if (!_alcDevice)
        return;
    alcMakeContextCurrent(_alcContext);
#ifdef ALC_SOFT_pause_device
    alcDeviceResumeSOFT(_alcDevice);
#endif

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
    if (!_alcDevice)
        return;
    AudioListener* listener = AudioListener::getInstance();
    if (listener)
    {
        AL_CHECK( alListenerf(AL_GAIN, listener->getGain()) );
        ALfloat orientation[3];
        orientation[0] = listener->getOrientation()[0];
        orientation[1] = listener->getOrientation()[1];
        orientation[2] = listener->getOrientation()[2];
        AL_CHECK( alListenerfv(AL_ORIENTATION, orientation) );
        ALfloat velocity[3];
        velocity[0] = listener->getVelocity().x;
        velocity[1] = listener->getVelocity().y;
        velocity[2] = listener->getVelocity().z;
        AL_CHECK( alListenerfv(AL_VELOCITY, velocity) );
        ALfloat position[3];
        position[0] = listener->getPosition().x;
        position[1] = listener->getPosition().y;
        position[2] = listener->getPosition().z;
        AL_CHECK( alListenerfv(AL_POSITION, position) );
    }
}

void AudioController::addPlayingSource(AudioSource* source)
{
    if (_playingSources.find(source) == _playingSources.end())
    {
        _playingSources.insert(source);

        if (source->isStreamed())
        {
            GP_ASSERT(_streamingSources.find(source) == _streamingSources.end());
            bool startThread = _streamingSources.empty() && _streamingThread.get() == NULL;
            _streamingMutex->lock();
            _streamingSources.insert(source);
            _streamingMutex->unlock();

            if (startThread)
                _streamingThread.reset(new std::thread(&streamingThreadProc, this));
        }
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
 
            if (source->isStreamed())
            {
                GP_ASSERT(_streamingSources.find(source) != _streamingSources.end());
                _streamingMutex->lock();
                _streamingSources.erase(source);
                _streamingMutex->unlock();
            }
        }
    } 
}

void AudioController::streamingThreadProc(void* arg)
{
    AudioController* controller = (AudioController*)arg;

    while (controller->_streamingThreadActive)
    {
        controller->_streamingMutex->lock();

        std::for_each(controller->_streamingSources.begin(), controller->_streamingSources.end(), std::mem_fn(&AudioSource::streamDataIfNeeded));
        
        controller->_streamingMutex->unlock();
   
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

}
