

## 加载GLTF模型

只支持gltf格式导入，其他格式模型转换为gltf格式数据。

```
    GltfLoader loader;
    loader.lighting = true; //是否启用光照
    auto _scene = loader.load("res/gltf/car.gltf");
```

默认构建不支持draco压缩，如果需要draco支持，需要使用fmake_full.props来构建。

## 动画

支持骨骼蒙皮动画和变形动画。有两种方式运行动画。

1.通过模型获取：
```
    Node* node = _scene->findNode("Cesium_Man");
    Model* model = dynamic_cast<Model*>(node->getDrawable());

    Animation* animation = model->getSkin()->getRootJoint()->getAnimation();
    AnimationClip* clip = animation->getClip();
    clip->setRepeatCount(AnimationClip::REPEAT_INDEFINITE);
    clip->play();
```

2.通过场景获取
```
    if (_scene->getAnimations().size() > 0) {
        Animation* animation = _scene->getAnimations()[0];
        AnimationClip* clip = animation->getClip();
        clip->setRepeatCount(AnimationClip::REPEAT_INDEFINITE);
        clip->play();
    }
```

默认情况下，骨骼节点是不加入主场景中的。

## 实例化渲染

通过clone方法创建一个模型的多个实例

```
    NodeCloneContext ctx;
    UPtr<Drawable> _model2 = model->clone(ctx);

    Node* modelNode2 = _scene->addNode("model2");
    modelNode2->setDrawable(std::move(_model2));
    modelNode2->translate(1, 0, 0);
```
