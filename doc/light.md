
## 灯光

支持方向光、点光源、聚光灯。目前只有方向光支持产生阴影。
灯光颜色值可以大于1.0

```
    Node* addLight(Scene* _scene, Vector3 eyePosition, Vector3 lookTarget) {
        //创建灯光
        UPtr<Light> directionalLight = Light::createDirectional(Vector3(10.0, 10.0, 10.0));

        //创建结点
        UPtr<Node> _directionalLightNode = Node::create("directionalLight");
        _directionalLightNode->setLight(std::move(directionalLight));

        //设置位置和方向
        Matrix m;
        Matrix::createLookAt(eyePosition, lookTarget, Vector3::unitY(), &m, false);
        _directionalLightNode->setMatrix(m);

        //添加到场景
        Node* node = _directionalLightNode.get();
        _scene->addNode(std::move(_directionalLightNode));
        return node;
    }
```

## 灯光作用范围

物体和灯光多有setLightMask方法，只有在同一组的灯光才照亮物体。
