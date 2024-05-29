/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#ifndef GP_NO_PLATFORM
#ifdef GP_GLFW

#if __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#define WIN32_LEAN_AND_MEAN
#define GLEW_STATIC
#include <GL/glew.h>
//#include <GL/wglew.h>
#include <GLFW/glfw3.h>

#include "PlatformGlfw.h"
#include "app/Application.h"

namespace mgp
{
GLFWwindow* window;
static bool __multiSampling = false;

#ifndef _WIN32
int __argc = 0;
char** __argv = 0;
#endif

PlatformGlfw::PlatformGlfw() {
    Platform::cur = this;
}


extern int strcmpnocase(const char* s1, const char* s2)
{
#ifdef _WIN32
    return _strcmpi(s1, s2);
#else
    return strcasecmp(s1, s2);
#endif
}

static Keyboard::Key getKey(int keyCode, bool modifier)
{
    bool shiftDown = (modifier & GLFW_MOD_SHIFT) != 0;
    switch (keyCode)
    {
    case GLFW_KEY_PAUSE:
        return mgp::Keyboard::KEY_PAUSE;
    case GLFW_KEY_SCROLL_LOCK:
        return mgp::Keyboard::KEY_SCROLL_LOCK;
    case GLFW_KEY_PRINT_SCREEN:
        return mgp::Keyboard::KEY_PRINT;
    case GLFW_KEY_ESCAPE:
        return mgp::Keyboard::KEY_ESCAPE;
    case GLFW_KEY_BACKSPACE:
    case GLFW_KEY_F16: // generated by CTRL + BACKSPACE
        return mgp::Keyboard::KEY_BACKSPACE;
    case GLFW_KEY_TAB:
        return shiftDown ? mgp::Keyboard::KEY_BACK_TAB : mgp::Keyboard::KEY_TAB;
    case GLFW_KEY_ENTER:
        return mgp::Keyboard::KEY_RETURN;
    case GLFW_KEY_CAPS_LOCK:
        return mgp::Keyboard::KEY_CAPS_LOCK;
    case GLFW_KEY_LEFT_SHIFT:
    case GLFW_KEY_RIGHT_SHIFT:
        return mgp::Keyboard::KEY_SHIFT;
    case GLFW_KEY_LEFT_CONTROL:
    case GLFW_KEY_RIGHT_CONTROL:
        return mgp::Keyboard::KEY_CTRL;
    case GLFW_KEY_MENU:
        return mgp::Keyboard::KEY_MENU;
    case GLFW_KEY_LEFT_ALT:
    case GLFW_KEY_RIGHT_ALT:
        return mgp::Keyboard::KEY_ALT;
    case GLFW_KEY_INSERT:
        return mgp::Keyboard::KEY_INSERT;
    case GLFW_KEY_HOME:
        return mgp::Keyboard::KEY_HOME;
    case GLFW_KEY_PAGE_UP:
        return mgp::Keyboard::KEY_PG_UP;
    case GLFW_KEY_DELETE:
        return mgp::Keyboard::KEY_DELETE;
    case GLFW_KEY_END:
        return mgp::Keyboard::KEY_END;
    case GLFW_KEY_PAGE_DOWN:
        return mgp::Keyboard::KEY_PG_DOWN;
    case GLFW_KEY_LEFT:
        return mgp::Keyboard::KEY_LEFT_ARROW;
    case GLFW_KEY_RIGHT:
        return mgp::Keyboard::KEY_RIGHT_ARROW;
    case GLFW_KEY_UP:
        return mgp::Keyboard::KEY_UP_ARROW;
    case GLFW_KEY_DOWN:
        return mgp::Keyboard::KEY_DOWN_ARROW;
    case GLFW_KEY_NUM_LOCK:
        return mgp::Keyboard::KEY_NUM_LOCK;
    case GLFW_KEY_KP_ADD:
        return mgp::Keyboard::KEY_KP_PLUS;
    case GLFW_KEY_KP_SUBTRACT:
        return mgp::Keyboard::KEY_KP_MINUS;
    case GLFW_KEY_KP_MULTIPLY:
        return mgp::Keyboard::KEY_KP_MULTIPLY;
    case GLFW_KEY_KP_DIVIDE:
        return mgp::Keyboard::KEY_KP_DIVIDE;
    case GLFW_KEY_KP_7:
        return mgp::Keyboard::KEY_KP_HOME;
    case GLFW_KEY_KP_8:
        return mgp::Keyboard::KEY_KP_UP;
    case GLFW_KEY_KP_9:
        return mgp::Keyboard::KEY_KP_PG_UP;
    case GLFW_KEY_KP_4:
        return mgp::Keyboard::KEY_KP_LEFT;
    case GLFW_KEY_KP_5:
        return mgp::Keyboard::KEY_KP_FIVE;
    case GLFW_KEY_KP_6:
        return mgp::Keyboard::KEY_KP_RIGHT;
    case GLFW_KEY_KP_1:
        return mgp::Keyboard::KEY_KP_END;
    case GLFW_KEY_KP_2:
        return mgp::Keyboard::KEY_KP_DOWN;
    case GLFW_KEY_KP_3:
        return mgp::Keyboard::KEY_KP_PG_DOWN;
    case GLFW_KEY_KP_0:
        return mgp::Keyboard::KEY_KP_INSERT;
    case GLFW_KEY_KP_DECIMAL:
        return mgp::Keyboard::KEY_KP_DELETE;
    case GLFW_KEY_F1:
        return mgp::Keyboard::KEY_F1;
    case GLFW_KEY_F2:
        return mgp::Keyboard::KEY_F2;
    case GLFW_KEY_F3:
        return mgp::Keyboard::KEY_F3;
    case GLFW_KEY_F4:
        return mgp::Keyboard::KEY_F4;
    case GLFW_KEY_F5:
        return mgp::Keyboard::KEY_F5;
    case GLFW_KEY_F6:
        return mgp::Keyboard::KEY_F6;
    case GLFW_KEY_F7:
        return mgp::Keyboard::KEY_F7;
    case GLFW_KEY_F8:
        return mgp::Keyboard::KEY_F8;
    case GLFW_KEY_F9:
        return mgp::Keyboard::KEY_F9;
    case GLFW_KEY_F10:
        return mgp::Keyboard::KEY_F10;
    case GLFW_KEY_F11:
        return mgp::Keyboard::KEY_F11;
    case GLFW_KEY_F12:
        return mgp::Keyboard::KEY_F12;
    case GLFW_KEY_SPACE:
        return mgp::Keyboard::KEY_SPACE;
    case 0x30:
        return shiftDown ? mgp::Keyboard::KEY_RIGHT_PARENTHESIS : mgp::Keyboard::KEY_ZERO;
    case 0x31:
        return shiftDown ? mgp::Keyboard::KEY_EXCLAM : mgp::Keyboard::KEY_ONE;
    case 0x32:
        return shiftDown ? mgp::Keyboard::KEY_AT : mgp::Keyboard::KEY_TWO;
    case 0x33:
        return shiftDown ? mgp::Keyboard::KEY_NUMBER : mgp::Keyboard::KEY_THREE;
    case 0x34:
        return shiftDown ? mgp::Keyboard::KEY_DOLLAR : mgp::Keyboard::KEY_FOUR;
    case 0x35:
        return shiftDown ? mgp::Keyboard::KEY_PERCENT : mgp::Keyboard::KEY_FIVE;
    case 0x36:
        return shiftDown ? mgp::Keyboard::KEY_CIRCUMFLEX : mgp::Keyboard::KEY_SIX;
    case 0x37:
        return shiftDown ? mgp::Keyboard::KEY_AMPERSAND : mgp::Keyboard::KEY_SEVEN;
    case 0x38:
        return shiftDown ? mgp::Keyboard::KEY_ASTERISK : mgp::Keyboard::KEY_EIGHT;
    case 0x39:
        return shiftDown ? mgp::Keyboard::KEY_LEFT_PARENTHESIS : mgp::Keyboard::KEY_NINE;
    case GLFW_KEY_EQUAL:
        return shiftDown ? mgp::Keyboard::KEY_EQUAL : mgp::Keyboard::KEY_PLUS;
    case GLFW_KEY_COMMA:
        return shiftDown ? mgp::Keyboard::KEY_LESS_THAN : mgp::Keyboard::KEY_COMMA;
    case GLFW_KEY_MINUS:
        return shiftDown ? mgp::Keyboard::KEY_UNDERSCORE : mgp::Keyboard::KEY_MINUS;
    case GLFW_KEY_PERIOD:
        return shiftDown ? mgp::Keyboard::KEY_GREATER_THAN : mgp::Keyboard::KEY_PERIOD;
    case GLFW_KEY_SEMICOLON:
        return shiftDown ? mgp::Keyboard::KEY_COLON : mgp::Keyboard::KEY_SEMICOLON;
    case GLFW_KEY_SLASH:
        return shiftDown ? mgp::Keyboard::KEY_QUESTION : mgp::Keyboard::KEY_SLASH;
    case GLFW_KEY_GRAVE_ACCENT:
        return shiftDown ? mgp::Keyboard::KEY_TILDE : mgp::Keyboard::KEY_GRAVE;
    case GLFW_KEY_LEFT_BRACKET:
        return shiftDown ? mgp::Keyboard::KEY_LEFT_BRACE : mgp::Keyboard::KEY_LEFT_BRACKET;
    case GLFW_KEY_BACKSLASH:
        return shiftDown ? mgp::Keyboard::KEY_BAR : mgp::Keyboard::KEY_BACK_SLASH;
    case GLFW_KEY_RIGHT_BRACKET:
        return shiftDown ? mgp::Keyboard::KEY_RIGHT_BRACE : mgp::Keyboard::KEY_RIGHT_BRACKET;
    case GLFW_KEY_APOSTROPHE:
        return shiftDown ? mgp::Keyboard::KEY_QUOTE : mgp::Keyboard::KEY_APOSTROPHE;
    case 0x41:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_A : mgp::Keyboard::KEY_A;
    case 0x42:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_B : mgp::Keyboard::KEY_B;
    case 0x43:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_C : mgp::Keyboard::KEY_C;
    case 0x44:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_D : mgp::Keyboard::KEY_D;
    case 0x45:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_E : mgp::Keyboard::KEY_E;
    case 0x46:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_F : mgp::Keyboard::KEY_F;
    case 0x47:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_G : mgp::Keyboard::KEY_G;
    case 0x48:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_H : mgp::Keyboard::KEY_H;
    case 0x49:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_I : mgp::Keyboard::KEY_I;
    case 0x4A:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_J : mgp::Keyboard::KEY_J;
    case 0x4B:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_K : mgp::Keyboard::KEY_K;
    case 0x4C:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_L : mgp::Keyboard::KEY_L;
    case 0x4D:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_M : mgp::Keyboard::KEY_M;
    case 0x4E:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_N : mgp::Keyboard::KEY_N;
    case 0x4F:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_O : mgp::Keyboard::KEY_O;
    case 0x50:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_P : mgp::Keyboard::KEY_P;
    case 0x51:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_Q : mgp::Keyboard::KEY_Q;
    case 0x52:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_R : mgp::Keyboard::KEY_R;
    case 0x53:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_S : mgp::Keyboard::KEY_S;
    case 0x54:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_T : mgp::Keyboard::KEY_T;
    case 0x55:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_U : mgp::Keyboard::KEY_U;
    case 0x56:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_V : mgp::Keyboard::KEY_V;
    case 0x57:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_W : mgp::Keyboard::KEY_W;
    case 0x58:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_X : mgp::Keyboard::KEY_X;
    case 0x59:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_Y : mgp::Keyboard::KEY_Y;
    case 0x5A:
        return shiftDown ? mgp::Keyboard::KEY_CAPITAL_Z : mgp::Keyboard::KEY_Z;
    default:
        return mgp::Keyboard::KEY_NONE;
    }
}


void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Keyboard::KeyEvent e = Keyboard::KeyEvent::KEY_PRESS;
    if (action == GLFW_RELEASE) {
        e = Keyboard::KeyEvent::KEY_RELEASE;
    }

    Keyboard evt;
    evt.evt = e;
    evt.key = getKey(key, mods);
    evt.modifier = mods;

    /*if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);*/

    Application::getInstance()->notifyKeyEvent(evt);
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
    Keyboard evt;
    evt.evt = Keyboard::KeyEvent::KEY_CHAR;
    evt.key = codepoint;

    Application::getInstance()->notifyKeyEvent(evt);
}

float lastXScale = 0;
float lastYScale = 0;

static void getContentScale(GLFWwindow* window) {
    if (lastXScale == 0) {
        float xscale, yscale;
        glfwGetWindowContentScale(window, &xscale, &yscale);
        lastXScale = xscale;
        lastYScale = yscale;
    }
}

static void getCursorPosPixel(GLFWwindow* window, double* x, double* y) {
    glfwGetCursorPos(window, x, y);
#if defined(_WIN32) || defined(__EMSCRIPTEN__)
#else
    getContentScale(window);
    *x *= lastXScale;
    *y *= lastYScale;
#endif
}

static void cursor_position_callback(GLFWwindow* window, double x, double y)
{
#if defined(_WIN32) || defined(__EMSCRIPTEN__)
#else
    getContentScale(window);
    x *= lastXScale;
    y *= lastYScale;
#endif
    
    mgp::MotionEvent evt;
    evt.type = mgp::MotionEvent::mouseMove;
    evt.x = x;
    evt.y = y;
    evt.wheelDelta = 0;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        evt.type = mgp::MotionEvent::touchMove;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        evt.type = mgp::MotionEvent::touchMove;
        evt.button = mgp::MotionEvent::right;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
        evt.type = mgp::MotionEvent::touchMove;
        evt.button = mgp::MotionEvent::middle;
    }

    Application::getInstance()->notifyMouseEvent(evt);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double x, y;
    getCursorPosPixel(window, &x, &y);

    mgp::MotionEvent evt;
    //evt.evt = mgp::Mouse::MOUSE_PRESS_LEFT_BUTTON;
    evt.x = x;
    evt.y = y;
    evt.wheelDelta = 0;
    evt.time = System::currentTimeMillis();

    evt.type = (action == GLFW_PRESS) ? mgp::MotionEvent::press : mgp::MotionEvent::release;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        evt.button = mgp::MotionEvent::left;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        evt.button = mgp::MotionEvent::right;
    }
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        evt.button = mgp::MotionEvent::middle;
    }

    Application::getInstance()->notifyMouseEvent(evt);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    double x, y;
    getCursorPosPixel(window, &x, &y);

    mgp::MotionEvent evt;
    evt.type = mgp::MotionEvent::wheel;
    evt.button = mgp::MotionEvent::Button::middle;
    evt.x = x;
    evt.y = y;

#if __APPLE__
    int delta = yoffset * 10;
    evt.wheelDelta = delta;
#else
    evt.wheelDelta = yoffset;
#endif

    Application::getInstance()->notifyMouseEvent(evt);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    Application::getInstance()->notifyResizeEvent(width, height);
}

void window_content_scale_callback(GLFWwindow* window, float xscale, float yscale)
{
    lastXScale = xscale;
    lastYScale = yscale;
}

void PlatformGlfw::init(const char* title, int w, int h)
{
    //FileSystem::setResourcePath("./");

    glfwSetErrorCallback(error_callback);

    /* Initialize the library */
    if (!glfwInit())
        goto error;

#if _WIN32
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_SAMPLES, 4);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(w, h, title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        goto error;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, character_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    //glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowContentScaleCallback(window, window_content_scale_callback);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    //gladLoadGL(glfwGetProcAddress);
    if (GLEW_OK != glewInit())
    {
        GP_ERROR("Failed to initialize GLEW.");
        glfwTerminate();
        goto error;
    }

#ifndef __EMSCRIPTEN__
    glfwSwapInterval(1);
#endif
    return;
error:

    exit(0);
}


static int doFrame(double time, void* userData) {
    Application* _game = Application::getInstance();
    GP_ASSERT(_game);

    _game->frame();

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();

    return _game->getState() != Application::UNINITIALIZED;
}

int PlatformGlfw::enterMessagePump()
{
    Application* _game = Application::getInstance();
    if (_game->getState() != Application::RUNNING) {
        int width = getDisplayWidth();
        int height = getDisplayHeight();
        _game->run(width, height);
    }

#if __EMSCRIPTEN__
    emscripten_request_animation_frame_loop(doFrame, NULL);
#else
    while (!glfwWindowShouldClose(window))
    {
        // If we are done, then exit.
        if (!doFrame(0, NULL))
            break;
    }

    if (_game->getState() == Application::RUNNING) {
        _game->shutdown();
    }
#endif

    
    return 0;
}


void PlatformGlfw::signalShutdown()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool PlatformGlfw::canExit()
{
    return true;
}

unsigned int PlatformGlfw::getDisplayWidth()
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return width;
}

unsigned int PlatformGlfw::getDisplayHeight()
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return height;
}

float PlatformGlfw::getScreenScale() {
    getContentScale(window);
    return lastXScale > lastYScale ? lastXScale : lastYScale;
}

bool PlatformGlfw::isVsync()
{
    return false;
}

void PlatformGlfw::setVsync(bool enable)
{
}

void PlatformGlfw::swapBuffers()
{
    glfwSwapBuffers(window);
}

void PlatformGlfw::setMultiSampling(bool enabled)
{
    if (enabled == __multiSampling)
    {
        return;
    }

    if (enabled)
    {
        glEnable(GL_MULTISAMPLE);
    }
    else
    {
        glDisable(GL_MULTISAMPLE);
    }

    __multiSampling = enabled;
}

bool PlatformGlfw::isMultiSampling()
{
    return __multiSampling;
}


bool PlatformGlfw::hasAccelerometer()
{
    return false;
}

void PlatformGlfw::getAccelerometerValues(float* pitch, float* roll)
{
    GP_ASSERT(pitch);
    GP_ASSERT(roll);

    *pitch = 0;
    *roll = 0;
}

void PlatformGlfw::getSensorValues(float* accelX, float* accelY, float* accelZ, float* gyroX, float* gyroY, float* gyroZ)
{
    if (accelX)
    {
        *accelX = 0;
    }

    if (accelY)
    {
        *accelY = 0;
    }

    if (accelZ)
    {
        *accelZ = 0;
    }

    if (gyroX)
    {
        *gyroX = 0;
    }

    if (gyroY)
    {
        *gyroY = 0;
    }

    if (gyroZ)
    {
        *gyroZ = 0;
    }
}

void PlatformGlfw::getArguments(int* argc, char*** argv)
{
    if (argc)
        *argc = __argc;
    if (argv)
        *argv = __argv;
}

bool PlatformGlfw::hasMouse()
{
    return true;
}

void PlatformGlfw::setMouseCaptured(bool captured)
{
    glfwSetInputMode(window, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

bool PlatformGlfw::isMouseCaptured()
{
    //return __mouseCaptured;
    return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
}

void PlatformGlfw::setCursorVisible(bool visible)
{
    glfwSetInputMode(window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
}

bool PlatformGlfw::isCursorVisible()
{
    //return __cursorVisible;
    return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
}

void PlatformGlfw::displayKeyboard(bool display)
{
    // Do nothing.
}

void PlatformGlfw::requestRepaint() {
    // Do nothing.
}

bool PlatformGlfw::launchURL(const char* url) {
    //TODO
    return false;
}
std::string PlatformGlfw::displayFileDialog(size_t mode, const char* title, const char* filterDescription, const char* filterExtensions, const char* initialDirectory) {
    //TODO
    return "";
}

}

#endif //GP_GLFW
#endif //GP_NO_PLATFORM
