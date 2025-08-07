
#include <iostream>
#include "mgp.h"
#include "waseGui.h"

using namespace sric;
using namespace waseGui;
using namespace mgp;

class MainApp : public Application {

    void initialize() {
        auto frame = new_<Frame>();
        {
            auto it = new_<waseGui::Button>();
            it->setText("Button");
            it->onClick = ([=](RefPtr<Widget> w) {
                waseGui::Toast::showText("hello world");
                });
            frame->add(std::move(it));
            frame->background.rgba = 0;
        }
        frame->show();
    }

    void render(float elapsedTime) override {
        Renderer::cur()->clear(Renderer::CLEAR_COLOR_DEPTH_STENCIL, Vector4::fromColor(0x888888ff));
        Application::render(elapsedTime);
    }

};

int main() {
    printf("main start\n");
    
    #if __EMSCRIPTEN__
        MainApp* instance = new MainApp();
        return Platform::run(instance);
    #else
        MainApp instance;
        return Platform::run(&instance);
    #endif
}