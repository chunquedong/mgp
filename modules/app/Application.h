#ifndef GAME_H_
#define GAME_H_

#include "base/Base.h"
#include "platform/Keyboard.h"
#include "platform/Mouse.h"
//#include "Gamepad.h"
#include "animation/AnimationController.h"
#include "scene/AudioListener.h"
#include "math/Rectangle.h"
#include "math/Vector4.h"
#include "platform/Toolkit.h"
#include "scene/Renderer.h"
#include "base/Serializable.h"
#include "InputListener.h"

#include "EventTimer.h"
#include "Platform.h"
#include "AppConfig.h"

#ifndef __EMSCRIPTEN__
    #include "audio/AudioController.h"
    #include "ai/AIController.h"
#endif



namespace mgp
{

class ScriptController;
class SceneView;
class FormManager;
class Font;
class PhysicsController;

/**
 * Defines the base class your game will extend for game initialization, logic and platform delegates.
 *
 * This represents a running cross-platform game application and provides an abstraction
 * to most typical platform functionality and events.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Game_Config
 */
class Application
{
    friend class Platform;
    //friend class Gamepad;
    friend class ShutdownListener;

public:

    /**
     * The game states.
     */
    enum State
    {
        Uninitialized,
        Initing,
        Runing,
        Paused
    };

    /**
     * Constructor.
     */
    Application();

    /**
     * Destructor.
     */
    virtual ~Application();

    /**
     * Gets the total game time (in milliseconds). This is the total accumulated game time (unpaused).
     *
     * You would typically use things in your game that you want to stop when the game is paused.
     * This includes things such as game physics and animation.
     *
     * @return The total game time (in milliseconds).
     */
    double getGameTime();

    /**
     * Gets the game state.
     *
     * @return The current game state.
     */
    inline State getState() const { return _state; }

    /**
     * Called to initialize the game, and begin running the game.
     *
     * @return Zero for normal termination, or non-zero if an error occurred.
     */
    int run(int w, int h);

    /**
     * Pauses the game after being run.
     */
    void pause();

    /**
     * Resumes the game after being paused.
     */
    void resume();

    /**
     * Exits the game.
     */
    void exit();

    /**
     * Platform frame delegate.
     *
     * This is called every frame from the platform.
     * This in turn calls back on the user implemented game methods: update() then render()
     */
    void frame();

    /**
     * Gets the current frame rate.
     *
     * @return The current frame rate.
     */
    inline unsigned int getFrameRate() const { return _frameRate; }
public:
    /**
     * Gets the game window width.
     *
     * @return The game window width.
     */
    inline unsigned int getWidth() const { return _width; }

    /**
     * Gets the game window height.
     *
     * @return The game window height.
     */
    inline unsigned int getHeight() const { return _height; }


protected:
    /**
     * Keyboard callback on keyPress events.
     *
     * @param evt The key event that occurred.
     * @param key If evt is KEY_PRESS or KEY_RELEASE then key is the key code from Keyboard::Key.
     *            If evt is KEY_CHAR then key is the unicode value of the character.
     *
     * @see Keyboard::KeyEvent
     * @see Keyboard::Key
     */
    virtual bool keyEvent(Keyboard evt);

    /**
     * Mouse callback on mouse events. If the game does not consume the mouse move event or left mouse click event
     * then it is interpreted as a touch event instead.
     *
     * @param evt The mouse event that occurred.
     * @param x The x position of the mouse in pixels. Left edge is zero.
     * @param y The y position of the mouse in pixels. Top edge is zero.
     * @param wheelDelta The number of mouse wheel ticks. Positive is up (forward), negative is down (backward).
     *
     * @return True if the mouse event is consumed or false if it is not consumed.
     *
     * @see MotionEvent::MotionType
     */
    virtual bool mouseEvent(Mouse evt);

    /**
     * Called when the game window has been resized.
     *
     * This method is called once the game window is created with its initial size
     * and then again any time the game window changes size.
     *
     * @param width The new game window width.
     * @param height The new game window height.
     */
    virtual void resizeEvent(unsigned int width, unsigned int height);

protected:

    /**
     * Initialize callback that is called just before the first frame when the game starts.
     */
    virtual void initialize();

    /**
     * Finalize callback that is called when the game on exits.
     */
    virtual void finalize();

    /**
     * Update callback for handling update routines.
     *
     * Called just before render, once per frame when game is running.
     * Ideal for non-render code and game logic such as input and animation.
     *
     * @param elapsedTime The elapsed game time.
     */
    virtual void update(float elapsedTime);

    /**
     * Render callback for handling rendering routines.
     *
     * Called just after update, once per frame when game is running.
     * Ideal for all rendering code.
     *
     * @param elapsedTime The elapsed game time.
     */
    virtual void render(float elapsedTime);

    virtual void onViewRender(SceneView* sceneView);

public:
    std::vector<SceneView*> &getSceneViews() { return _sceneViews; }

    void setInputListener(InputListener* t);

    InputListener* getInputListener() { return _inputListener; }

    SceneView* getView(int i = 0) { return _sceneViews[i]; }

#ifdef GP_UI
    FormManager* getFormManager() { return _forms; }
#endif

    /**
     * Shuts down the game.
     */
    void shutdown();
private:

    struct ShutdownListener : public TimeListener
    {
        void timeEvent(int64_t timeDiff, void* cookie);
    };

    /**
     * Constructor.
     *
     * @param copy The game to copy.
     */
    Application(const Application& copy);

    /**
     * Starts the game.
     */
    bool startup();

    void drawFps();
public:
    void showFps(bool v);

    void notifyKeyEvent(Keyboard evt);
    bool notifyMouseEvent(Mouse evt);
    void notifyResizeEvent(unsigned int width, unsigned int height);

private:
    State _state;                               // The game state.
    unsigned int _pausedCount;                  // Number of times pause() has been called.
    double _pausedTimeLast;                     // The last time paused.
    double _pausedTimeTotal;                    // The total time paused.
    double _frameTimeLastFPS;                       // The last time the frame count was updated.
    unsigned int _frameCount;                   // The current frame count.
    unsigned int _frameRate;                    // The current frame rate.
    unsigned int _width;                        // The game's display width.
    unsigned int _height;                       // The game's display height.
    //Vector4 _clearColor;                        // The clear color value last used for clearing the color buffer.
    //float _clearDepth;                          // The clear depth value last used for clearing the depth buffer.
    //int _clearStencil;                          // The clear stencil value last used for clearing the stencil buffer.

protected:
    AnimationController* _animationController;  // Controls the scheduling and running of animations.
    Renderer* _renderer;

    PhysicsController* _physicsController;      // Controls the simulation of a physics scene and entities.
#ifndef __EMSCRIPTEN__
    AudioController* _audioController;          // Controls audio sources that are playing in the game.
    
    AIController* _aiController;                // Controls AI simulation.
    AudioListener* _audioListener;              // The audio listener in 3D space.
#endif

#if GP_SCRIPT_ENABLE
    ScriptController* _scriptController;        // Controls the scripting engine.
    ScriptTarget* _scriptTarget;                // Script target for the game
#endif


protected:
    InputListener* _inputListener;
protected:

    FormManager* _forms;

    std::vector<SceneView*> _sceneViews;

    UPtr<Font> _font;
    bool _showFps;


    // Note: Do not add STL object member variables on the stack; this will cause false memory leaks to be reported.
    friend class ScreenDisplayer;
};

typedef Application Game;

}

#endif
