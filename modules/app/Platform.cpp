// Implementation of base platform-agnostic platform functionality.
#include "base/Base.h"
#include "Platform.h"
#include "app/Application.h"
//#include "script/ScriptController.h"
#include "PlatformGlfw.h"

namespace mgp
{
    Platform::Platform() {

    }
    Platform::~Platform() {

    }

	Platform* Platform::cur = NULL;

    int Platform::run(const char* title, int w, int h) {
        PlatformGlfw glfw;

        Platform* platform = Platform::cur;

        Application* game = Application::getInstance();
        GP_ASSERT(game);
        platform->init(title, w, h);
        int result = platform->enterMessagePump();

#ifndef __EMSCRIPTEN__
        platform->signalShutdown();
#endif
        return result;
    }
}
