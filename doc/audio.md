

## 声音

目前只支持WAV和OGG两种格式

```
class MainApp : public Application {
    UPtr<AudioSource> _audioEngine;

    void initialize() {

        _audioEngine = AudioSource::create("res/audio/engine_loop.ogg", false);
        _audioEngine->setLooped(true);
        _audioEngine->play();
    }

    void finalize() {
        _audioEngine.clear();
    }
};
```
