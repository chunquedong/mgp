
#include <iostream>
#include "mgp.h"

using namespace mgp;

/**
 * Creates a triangle mesh with vertex colors.
 */
static UPtr<Mesh> createTriangleMesh()
{
    // Calculate the vertices of the equilateral triangle.
    float a = 0.5f;     // length of the side
    Vector2 p1(0.0f, a / sqrtf(3.0f));
    Vector2 p2(-a / 2.0f, -a / (2.0f * sqrtf(3.0f)));
    Vector2 p3(a / 2.0f, -a / (2.0f * sqrtf(3.0f)));

    // Create 3 vertices. Each vertex has position (x, y, z) and color (red, green, blue)
    float vertices[] =
    {
        (float)p1.x, (float)p1.y, 0.0f,     1.0f, 0.0f, 0.0f,
        (float)p2.x, (float)p2.y, 0.0f,     0.0f, 1.0f, 0.0f,
        (float)p3.x, (float)p3.y, 0.0f,     0.0f, 0.0f, 1.0f,
    };
    unsigned int vertexCount = 3;
    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::COLOR, 3)
    };
    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 2), vertexCount);
    if (mesh.isNull())
    {
        GP_ERROR("Failed to create mesh.");
        return UPtr<Mesh>(NULL);
    }
    mesh->setPrimitiveType(Mesh::TRIANGLES);
    mesh->getVertexBuffer()->setData((char*)vertices, sizeof(vertices));
    return mesh;
}

class MainApp : public Game {


    void initialize() {

        // Create a new empty scene.
        UPtr<Scene> _scene = Scene::create();

        // Create the triangle mesh.
        UPtr<Mesh> mesh = createTriangleMesh();

        // Create a model for the triangle mesh. A model is an instance of a Mesh that can be drawn with a specified material.
        UPtr<Model> _model = Model::create(std::move(mesh));
        //SAFE_RELEASE(mesh);

        // Create a material from the built-in "colored-unlit" vertex and fragment shaders.
        // This sample doesn't use lighting so the unlit shader is used.
        // This sample uses vertex color so VERTEX_COLOR is defined. Look at the shader source files to see the supported defines.
        Material *material = _model->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "VERTEX_COLOR");
        material->getStateBlock()->setCullFace(false);
        //material->getParameter("u_diffuseColor")->setVector4(Vector4(50,0,0,1));

        Node* modelNode = _scene->addNode("model");
        modelNode->setDrawable(_model.dynamicCastTo<Drawable>());
        //SAFE_RELEASE(_model);


        getView()->setScene(std::move(_scene));
        getView()->initCamera(false, 0.01);
    }
};


int main() {
    printf("main start\n");
    
    #if __EMSCRIPTEN__
        MainApp* instance = new MainApp();
    #else
        MainApp instance;
    #endif
    
    return Platform::run();
}
