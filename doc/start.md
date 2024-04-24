
## 绘制三角形

```
class MainApp : public Application {
    void initialize() {

        // 创建空场景
        UPtr<Scene> _scene = Scene::create();

        // 场景三角形网格
        UPtr<Mesh> mesh = createTriangleMesh();

        // 创建模型
        UPtr<Model> _model = Model::create(std::move(mesh));

        // 添加材质
        Material *material = _model->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "VERTEX_COLOR");
        material->getStateBlock()->setCullFace(false);

        // 创建结点并将模型关联到结点
        Node* modelNode = _scene->addNode("model");
        modelNode->setDrawable(_model.dynamicCastTo<Drawable>());

        // 设置主场景
        getView()->setScene(std::move(_scene));

        // 初始化相机位置
        getView()->initCamera(false, 0.01);
    }
};

int main() {
    printf("main start\n");
    MainApp* instance = new MainApp();
    return Platform::run();
}

```

创建三角形的代码

```
static UPtr<Mesh> createTriangleMesh()
{
    // 计算三角形三个点的位置
    float a = 0.5f;     // length of the side
    Vector2 p1(0.0f, a / sqrtf(3.0f));
    Vector2 p2(-a / 2.0f, -a / (2.0f * sqrtf(3.0f)));
    Vector2 p3(a / 2.0f, -a / (2.0f * sqrtf(3.0f)));

    // 创建顶点. 每个顶点有位置和颜色 position (x, y, z) and color (red, green, blue)
    float vertices[] =
    {
        (float)p1.x, (float)p1.y, 0.0f,     1.0f, 0.0f, 0.0f,
        (float)p2.x, (float)p2.y, 0.0f,     0.0f, 1.0f, 0.0f,
        (float)p3.x, (float)p3.y, 0.0f,     0.0f, 0.0f, 1.0f,
    };
    unsigned int vertexCount = 3;
    
    //创建顶点格式
    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::COLOR, 3)
    };

    //创建网格
    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 2), vertexCount);
    if (mesh.isNull())
    {
        GP_ERROR("Failed to create mesh.");
        return UPtr<Mesh>(NULL);
    }

    //设置形状为三角形
    mesh->setPrimitiveType(Mesh::TRIANGLES);

    //将顶点数据设置给mesh
    mesh->getVertexBuffer()->setData((char*)vertices, sizeof(vertices));
    return mesh;
}

```

