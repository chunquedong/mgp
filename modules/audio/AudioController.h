#ifndef AUDIOCONTROLLER_H_
#define AUDIOCONTROLLER_H_

#include <thread>
#include <mutex>
#include <set>

struct ma_engine;

namespace mgp
{

class AudioListener;
class AudioSource;

/**
 * Defines a class for controlling game audio.
 */
class AudioController
{
    friend class Application;
    friend class AudioSource;

public:
    
    /**
     * Destructor.
     */
    virtual ~AudioController();

    static AudioController* cur();

    bool isValid();
private:
    
    /**
     * Constructor.
     */
    AudioController();

    /**
     * Controller initialize.
     */
    void initialize();

    /**
     * Controller finalize.
     */
    void finalize();

    /**
     * Controller pause.
     */
    void pause();

    /**
     * Controller resume.
     */
    void resume();

    /**
     * Controller update.
     */
    void update(float elapsedTime);

    void addPlayingSource(AudioSource* source);
    
    void removePlayingSource(AudioSource* source);

    ma_engine* _engine;
    std::set<AudioSource*> _playingSources;
    std::set<AudioSource*> _streamingSources;
    AudioSource* _pausingSource;
};

}

#endif
