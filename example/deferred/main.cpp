
#include <iostream>
#include "mgp.h"
#include "render/RenderPath.h"

using namespace mgp;

class MainApp : public Game {

    void makeSpherical(Scene* _scene) {
        // Create the triangle mesh.
        Mesh* mesh = Mesh::createSpherical();
        Model* _model = Model::create(mesh);
        SAFE_RELEASE(mesh);
        Material* material = _model->setMaterial("res/shaders/colored.vert", "res/shaders/deferred/colored.frag");
        material->getParameter("u_diffuseColor")->setVector4(Vector4(0.5, 0.5, 0.5, 1.0));
        material->getParameter("u_specularExponent")->setFloat(5.0);

        Node* modelNode = _scene->addNode("model");
        modelNode->setDrawable(_model);
        modelNode->setTranslation(3, 0, 0);
        SAFE_RELEASE(_model);
    }

    void initialize() {
        Scene* _scene = Scene::create();
        getView()->setScene(_scene);
        makeSpherical(_scene);
        getView()->initCamera(true);

        // Create a directional light and a reference icon for the light
        Light* directionalLight = Light::createDirectional(Vector3(1.0, 0.0, 0.0));
        Node* _directionalLightNode = Node::create("directionalLight");
        _directionalLightNode->setLight(directionalLight);
        SAFE_RELEASE(directionalLight);
        _directionalLightNode->rotateY(MATH_DEG_TO_RAD(-90));
        _directionalLightNode->setTranslation(-10.0f, 0.0f, 0.0f);
        _scene->addNode(_directionalLightNode);
        SAFE_RELEASE(_directionalLightNode);

        getView()->getRenderPath()->initDeferred();
    }
};

int main() {
    MainApp instance;
    return Platform::run();
}