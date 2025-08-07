#ifndef MGPWINDOW_H_
#define MGPWINDOW_H_

#include "platform/Keyboard.h"
#include "platform/Mouse.h"

namespace waseUI {
    void init();
    void finalize();
    bool doFrame();

    void resize(int w, int h);
    bool keyEvent(mgp::Keyboard key);
    bool mouseEvent(mgp::Mouse evt);
}

#endif