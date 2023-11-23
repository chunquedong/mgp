#include "app/Game.h"
#include "Platform.h"
#include "base/FileSystem.h"
#include "render/FrameBuffer.h"
#include "ui/ControlFactory.h"
#include "ui/Theme.h"
#include "ui/Form.h"
#include "render/RenderPath.h"
#include "base/SerializerManager.h"
#include "openGL/GLRenderer.h"
#include "SceneView.h"
#include "base/ThreadPool.h"

#ifndef __EMSCRIPTEN__
#include "script/ScriptController.h"
//#include "net/HttpClient.hpp"
#endif

#define SPLASH_DURATION     2.0f


// Graphics
#define GP_GRAPHICS_WIDTH                           1280
#define GP_GRAPHICS_HEIGHT                          720
#define GP_GRAPHICS_FULLSCREEN                      false
#define GP_GRAPHICS_VSYNC                           true
#define GP_GRAPHICS_MULTISAMPLING                   1
#define GP_GRAPHICS_VALIDATION                      false

#ifndef __EMSCRIPTEN__
/** @script{ignore} */
ALenum __al_error_code = AL_NO_ERROR;
#endif

extern mgp::Renderer* g_rendererInstance;
//extern mgp::ThreadPool* g_threadPool;

namespace mgp
{

static Game* __gameInstance = NULL;
double Game::_pausedTimeLast = 0.0;
double Game::_pausedTimeTotal = 0.0;
double Game::_timeStart = 0.0;


void regiseterSerializer() {
    SerializerManager *mgr = SerializerManager::getActivator();
    mgr->registerType("mgp::Game::Config", Game::Config::createObject);
    mgr->registerType("mgp::Scene", Scene::createObject);
    mgr->registerType("mgp::Node", Node::createObject);
    mgr->registerType("mgp::Camera", Camera::createObject);
    mgr->registerType("mgp::Light", Light::createObject);
    mgr->registerType("mgp::Model", Model::createObject);
    mgr->registerType("mgp::Material", Material::createObject);
    mgr->registerType("mgp::Texture", Texture::createObject);
    mgr->registerType("mgp::MaterialParameter", MaterialParameter::createObject);

    // Register engine enums
    mgr->registerEnum("mgp::Camera::Mode", Camera::enumToString, Camera::enumParse);
    mgr->registerEnum("mgp::Light::Type", Light::enumToString, Light::enumParse);
    mgr->registerEnum("mgp::Light::Mode", Light::enumToString, Light::enumParse);
    mgr->registerEnum("mgp::Light::Shadows", Light::enumToString, Light::enumParse);

    mgr->registerEnum("mgp::Texture::Format", Texture::enumToString, Texture::enumParse);
    mgr->registerEnum("mgp::Texture::Type", Texture::enumToString, Texture::enumParse);
    mgr->registerEnum("mgp::Texture::Wrap", Texture::enumToString, Texture::enumParse);
    mgr->registerEnum("mgp::Texture::Filter", Texture::enumToString, Texture::enumParse);

    mgr->registerEnum("mgp::MaterialParameter::Type", MaterialParameter::enumToString, MaterialParameter::enumParse);
}
#if GP_SCRIPT_ENABLE
/**
* @script{ignore}
*/
class GameScriptTarget : public ScriptTarget
{
    friend class Game;

    GP_SCRIPT_EVENTS_START();
    GP_SCRIPT_EVENT(initialize, "");
    GP_SCRIPT_EVENT(finalize, "");
    GP_SCRIPT_EVENT(update, "f");
    GP_SCRIPT_EVENT(render, "f");
    GP_SCRIPT_EVENT(resizeEvent, "ii");
    GP_SCRIPT_EVENT(keyEvent, "[Keyboard::KeyEvent]i");
    GP_SCRIPT_EVENT(touchEvent, "[MotionEvent::MotionType]iiui");
    GP_SCRIPT_EVENT(mouseEvent, "[MotionEvent::MotionType]iii");
    GP_SCRIPT_EVENT(gestureSwipeEvent, "iii");
    GP_SCRIPT_EVENT(gesturePinchEvent, "iif");
    GP_SCRIPT_EVENT(gestureTapEvent, "ii");
    GP_SCRIPT_EVENT(gestureLongTapevent, "iif");
    GP_SCRIPT_EVENT(gestureDragEvent, "ii");
    GP_SCRIPT_EVENT(gestureDropEvent, "ii");
    GP_SCRIPT_EVENT(gamepadEvent, "[Gamepad::GamepadEvent]<Gamepad>");
    GP_SCRIPT_EVENTS_END();

public:

    GameScriptTarget()
    {
        GP_REGISTER_SCRIPT_EVENTS();
    }

    const char* getTypeName() const
    {
        return "GameScriptTarget";
    }
};
#endif

Game::Game()
    : _initialized(false), _state(UNINITIALIZED), _pausedCount(0),
    _frameLastFPS(0), _frameCount(0), _frameRate(0), _width(0), _height(0),
    _clearDepth(1.0f), _clearStencil(0), _properties(NULL),
    _animationController(NULL), 
    #ifndef __EMSCRIPTEN__
    _audioController(NULL),
    _physicsController(NULL), _aiController(NULL), _audioListener(NULL),
    #endif
    #if GP_SCRIPT_ENABLE
    _scriptController(NULL), _scriptTarget(NULL),
    #endif
    _timeEvents(NULL), 
    _inputListener(NULL), _forms(NULL),
    _showFps(true)
{
    GP_ASSERT(__gameInstance == NULL);

    __gameInstance = this;
    _timeEvents = new std::priority_queue<TimeEvent, std::vector<TimeEvent>, std::less<TimeEvent> >();

    Toolkit::g_instance = this;

    regiseterSerializer();
    setInputListener(this);

    _timeStart = System::nanoTicks() / 1000000.0;

    printf("MGP 1.0\n");
}

Game::~Game()
{
    SerializerManager::releaseStatic();

#if GP_SCRIPT_ENABLE
    SAFE_DELETE(_scriptTarget);
	SAFE_DELETE(_scriptController);
#endif
    // Do not call any virtual functions from the destructor.
    // Finalization is done from outside this class.
    SAFE_DELETE(_timeEvents);

    __gameInstance = NULL;

    //if (g_threadPool) {
    //    g_threadPool->stop();
    //    g_threadPool = NULL;
    //}
#ifdef GP_USE_REF_TRACE
    Refable::printLeaks();
#endif
#ifdef GP_USE_MEM_LEAK_DETECTION
    printMemoryLeaks();
#endif

}

Game* Game::getInstance()
{
    GP_ASSERT(__gameInstance);
    return __gameInstance;
}

void Game::initialize()
{
    // stub
}

void Game::finalize()
{
    // stub
}

void Game::update(float elapsedTime)
{
    for (auto view : _sceneViews) {
        view->update(elapsedTime);
    }
}

void Game::render(float elapsedTime)
{
    for (auto view : _sceneViews) {
        view->render();
    }

    Rectangle* viewport = getView()->getViewport();
    Renderer::cur()->setViewport((int)viewport->x, (int)viewport->y, (int)viewport->width, (int)viewport->height);

    for (int i = 0; i < _forms->getForms().size(); ++i) {
        _forms->getForms()[i]->draw(NULL);
    }

    if (_showFps) {
        drawFps();
    }
}

void Game::drawFps() {
    _font->start();
    Rectangle* viewport = getView()->getViewport();
    int padding = 10;
    int fontSize = 13;
    float y = viewport->height / Toolkit::cur()->getScreenScale() - fontSize - padding;
    float x = 100+padding;
    char buffer[256] = { 0 };
    int drawCall = Renderer::cur()->drawCallCount();
    snprintf(buffer, 256, "FPS:%d, DC:%d", _frameRate, drawCall);
    _font->drawText(buffer, x, y, Vector4::one(), fontSize);
    _font->finish(NULL);
    //Renderer::cur()->drawCallCount();
}

void Game::showFps(bool v) {
    _showFps = v;
}

double Game::getGameTime()
{
    return (System::nanoTicks() / 1000000.0) - _timeStart - _pausedTimeTotal;
}

int Game::run()
{
    if (_state != UNINITIALIZED)
        return -1;

    loadConfig();

    _width = Platform::getDisplayWidth();
    _height = Platform::getDisplayHeight();

    // Start up game systems.
    if (!startup())
    {
        shutdown();
        return -2;
    }

    return 0;
}

bool Game::startup()
{
    if (_state != UNINITIALIZED)
        return false;

    g_rendererInstance = new GLRenderer();
    _sceneViews.push_back(new SceneView());
    _sceneViews[0]->setRenderPath(UPtr<RenderPath>(new RenderPath(Renderer::cur()) ));

    _animationController = new AnimationController();
    _animationController->initialize();

#ifndef __EMSCRIPTEN__
    _audioController = new AudioController();
    _audioController->initialize();

    _physicsController = new PhysicsController();
    _physicsController->initialize();

    _aiController = new AIController();
    _aiController->initialize();

    _audioListener = new AudioListener();
#endif
    _forms = new FormManager();

    _font = Font::create("res/ui/sans.ttf");

    // Load any gamepads, ui or physical.
    //loadGamepads();
#if GP_SCRIPT_ENABLE
    _scriptController = new ScriptController();
    _scriptController->initialize();

    // Set script handler
    if (_properties)
    {
        const char* scriptPath = _properties->getString("script");
        if (scriptPath)
        {
            _scriptTarget = new GameScriptTarget();
            _scriptTarget->addScript(scriptPath);
        }
        else
        {
            // Use the older scripts namespace for loading individual global script callback functions.
            Properties* sns = _properties->getNamespace("scripts", true);
            if (sns)
            {
                _scriptTarget = new GameScriptTarget();

                // Define a macro to simplify defining the following script callback registrations
                #define GP_REG_GAME_SCRIPT_CB(e) if (sns->exists(#e)) _scriptTarget->addScriptCallback(GP_GET_SCRIPT_EVENT(GameScriptTarget, e), sns->getString(#e))

                // Register all supported script callbacks if they are defined
                GP_REG_GAME_SCRIPT_CB(initialize);
                GP_REG_GAME_SCRIPT_CB(finalize);
                GP_REG_GAME_SCRIPT_CB(update);
                GP_REG_GAME_SCRIPT_CB(render);
                GP_REG_GAME_SCRIPT_CB(resizeEvent);
                GP_REG_GAME_SCRIPT_CB(keyEvent);
                GP_REG_GAME_SCRIPT_CB(touchEvent);
                GP_REG_GAME_SCRIPT_CB(mouseEvent);
                GP_REG_GAME_SCRIPT_CB(gestureSwipeEvent);
                GP_REG_GAME_SCRIPT_CB(gesturePinchEvent);
                GP_REG_GAME_SCRIPT_CB(gestureTapEvent);
                GP_REG_GAME_SCRIPT_CB(gestureLongTapevent);
                GP_REG_GAME_SCRIPT_CB(gestureDragEvent);
                GP_REG_GAME_SCRIPT_CB(gestureDropEvent);
                GP_REG_GAME_SCRIPT_CB(gamepadEvent);
            }
        }
    }
#endif
    _state = RUNNING;

    return true;
}

void Game::shutdown()
{
    // Call user finalization.
    if (_state != UNINITIALIZED)
    {
        GP_ASSERT(_animationController);
    #ifndef __EMSCRIPTEN__
        GP_ASSERT(_audioController);
        GP_ASSERT(_physicsController);
        GP_ASSERT(_aiController);
    #endif

        //Platform::signalShutdown();

        clearSchedule();

		// Call user finalize
        finalize();

        for (auto view : _sceneViews) {
            view->finalize();
            SAFE_DELETE(view);
        }
        _sceneViews.clear();

        _forms->finalize();
        SAFE_DELETE(_forms);

    #if GP_SCRIPT_ENABLE
        // Call script finalize
        if (_scriptTarget)
            _scriptTarget->fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(GameScriptTarget, finalize));

        // Destroy script target so no more script events are fired
        SAFE_DELETE(_scriptTarget);

		// Shutdown scripting system first so that any objects allocated in script are released before our subsystems are released
		_scriptController->finalize();

    #endif
    #ifndef __EMSCRIPTEN__
        /*unsigned int gamepadCount = Gamepad::getGamepadCount();
        for (unsigned int i = 0; i < gamepadCount; i++)
        {
            Gamepad* gamepad = Gamepad::getGamepad(i, false);
            SAFE_DELETE(gamepad);
        }*/

        _audioController->finalize();
        SAFE_DELETE(_audioController);

        _physicsController->finalize();
        SAFE_DELETE(_physicsController);
        _aiController->finalize();
        SAFE_DELETE(_aiController);

        SAFE_DELETE(_audioListener);
    #endif
        
        ControlFactory::finalize();

        Theme::finalize();

        // Note: we do not clean up the script controller here
        // because users can call Game::exit() from a script.

        //FrameBuffer::finalize();
        //RenderState::finalize();

        _font.clear();

        SAFE_DELETE(_properties);


        RenderPath::releaseStatic();

        _animationController->finalize();
        SAFE_DELETE(_animationController);

        delete g_rendererInstance;
        g_rendererInstance = NULL;

		_state = UNINITIALIZED;
    }
}

void Game::pause()
{
    if (_state == RUNNING)
    {
        GP_ASSERT(_animationController);
    #ifndef __EMSCRIPTEN__
        GP_ASSERT(_audioController);
        GP_ASSERT(_physicsController);
        GP_ASSERT(_aiController);
    #endif
        _state = PAUSED;
        _pausedTimeLast = System::nanoTicks() / 1000000.0;
        _animationController->pause();
    #ifndef __EMSCRIPTEN__
        _audioController->pause();
        _physicsController->pause();
        _aiController->pause();
    #endif
    }

    ++_pausedCount;
}

void Game::resume()
{
    if (_state == PAUSED)
    {
        --_pausedCount;

        if (_pausedCount == 0)
        {
            GP_ASSERT(_animationController);
        #ifndef __EMSCRIPTEN__
            GP_ASSERT(_audioController);
            GP_ASSERT(_physicsController);
            GP_ASSERT(_aiController);
        #endif
            _state = RUNNING;
            _pausedTimeTotal += System::nanoTicks() / 1000000.0 - _pausedTimeLast;
            _animationController->resume();

        #ifndef __EMSCRIPTEN__
            _audioController->resume();
            _physicsController->resume();
            _aiController->resume();
        #endif
        }
    }
}

void Game::exit()
{
    // Only perform a full/clean shutdown if GP_USE_MEM_LEAK_DETECTION is defined.
	// Every modern OS is able to handle reclaiming process memory hundreds of times
	// faster than it would take us to go through every pointer in the engine and
	// release them nicely. For large games, shutdown can end up taking long time,
    // so we'll just call ::exit(0) to force an instant shutdown.

#ifdef GP_USE_MEM_LEAK_DETECTION

    // Schedule a call to shutdown rather than calling it right away.
	// This handles the case of shutting down the script system from
	// within a script function (which can cause errors).
	static ShutdownListener listener;
	schedule(0, &listener);

#else

    // End the process immediately without a full shutdown
    ::exit(0);

#endif
}


void Game::frame()
{
    if (!_initialized)
    {
        // Perform lazy first time initialization
        initialize();
        #if GP_SCRIPT_ENABLE
        if (_scriptTarget)
            _scriptTarget->fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(GameScriptTarget, initialize));
        #endif
        // Fire first game resize event
        resizeEventInternal(_width, _height);
        _initialized = true;
    }

	static double lastFrameTime = Game::getGameTime();
	double frameTime = getGameTime();

    // Fire time events to scheduled TimeListeners
    fireTimeEvents(frameTime);

    if (_state == Game::RUNNING)
    {
        GP_ASSERT(_animationController);

    #ifndef __EMSCRIPTEN__
        GP_ASSERT(_audioController);
        GP_ASSERT(_physicsController);
        GP_ASSERT(_aiController);
    #endif
        // Update Time.
        float elapsedTime = (frameTime - lastFrameTime);
        lastFrameTime = frameTime;

        // Update the scheduled and running animations.
        _animationController->update(elapsedTime);

    #ifndef __EMSCRIPTEN__
        // Update the physics.
        _physicsController->update(elapsedTime);

        // Update AI.
        _aiController->update(elapsedTime);
    #endif
        // Update gamepads.
        //Gamepad::updateInternal(elapsedTime);

        // Application Update.
        update(elapsedTime);

        // Update forms.
        _forms->updateInternal(elapsedTime);

    #if GP_SCRIPT_ENABLE
        // Run script update.
        if (_scriptTarget)
            _scriptTarget->fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(GameScriptTarget, update), elapsedTime);
    #endif
    #ifndef __EMSCRIPTEN__
        // Audio Rendering.
        _audioController->update(elapsedTime);
    #endif
        // Graphics Rendering.
        render(elapsedTime);

    #if GP_SCRIPT_ENABLE
        // Run script render.
        if (_scriptTarget)
            _scriptTarget->fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(GameScriptTarget, render), elapsedTime);
    #endif

        // Update FPS.
        ++_frameCount;
        if ((Game::getGameTime() - _frameLastFPS) >= 1000)
        {
            _frameRate = _frameCount;
            _frameCount = 0;
            _frameLastFPS = getGameTime();
        }
    }
	else if (_state == Game::PAUSED)
    {
        // Update gamepads.
        //Gamepad::updateInternal(0);

        // Application Update.
        update(0);

        // Update forms.
        _forms->updateInternal(0);

    #if GP_SCRIPT_ENABLE
        // Script update.
        if (_scriptTarget)
            _scriptTarget->fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(GameScriptTarget, update), 0);
    #endif

        // Graphics Rendering.
        render(0);

    #if GP_SCRIPT_ENABLE
        // Script render.
        if (_scriptTarget)
            _scriptTarget->fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(GameScriptTarget, render), 0);
    #endif
    }
}

void Game::keyEvent(Keyboard evt) {
    auto cameraCtrl = getView()->getCameraCtrl();
    if (cameraCtrl) {
        cameraCtrl->keyEvent(evt);
    }
}

bool Game::mouseEvent(Mouse evt) {
    auto cameraCtrl = getView()->getCameraCtrl();
    if (cameraCtrl) {
        if (cameraCtrl->mouseEvent(evt)) return true;
        cameraCtrl->touchEvent(evt);
        return true;
    }
    return false;
}

void Game::resizeEvent(unsigned int width, unsigned int height) {
    Rectangle vp(0.0f, 0.0f, (float)_width, (float)_height);
    getView()->setViewport(&vp);
}

void Game::keyEventInternal(Keyboard evt)
{
    if (_forms->keyEventInternal(evt.evt, evt.key)) {
        return;
    }
    if (_inputListener) _inputListener->keyEvent(evt);

    #if GP_SCRIPT_ENABLE
    if (_scriptTarget)
        _scriptTarget->fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(GameScriptTarget, keyEvent), &evt);
    #endif
}

bool Game::mouseEventInternal(Mouse evt)
{
    if (_forms->mouseEventInternal(evt))
        return true;

    if (_inputListener && _inputListener->mouseEvent(evt))
        return true;

    #if GP_SCRIPT_ENABLE
    if (_scriptTarget)
        return _scriptTarget->fireScriptEvent<bool>(GP_GET_SCRIPT_EVENT(GameScriptTarget, mouseEvent), &evt);
    #endif
    return false;
}

void Game::resizeEventInternal(unsigned int width, unsigned int height)
{
    // Update the width and height of the game
    if (!_initialized || _width != width || _height != height)
    {
        _width = width;
        _height = height;
        if (_inputListener) _inputListener->resizeEvent(width, height);

        #if GP_SCRIPT_ENABLE
        if (_scriptTarget)
            _scriptTarget->fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(GameScriptTarget, resizeEvent), width, height);
        #endif
    }
    _forms->resizeEventInternal(width, height);
}

//void Game::gamepadEventInternal(Gamepad::GamepadEvent evt, Gamepad* gamepad)
//{
//    if (_inputListener) _inputListener->gamepadEvent(evt, gamepad);
//    if (_scriptTarget)
//        _scriptTarget->fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(GameScriptTarget, gamepadEvent), evt, gamepad);
//}

void Game::schedule(float timeOffset, TimeListener* timeListener, void* cookie)
{
    GP_ASSERT(_timeEvents);
    SPtr<TimeListener> listener;
    listener = timeListener;
    TimeEvent timeEvent(getGameTime() + timeOffset, listener, cookie);
    _scheduleLock.lock();
    _timeEvents->push(timeEvent);
    _scheduleLock.unlock();
}

void Game::schedule(float timeOffset, const char* function)
{
    #if undeclared
    _scriptController->schedule(timeOffset, function);
    #endif
}

void Game::clearSchedule()
{
    _scheduleLock.lock();
    SAFE_DELETE(_timeEvents);
    _timeEvents = new std::priority_queue<TimeEvent, std::vector<TimeEvent>, std::less<TimeEvent> >();
    _scheduleLock.unlock();
}

void Game::fireTimeEvents(double frameTime)
{
    std::vector<TimeEvent> toFire;
    _scheduleLock.lock();
    while (_timeEvents->size() > 0)
    {
        const TimeEvent timeEvent = _timeEvents->top();
        if (timeEvent.time > frameTime)
        {
            break;
        }
        //if (timeEvent.listener)
        //{
            toFire.push_back(timeEvent);
        //}
        _timeEvents->pop();
    }
    _scheduleLock.unlock();

    for (auto timeEvent : toFire) {
        SPtr<TimeListener>& listener = timeEvent.listener;
        if (!listener.isNull()) {
            listener->timeEvent(frameTime - timeEvent.time, timeEvent.cookie);
        }
    }
}

Game::TimeEvent::TimeEvent(double time, SPtr<TimeListener> timeListener, void* cookie)
    : time(time), listener(timeListener), cookie(cookie)
{
}

bool Game::TimeEvent::operator<(const TimeEvent& v) const
{
    // The first element of std::priority_queue is the greatest.
    return time > v.time;
}

Properties* Game::getConfig() const
{
    if (_properties == NULL)
        const_cast<Game*>(this)->loadConfig();

    return _properties;
}

void Game::loadConfig()
{
    if (_properties == NULL)
    {
        // Try to load custom config from file.
        if (FileSystem::fileExists("game.config"))
        {
            _properties = Properties::create("game.config").take();

            // Load filesystem aliases.
            Properties* aliases = _properties->getNamespace("aliases", true);
            if (aliases)
            {
                FileSystem::loadResourceAliases(aliases);
            }
        }
        else
        {
            // Create an empty config
            _properties = new Properties();
        }
    }
}

void Game::ShutdownListener::timeEvent(long timeDiff, void* cookie)
{
	Game::getInstance()->shutdown();
}


Game::Config::Config() :
    title(""),
    width(GP_GRAPHICS_WIDTH),
    height(GP_GRAPHICS_HEIGHT),
    fullscreen(GP_GRAPHICS_FULLSCREEN),
    vsync(GP_GRAPHICS_VSYNC),
    multisampling(GP_GRAPHICS_MULTISAMPLING),
    validation(GP_GRAPHICS_VALIDATION),
    homePath(GP_ENGINE_HOME_PATH),
    mainScene("main.scene")
{
}

Game::Config::~Config()
{
}

Serializable* Game::Config::createObject()
{
    return new Game::Config();
}

std::string Game::Config::getClassName()
{
    return "mgp::Game::Config";
}

void Game::Config::onSerialize(Serializer* serializer)
{
    serializer->writeString("title", title.c_str(), "");
    serializer->writeInt("width", width, 0);
    serializer->writeInt("height", height, 0);
    serializer->writeBool("fullscreen", fullscreen, false);
    serializer->writeBool("vsync", vsync, false);
    serializer->writeInt("multisampling", (uint32_t)multisampling, 0);
    serializer->writeBool("validation", validation, false);
    serializer->writeString("homePath", homePath.c_str(), GP_ENGINE_HOME_PATH);
    serializer->writeList("splashScreens", splashScreens.size());
    for (size_t i = 0; i < splashScreens.size(); i++)
    {
        std::string splash = std::string(splashScreens[i].url) + ":" + std::to_string(splashScreens[i].duration);
        serializer->writeString(nullptr, splash.c_str(), "");
    }
    serializer->finishColloction();
    serializer->writeString("mainScene", mainScene.c_str(), "");
}

void Game::Config::onDeserialize(Serializer* serializer)
{
    serializer->readString("title", title, "");
    width = serializer->readInt("width", 0);
    height = serializer->readInt("height", 0);
    fullscreen = serializer->readBool("fullscreen", false);
    vsync = serializer->readBool("vsync", false);
    multisampling = serializer->readInt("multisampling", 0);
    validation = serializer->readBool("validation", false);
    serializer->readString("homePath", homePath, "");
    size_t splashScreensCount = serializer->readList("splashScreens");
    for (size_t i = 0; i < splashScreensCount; i++)
    {
        std::string splash;
        serializer->readString(nullptr, splash, "");
        if (splash.length() > 0)
        {
            SplashScreen splashScreen;
            size_t semiColonIdx = splash.find(':');
            if (semiColonIdx == std::string::npos)
            {
                splashScreen.url = splash;
                splashScreen.duration = SPLASH_DURATION;
            }
            else
            {
                splashScreen.url = splash.substr(0, semiColonIdx);
                std::string durationStr = splash.substr(semiColonIdx + 1, splash.length() - semiColonIdx);
                try
                {
                    splashScreen.duration = std::stof(durationStr);
                }
                catch (...)
                {
                    splashScreen.duration = SPLASH_DURATION;
                }
            }
            splashScreens.push_back(splashScreen);
        }
    }
    serializer->finishColloction();
    serializer->readString("mainScene", mainScene, "");
}

}

