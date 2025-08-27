#ifndef AUDIOSOURCE_H_
#define AUDIOSOURCE_H_

#include "math/Vector3.h"
#include "base/Ref.h"
#include "scene/Transform.h"
#include "scene/Component.h"

struct ma_sound;

namespace mgp
{

//class AudioBuffer;
class Node;
class NodeCloneContext;
class AudioController;

/**
 * Defines an audio source in 3D space.
 *
 * This can be attached to a Node for applying its 3D transformation.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Audio
 */
class AudioSource : public Refable, public Component, public Transform::Listener
{
public:

    friend class Node;
    friend class AudioController;

    /**
     * The audio source's audio state.
     */
    enum State
    {
        INITIAL,
        PLAYING,
        PAUSED,
        STOPPED
    };

    /**
     * Create an audio source. This is used to instantiate an Audio Source. Currently only wav, au, and raw files are supported.
     * Alternately, a URL specifying a Properties object that defines an audio source can be used (where the URL is of the format
     * "<file-path>.<extension>#<namespace-id>/<namespace-id>/.../<namespace-id>" and "#<namespace-id>/<namespace-id>/.../<namespace-id>" is optional).
     * 
     * @param url The relative location on disk of the sound file or a URL specifying a Properties object defining an audio source.
     * @param streamed Don't read the entire audio buffer first before playing, instead play immediately from a stream that is read on demand.
     * @return The newly created audio source, or NULL if an audio source cannot be created.
     * @script{create}
     */
    static UPtr<AudioSource> create(const char* url, bool streamed = false);

    /**
     * Plays the audio source.
     */
    void play();

    /**
     * Pauses the playing of the audio source.
     */
    void pause();

    /**
     * Resumes playing of the audio source.
     */
    void resume();

    /**
     * Stops the playing of the audio source.
     */
    void stop();

    /**
     * Rewinds the audio source to the beginning.
     */
    void rewind();

    /**
     * Gets the current state of the audio source.
     *
     * @return PLAYING if the source is playing, STOPPED if the source is stopped, PAUSED if the source is paused and INITIAL otherwise.
     */
    AudioSource::State getState() const;

    /**
     * Determines whether the audio source is streaming or not.
     *
     * @return true if the audio source is streaming, false if not.
     */
    bool isStreamed() const;

    /**
     * Determines whether the audio source is looped or not.
     *
     * @return true if the audio source is looped, false if not.
     */
    bool isLooped() const;

    /**
     * Sets the state of the audio source to be looping or not.
     *
     * @param looped true to loop the sound, false to not loop it.
     */
    void setLooped(bool looped);

    /**
     * Returns the gain of the audio source.
     *
     * @return The gain.
     */
    float getGain() const;

    /**
     * Sets the gain/volume of the audio source.
     *
     * @param gain The gain/volume of the source.
     */
    void setGain(float gain);

    /**
     * Returns the pitch of the audio source.
     *
     * @return The pitch.
     */
    float getPitch() const;

    /**
     * Sets the pitch of the audio source.
     *
     * @param pitch The pitch of the source.
     */
    void setPitch(float pitch);

    /**
     * Gets the velocity of the audio source.
     *
     * @return The velocity as a vector.
     */
    const Vector3& getVelocity() const;

    /**
     * Sets the velocity of the audio source.
     *
     * @param velocity A vector representing the velocity.
     */
    void setVelocity(const Vector3& velocity);

    /**
     * Sets the velocity of the audio source.
     *
     * @param x The x coordinate of the velocity.
     * @param y The y coordinate of the velocity.
     * @param z The z coordinate of the velocity.
     */
    void setVelocity(float x, float y, float z);

    /**
     * Gets the node that this source is attached to.
     * 
     * @return The node that this audio source is attached to.
     */
    Node* getNode() const;

private:

    /**
     * Constructor that takes an AudioBuffer.
     */
    AudioSource(AudioController* ctrl);

    /**
     * Destructor.
     */
    virtual ~AudioSource();

    /**
     * Hidden copy assignment operator.
     */
    AudioSource& operator=(const AudioSource&);

    /**
     * Sets the node for this audio source.
     */
    void setNode(Node* node);

    /**
     * @see Transform::Listener::transformChanged
     */
    void transformChanged(Transform* transform, long cookie);

    /**
     * Clones the audio source and returns a new audio source.
     * 
     * @param context The clone context.
     * @return The newly created audio source.
     */
    AudioSource* clone(NodeCloneContext& context);


    ma_sound* _sound;

    bool _looped;
    float _gain;
    float _pitch;
    Vector3 _velocity;
    //Node* _node;
    State _state;
    AudioController* _audioController;
};

}

#endif
