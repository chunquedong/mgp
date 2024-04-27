

## 序列化

### 保存
```
    auto sceneWriter = mgp::SerializerJson::createWriter("test.json");
    sceneWriter->writeObject(nullptr, form->getContent());
    sceneWriter->close();
```

### 加载
```
    auto reader = mgp::Serializer::createReader("test.json");
    auto content = reader->readObject(nullptr).dynamicCastTo<mgp::Container>();
```

### HiML格式
```
    auto sceneWriter = mgp::SerializerJson::createWriter("ui.hml", true);
    sceneWriter->writeObject(nullptr, form->getContent());

    auto reader = mgp::Serializer::createReader("ui.hml", true);
    auto content = reader->readObject(nullptr).dynamicCastTo<mgp::Container>();
```

## 自定义序列化对象
### 继承Serializable
需要重写的方法：
```
    /**
     * Gets the class name string for the object.
     *
     * This is used by the Serializer when reading/writing objects.
     * The class name should be namespaced. Ex: mgp::SceneObject
     */
    virtual std::string getClassName() override;

    /**
     * Event handled when an object is asked to serialize itself.
     * 
     * @param serializer The serializer to write properties to.
     */
    virtual void onSerialize(Serializer* serializer) override;

    /**
     * Event handled when an object properties are being deserialized.
     *
     * @param serializer The serializer to read properties from.
     */
    virtual void onDeserialize(Serializer* serializer) override;

```

### 注册函数
需要注册的方法：
```
    static Serializable *createObject();

    static std::string enumToString(const std::string& enumName, int value);

    static int enumParse(const std::string& enumName, const std::string& str);
```
在启动时注册给SerializerManager（Application类中）
```
    mgr->registerType("mgp::Camera", Camera::createObject);
    mgr->registerEnum("mgp::Camera::Mode", Camera::enumToString, Camera::enumParse);
```
