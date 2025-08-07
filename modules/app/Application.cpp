#include "app/Application.h"
#include "Platform.h"
#include "base/FileSystem.h"
#include "render/FrameBuffer.h"

#ifdef GP_UI
    #include "ui/Theme.h"
    #include "ui/FormManager.h"
#endif

#include "render/RenderPath.h"
#include "base/SerializerManager.h"
#include "openGL/GLRenderer.h"
#include "SceneView.h"
#include "base/ThreadPool.h"
#include "objects/Terrain.h"
#include "openGL/CompressedTexture.h"
#include "scene/AssetManager.h"
#include "objects/Font.h"

#ifdef WASE_UI
    #include "waseUI/WaseWindow.h"
#endif

#ifndef __EMSCRIPTEN__
#include "script/ScriptController.h"
//#include "net/HttpClient.hpp"
#endif


#ifndef __EMSCRIPTEN__
/** @script{ignore} */
ALenum __al_error_code = AL_NO_ERROR;
#endif

extern mgp::Renderer* g_rendererInstance;
//extern mgp::ThreadPool* g_threadPool;
extern mgp::CompressedTexture* g_compressedTexture;

namespace mgp
{

int g_appInstanceCount = 0;

void regiseterSerializer() {
    SerializerManager *mgr = SerializerManager::getActivator();
    mgr->registerType("mgp::Application::Config", AppConfig::createObject);
    mgr->registerType("mgp::Scene", Scene::createObject);
    mgr->registerType("mgp::Node", Node::createObject);
    mgr->registerType("mgp::Camera", Camera::createObject);
    mgr->registerType("mgp::Light", Light::createObject);
    mgr->registerType("mgp::Model", Model::createObject);
    mgr->registerType("mgp::Material", Material::createObject);
    mgr->registerType("mgp::Texture", Texture::createObject);
    mgr->registerType("mgp::MaterialParameter", MaterialParameter::createObject);
    mgr->registerType("mgp::Terrain", Terrain::createObject);
    mgr->registerType("mgp::Terrain::Layer", Terrain::createLayerObject);

#ifdef GP_UI
    FormManager::regiseterSerializer(mgr);
#endif

    // Register engine enums
    mgr->registerEnum("mgp::Camera::Mode", Camera::enumToString, Camera::enumParse);
    mgr->registerEnum("mgp::Light::Type", Light::enumToString, Light::enumParse);
    mgr->registerEnum("mgp::Light::Mode", Light::enumToString, Light::enumParse);
    mgr->registerEnum("mgp::Light::Shadows", Light::enumToString, Light::enumParse);

    mgr->registerEnum("mgp::Image::Format", Texture::enumToString, Texture::enumParse);
    mgr->registerEnum("mgp::Texture::Type", Texture::enumToString, Texture::enumParse);
    mgr->registerEnum("mgp::Texture::Wrap", Texture::enumToString, Texture::enumParse);
    mgr->registerEnum("mgp::Texture::Filter", Texture::enumToString, Texture::enumParse);

    mgr->registerEnum("mgp::MaterialParameter::Type", MaterialParameter::enumToString, MaterialParameter::enumParse);
    mgr->registerEnum("mgp::StateBlock::DepthFunction", StateBlock::enumToString, StateBlock::enumParse);
    mgr->registerEnum("mgp::StateBlock::Blend", StateBlock::enumToString, StateBlock::enumParse);
    mgr->registerEnum("mgp::StateBlock::CullFaceSide", StateBlock::enumToString, StateBlock::enumParse);
    mgr->registerEnum("mgp::StateBlock::FrontFace", StateBlock::enumToString, StateBlock::enumParse);
    mgr->registerEnum("mgp::StateBlock::StencilOperation", StateBlock::enumToString, StateBlock::enumParse);
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

Application::Application()
    : _state(Uninitialized), _pausedCount(0),
    _frameTimeLastFPS(0), _frameCount(0), _frameRate(0), _width(0), _height(0),
    //_clearDepth(1.0f), _clearStencil(0),
    _animationController(NULL), _renderer(NULL), _physicsController(NULL), 
    #ifndef __EMSCRIPTEN__
        _audioController(NULL),
        _aiController(NULL), _audioListener(NULL),
    #endif
    #if GP_SCRIPT_ENABLE
        _scriptController(NULL), _scriptTarget(NULL),
    #endif
    _inputListener(NULL),
    #ifdef GP_UI
        _forms(NULL),
    #endif
    _pausedTimeLast(0), _pausedTimeTotal(0),
    _showFps(true)
{
    ++g_appInstanceCount;

    regiseterSerializer();

    g_compressedTexture = new GLCompressedTexture();

    printf("MGP 1.0\n");
}

Application::~Application()
{
    --g_appInstanceCount;

    SerializerManager::releaseStatic();

#if GP_SCRIPT_ENABLE
    SAFE_DELETE(_scriptTarget);
	SAFE_DELETE(_scriptController);
#endif
    // Do not call any virtual functions from the destructor.
    // Finalization is done from outside this class.

    delete g_compressedTexture;
    g_compressedTexture = NULL;

    //if (g_threadPool) {
    //    g_threadPool->stop();
    //    g_threadPool = NULL;
    //}

    if (g_appInstanceCount == 0) {
#ifdef GP_USE_REF_TRACE
        Refable::printLeaks();
#endif
#ifdef GP_USE_MEM_LEAK_DETECTION
        printMemoryLeaks();
#endif
    }
}

void Application::initialize()
{
    // stub
}

void Application::finalize()
{
    // stub
}

void Application::update(float elapsedTime)
{
    for (auto view : _sceneViews) {
        view->update(elapsedTime);
    }
}

void Application::onViewRender(SceneView* view) {
    view->render();
}

void Application::render(float elapsedTime)
{
    for (auto view : _sceneViews) {
        onViewRender(view);
    }

    //Rectangle* viewport = getView()->getViewport();
    _renderer->setViewport(0, 0, _width, _height);

#ifdef GP_UI
    _forms->draw(NULL);
#endif

#ifdef WASE_UI
    waseUI::doFrame();
#endif

    if (_showFps) {
        _renderer->resetState();
        drawFps();
    }
}

void Application::drawFps() {
    _font->start();
    Rectangle* viewport = getView()->getViewport();
    int padding = 10;
    int fontSize = 13;
    float y = viewport->height / Toolkit::cur()->getScreenScale() - fontSize - padding;
    float x = 100+padding;
    char buffer[256] = { 0 };
    int drawCall = _renderer->drawCallCount();
    snprintf(buffer, 256, "FPS:%d, DC:%d", _frameRate, drawCall);
    _font->drawText(buffer, x, y, Vector4::one(), fontSize);
    _font->finish(NULL);
}

void Application::showFps(bool v) {
    _showFps = v;
}

double Application::getGameTime()
{
    return Toolkit::cur()->getGameTime() - _pausedTimeTotal;
}

int Application::run(int w, int h)
{
    if (_state != Uninitialized)
        return -1;

    //loadConfig();

    _width = w;
    _height = h;

    // Start up game systems.
    if (!startup())
    {
        shutdown();
        return -2;
    }

    return 0;
}

bool Application::startup()
{
    if (_state != Uninitialized)
        return false;

    _renderer = new GLRenderer();
    g_rendererInstance = _renderer;
    _sceneViews.push_back(new SceneView());
    _sceneViews[0]->setRenderPath(UPtr<RenderPath>(new RenderPath(_renderer) ));

    _animationController = new AnimationController();
    _animationController->initialize();

    _physicsController = new PhysicsController();
    _physicsController->initialize();
#ifndef __EMSCRIPTEN__
    _audioController = new AudioController();
    _audioController->initialize();

    _aiController = new AIController();
    _aiController->initialize();

    _audioListener = new AudioListener();
#endif

#ifdef WASE_UI
    waseUI::init();
#endif
#ifdef GP_UI
    _forms = new FormManager();
#endif
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
    _state = Initing;

    return true;
}

void Application::shutdown()
{
    g_rendererInstance = _renderer;
    // Call user finalization.
    if (_state != Uninitialized)
    {
        GP_ASSERT(_animationController);
        GP_ASSERT(_physicsController);
    #ifndef __EMSCRIPTEN__
        GP_ASSERT(_audioController);
        GP_ASSERT(_aiController);
    #endif

        //Platform::signalShutdown();

        Platform::cur()->clearSchedule();

		// Call user finalize
        finalize();

        for (auto view : _sceneViews) {
            view->finalize();
            SAFE_DELETE(view);
        }
        _sceneViews.clear();
        
#ifdef WASE_UI
    waseUI::finalize();
#endif
#ifdef GP_UI
        _forms->finalize();
        SAFE_DELETE(_forms);
#endif
    #if GP_SCRIPT_ENABLE
        // Call script finalize
        if (_scriptTarget)
            _scriptTarget->fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(GameScriptTarget, finalize));

        // Destroy script target so no more script events are fired
        SAFE_DELETE(_scriptTarget);

		// Shutdown scripting system first so that any objects allocated in script are released before our subsystems are released
		_scriptController->finalize();

    #endif
        _physicsController->finalize();
        SAFE_DELETE(_physicsController);
    #ifndef __EMSCRIPTEN__
        /*unsigned int gamepadCount = Gamepad::getGamepadCount();
        for (unsigned int i = 0; i < gamepadCount; i++)
        {
            Gamepad* gamepad = Gamepad::getGamepad(i, false);
            SAFE_DELETE(gamepad);
        }*/

        _audioController->finalize();
        SAFE_DELETE(_audioController);

        _aiController->finalize();
        SAFE_DELETE(_aiController);

        SAFE_DELETE(_audioListener);
    #endif
        
        //ControlFactory::finalize();
#ifdef GP_UI
        Theme::finalize();
#endif
        // Note: we do not clean up the script controller here
        // because users can call Application::exit() from a script.

        //FrameBuffer::finalize();
        //RenderState::finalize();

        _font.clear();


        //RenderPath::releaseStatic();


        AssetManager::getInstance()->clear();

        _animationController->finalize();
        SAFE_DELETE(_animationController);


        delete _renderer;
        _renderer = NULL;
        g_rendererInstance = NULL;

		_state = Uninitialized;
    }
}

void Application::pause()
{
    if (_state == Runing)
    {
        GP_ASSERT(_animationController);
        GP_ASSERT(_physicsController);
    #ifndef __EMSCRIPTEN__
        GP_ASSERT(_audioController);
        GP_ASSERT(_aiController);
    #endif
        _state = Paused;
        _pausedTimeLast = System::millisTicks();
        _animationController->pause();
        _physicsController->pause();
    #ifndef __EMSCRIPTEN__
        _audioController->pause();
        _aiController->pause();
    #endif
    }

    ++_pausedCount;
}

void Application::resume()
{
    if (_state == Paused)
    {
        --_pausedCount;

        if (_pausedCount == 0)
        {
            GP_ASSERT(_animationController);
            GP_ASSERT(_physicsController);
        #ifndef __EMSCRIPTEN__
            GP_ASSERT(_audioController);
            GP_ASSERT(_aiController);
        #endif
            _state = Runing;
            _pausedTimeTotal += System::millisTicks() - _pausedTimeLast;
            _animationController->resume();

            _physicsController->resume();
        #ifndef __EMSCRIPTEN__
            _audioController->resume();
            _aiController->resume();
        #endif
        }
    }
}

void Application::exit()
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
	Toolkit::cur()->schedule(0, &listener, this);

#else

    // End the process immediately without a full shutdown
    ::exit(0);

#endif
}


void Application::frame()
{
    g_rendererInstance = _renderer;
    if (_state != Runing)
    {
        // Perform lazy first time initialization
        _renderer->init();
        initialize();
        #if GP_SCRIPT_ENABLE
        if (_scriptTarget)
            _scriptTarget->fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(GameScriptTarget, initialize));
        #endif
        _state = Runing;
        // Fire first game resize event
        notifyResizeEvent(_width, _height);
    }

	static double lastFrameTime = Application::getGameTime();
	double frameTime = getGameTime();

    // Fire time events to scheduled TimeListeners
    Platform::cur()->fireTimeEvents();

    _renderer->beginFrame();
    if (_state == Application::Runing)
    {
        GP_ASSERT(_animationController);
        GP_ASSERT(_physicsController);
    #ifndef __EMSCRIPTEN__
        GP_ASSERT(_audioController);
        GP_ASSERT(_aiController);
    #endif
        // Update Time.
        float elapsedTime = (frameTime - lastFrameTime);
        lastFrameTime = frameTime;

        // Update the scheduled and running animations.
        _animationController->update(elapsedTime);

        // Update the physics.
        _physicsController->update(elapsedTime);
    #ifndef __EMSCRIPTEN__
        // Update AI.
        _aiController->update(elapsedTime);
    #endif
        // Update gamepads.
        //Gamepad::updateInternal(elapsedTime);

        // Application Update.
        update(elapsedTime);
    #ifdef GP_UI
            // Update forms.
        _forms->updateInternal(elapsedTime);
    #endif
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
        if ((Application::getGameTime() - _frameTimeLastFPS) >= 1000)
        {
            _frameRate = _frameCount;
            _frameCount = 0;
            _frameTimeLastFPS = getGameTime();
        }
    }
	else if (_state == Application::Paused)
    {
        // Update gamepads.
        //Gamepad::updateInternal(0);

        // Application Update.
        update(0);
    #ifdef GP_UI
            // Update forms.
        _forms->updateInternal(0);
    #endif

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
    _renderer->endFrame();
}

bool Application::keyEvent(Keyboard evt) {
    auto cameraCtrl = getView()->getCameraCtrl();
    if (cameraCtrl) {
        return cameraCtrl->keyEvent(evt);
    }
    return false;
}

bool Application::mouseEvent(Mouse evt) {
    auto cameraCtrl = getView()->getCameraCtrl();
    if (cameraCtrl) {
        return cameraCtrl->mouseEvent(evt);
    }
    return false;
}

void Application::resizeEvent(unsigned int width, unsigned int height) {
    Rectangle vp(0.0f, 0.0f, (float)_width, (float)_height);
    getView()->setViewport(&vp);
}

void Application::notifyKeyEvent(Keyboard evt)
{
#ifdef WASE_UI
    if (waseUI::keyEvent(evt)) {
        return;
    }
#endif
#ifdef GP_UI
    if (_forms->keyEventInternal(evt.evt, evt.key)) {
        return;
    }
#endif
    if (_inputListener && _inputListener->keyEvent(evt))
        return;

    if (keyEvent(evt))
        return;

    #if GP_SCRIPT_ENABLE
    if (_scriptTarget)
        _scriptTarget->fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(GameScriptTarget, keyEvent), &evt);
    #endif
}

bool Application::notifyMouseEvent(Mouse evt)
{
    if (!evt.time) {
        evt.time = System::currentTimeMillis();
    }

#ifdef WASE_UI
    if (waseUI::mouseEvent(evt)) {
        return true;
    }
#endif
#ifdef GP_UI
    if (_forms->mouseEventInternal(evt))
        return true;
#endif
    if (_inputListener && _inputListener->mouseEvent(evt))
        return true;

    if (mouseEvent(evt))
        return true;

    #if GP_SCRIPT_ENABLE
    if (_scriptTarget)
        return _scriptTarget->fireScriptEvent<bool>(GP_GET_SCRIPT_EVENT(GameScriptTarget, mouseEvent), &evt);
    #endif
    return false;
}

void Application::notifyResizeEvent(unsigned int width, unsigned int height)
{
    // Update the width and height of the game
    if (_width != width || _height != height)
    {
        _width = width;
        _height = height;
    }

    if (_renderer)
        _renderer->onResize(width, height);

    if (_state == Runing) {
        this->resizeEvent(width, height);

#if GP_SCRIPT_ENABLE
        if (_scriptTarget)
            _scriptTarget->fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(GameScriptTarget, resizeEvent), width, height);
#endif
    }

#ifdef WASE_UI
    waseUI::resize(width, height);
#endif
#ifdef GP_UI
    _forms->resizeEventInternal(width, height);
#endif
}

void Application::ShutdownListener::timeEvent(int64_t timeDiff, void* cookie)
{
	((Application*)cookie)->shutdown();
}

void Application::setInputListener(InputListener* t) {
    if (_inputListener != t) {
        if (_inputListener) {
            _inputListener->onTeardown();
        }

        _inputListener = t;

        if (_inputListener) {
            _inputListener->onSetup();
        }
    }
}

}

