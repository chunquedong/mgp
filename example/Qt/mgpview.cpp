#define GLEW_STATIC
#include <GL/glew.h>

#include "mgpview.h"
#include <QtWidgets>


using namespace mgp;

class PlatformQt : public Platform {
public:
    PlatformQt() {
    }

    virtual float getScreenScale() override {
        return QGuiApplication::primaryScreen()->devicePixelRatio();
    }
};

MgpView::MgpView(QWidget* parent) : QOpenGLWidget(parent)
{
    Platform::_cur = new PlatformQt();
}


MgpView::~MgpView()
{
    if (this->getState() == Application::RUNNING) {
        this->makeCurrent();
        this->shutdown();
    }
    //delete Platform::_cur;
    //Platform::_cur = NULL;
    //Game::exit();
}


static void addTestMesh(Scene* _scene, float r)
{
#if 1
    // Create 3 vertices. Each vertex has position (x, y, z) and color (red, green, blue)
    float vertices[] =
        {
            0, 0, 0,     1.0f, 0.0f, 0.0f,
            r, 0, 0,     1.0f, 0.0f, 0.0f,

            0, 0, 0,     0.0f, 1.0f, 0.0f,
            0, r, 0,     0.0f, 1.0f, 0.0f,

            0, 0, 0,     0.0f, 0.0f, 1.0f,
            0, 0, r,     0.0f, 0.0f, 1.0f,
        };
    unsigned int vertexCount = 6;
    VertexFormat::Element elements[] =
        {
            VertexFormat::Element(VertexFormat::POSITION, 3),
            VertexFormat::Element(VertexFormat::COLOR, 3)
        };
    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 2), vertexCount);
    if (mesh.get() == NULL)
    {
        GP_ERROR("Failed to create mesh.");
        return;
    }
    mesh->setPrimitiveType(Mesh::LINES);
    mesh->getVertexBuffer()->setData((char*)vertices, sizeof(vertices));

    UPtr<Model> _model = Model::create(std::move(mesh));

    Material* material = _model->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "VERTEX_COLOR");
    material->getStateBlock()->setCullFace(false);

    Node* modelNode = _scene->addNode("axis");
    modelNode->setDrawable(_model.dynamicCastTo<Drawable>());
#else
    UPtr<Model> _model = Model::create(Mesh::createSpherical());
    Material* material = _model->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag");
    material->getParameter("u_diffuseColor")->setVector4(Vector4(0.0, 0.8, 1.0, 1.0));

    Node* modelNode = _scene->addNode("test");
    modelNode->setDrawable(_model.dynamicCastTo<Drawable>());
    modelNode->setTranslation(-16679.187500000000, -379663.18750000000, -191802.21875000000);
    modelNode->scale(1671194.7448840307);
#endif
}

void MgpView::initialize() {
    UPtr<Scene> _scene = Scene::create();
    addTestMesh(_scene.get(), 1000);

    getView()->setScene(std::move(_scene));
    getView()->initCamera(false, 0.01);
}

void MgpView::finalize() {
    Application::finalize();
}

void MgpView::initializeGL() {
    initializeOpenGLFunctions();
    if (GLEW_OK != glewInit())
    {
        printf("Failed to initialize GLEW.\n");
        return;
    }

    /* Version */
    const GLubyte *vendor = glGetString( GL_VENDOR );
    const GLubyte *renderer = glGetString( GL_RENDERER );
    const GLubyte *version = glGetString( GL_VERSION );
    const GLubyte *shader = glGetString( GL_SHADING_LANGUAGE_VERSION );

    printf("GL Vendor    : %s\n", vendor);
    printf("GL Renderer  : %s\n", renderer);
    printf("GL Version   : %s\n", version);
    printf("GL Shader    : %s\n", shader);

    int width = this->size().width();
    int height = this->size().height();
    width *= QGuiApplication::primaryScreen()->devicePixelRatio();
    height *= QGuiApplication::primaryScreen()->devicePixelRatio();
    this->run(width, height);
}

void MgpView::paintGL() {
    Application::frame();

    //context()->swapBuffers(context()->surface());
    QOpenGLWidget::update();
}

void MgpView::resizeGL(int width, int height) {
    width *= QGuiApplication::primaryScreen()->devicePixelRatio();
    height *= QGuiApplication::primaryScreen()->devicePixelRatio();
    this->notifyResizeEvent(width, height);
}

static void mouseEventConvert(QMouseEvent* evt, mgp::MotionEvent &mouse) {
    float d = QGuiApplication::primaryScreen()->devicePixelRatio();
    mouse.x = evt->x() * d;
    mouse.y = evt->y() * d;

    if (evt->buttons() & Qt::LeftButton) {
        mouse.button = MotionEvent::left;
    }
    else if (evt->buttons() & Qt::RightButton) {
        mouse.button = MotionEvent::right;
    }
    else if (evt->buttons() & Qt::MiddleButton) {
        mouse.button = MotionEvent::middle;
    }
    else {
        if (evt->button() == Qt::LeftButton) {
            mouse.button = MotionEvent::left;
        }
        else if (evt->button() == Qt::RightButton) {
            mouse.button = MotionEvent::right;
        }
        else if (evt->button() == Qt::MiddleButton) {
            mouse.button = MotionEvent::middle;
        }
    }
}

void MgpView::mousePressEvent(QMouseEvent* evt)
{
    this->makeCurrent();
    _mouseDown = true;

    mgp::MotionEvent mouse;
    mouse.type = mgp::MotionEvent::press;
    mouseEventConvert(evt, mouse);

    this->notifyMouseEvent(mouse);
}

void MgpView::mouseReleaseEvent(QMouseEvent* evt)
{
    this->makeCurrent();
    _mouseDown = false;

    mgp::MotionEvent mouse;
    mouse.type = mgp::MotionEvent::release;
    mouseEventConvert(evt, mouse);

    this->notifyMouseEvent(mouse);
}

void MgpView::mouseMoveEvent(QMouseEvent* evt)
{
    this->makeCurrent();

    mgp::MotionEvent mouse;
    mouse.type = mgp::MotionEvent::mouseMove;
    if (_mouseDown)
    {
        mouse.type = mgp::MotionEvent::touchMove;
    }
    mouseEventConvert(evt, mouse);

    this->notifyMouseEvent(mouse);
}

void MgpView::wheelEvent(QWheelEvent* evt)
{
    this->makeCurrent();
    mgp::MotionEvent mouse;
    mouse.type = mgp::MotionEvent::wheel;
    float d = QGuiApplication::primaryScreen()->devicePixelRatio();
    mouse.x = evt->position().x() * d;
    mouse.y = evt->position().y() * d;
    mouse.wheelDelta = evt->angleDelta().y() / 120;

    this->notifyMouseEvent(mouse);
}

void MgpView::keyPressEvent(QKeyEvent* evt)
{
    this->makeCurrent();

    //TODO
    //Game::keyEventInternal(mgp::Keyboard::KEY_PRESS, getKey(wParam, shiftDown ^ capsOn));
}

void MgpView::keyReleaseEvent(QKeyEvent* evt)
{
    this->makeCurrent();
    // TODO: Handler here...
    //Game::keyEventInternal(mgp::Keyboard::KEY_RELEASE, getKey(wParam, shiftDown ^ capsOn));
}

void MgpView::closeEvent(QCloseEvent* evt)
{
    Game::exit();
}


