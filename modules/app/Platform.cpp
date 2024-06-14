// Implementation of base platform-agnostic platform functionality.
#include "base/Base.h"
#include "Platform.h"
#include "app/Application.h"
//#include "script/ScriptController.h"
#include "PlatformGlfw.h"

namespace mgp
{
    Platform::Platform() {
        _eventTimer = new EventTimer();
        Toolkit::g_instance = this;
    }
    Platform::~Platform() {
        delete _eventTimer;
    }

	Platform* Platform::_cur = NULL;

    int Platform::run(Application* game, const char* title, int w, int h) {
        PlatformGlfw* platform = new PlatformGlfw();
        Platform::_cur = platform;
        platform->_game = game;

        GP_ASSERT(game);
        platform->init(title, w, h);
        int result = platform->enterMessagePump();

#ifndef __EMSCRIPTEN__
        platform->signalShutdown();
        delete platform;
#endif
        return result;
    }
}
