
#include <iostream>
#include "mgp.h"

using namespace mgp;

class MainApp : public Application {
    void initialize() {

        GltfLoader loader;
        auto _scene = loader.load("res/gltf/car.gltf");

        getView()->setScene(std::move(_scene));
        getView()->initCamera(false);
    }
};

int main() {
    MainApp instance;
    return Platform::run(&instance);
}