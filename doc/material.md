

## 材质

创建材质
```
//创建并赋值给model
_model->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "LIGHTING;PBR;IBL;LDR");

//直接创建
Material::create("res/shaders/colored.vert", "res/shaders/colored.frag", "LIGHTING;PBR;IBL;LDR");
```

只支持GLSL。第三个参数为shader里面的宏定义，中间用分号分隔。

### 材质参数

目前材质参数使用字符串定义和shader里面的变量名相同。常见的PBR参数如下：
```
material->getParameter("u_diffuseColor")->setVector4(Vector4(0.5, 0.0, 0.0, 1.0));
material->getParameter("u_albedo")->setVector3(Vector3(0.5, 0.0, 0.0));
material->getParameter("u_metallic")->setFloat(metalness);
material->getParameter("u_roughness")->setFloat(roughness);
material->getParameter("u_ao")->setFloat(1.0);
material->getParameter("u_emissive")->setVector3(Vector3(0.0, 0.0, 0.0));
```


### 纹理

```
Texture* sampler = material->getParameter("u_diffuseTexture")->setValue("res/image/crate.png", true);
sampler->setFilterMode(Texture::LINEAR_MIPMAP_LINEAR, Texture::LINEAR);
```


### 半透明
半透明效果需要设置多个地方：

1. 材质颜色alpha小于1.0， 例如：
```
material->getParameter("u_diffuseColor")->setVector4(Vector4(1.0, 0.5, 0.5, 0.4));
```

2. 启用混合
```
material->getStateBlock()->setBlend(true);
```

3. 在半透明层绘制
```
_model->setRenderLayer(Drawable::Transparent);
```
