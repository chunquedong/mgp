#include "base/Base.h"
#include "Model.h"
#include "Scene.h"
#include "material/Material.h"
#include "scene/Node.h"
#include "scene/Renderer.h"
#include "AssetManager.h"

namespace mgp
{

Model::Model() : Drawable(),
    _mesh(NULL), _material(NULL), _skin(NULL), _lodLimit(1000)
{
}

Model::Model(UPtr<Mesh> mesh) : Drawable(),
    _mesh(std::move(mesh)), _material(NULL), _skin(NULL), _lodLimit(1000)
{
    GP_ASSERT(_mesh.get());
}

Model::~Model()
{
    /*SAFE_RELEASE(_material);

    for (unsigned int i = 0; i < _partMaterials.size(); ++i)
    {
        SAFE_RELEASE(_partMaterials[i]);
    }*/
    _partMaterials.clear();

    /*SAFE_RELEASE(_mesh);
    SAFE_RELEASE(_skin);*/
}

UPtr<Model> Model::create(UPtr<Mesh> mesh)
{
    GP_ASSERT(mesh.get());
    //mesh->addRef();
    return UPtr<Model>(new Model(std::move(mesh)));
}

Mesh* Model::getMesh() const
{
    return _mesh.get();
}

unsigned int Model::getMeshPartCount() const
{
    GP_ASSERT(_mesh.get());
    return _mesh->getPartCount();
}

Material* Model::getMaterial(int partIndex)
{
    GP_ASSERT(partIndex == -1 || partIndex >= 0);

    Material* m = NULL;

    if (partIndex < 0)
        return _material.get();
    if (partIndex >= (int)_partMaterials.size())
        return NULL;

    // Look up explicitly specified part material.
    if (_partMaterials.size() > partIndex)
    {
        m = _partMaterials[partIndex].get();
    }
    if (m == NULL)
    {
        // Return the shared material.
         m = _material.get();
    }

    return m;
}

Material* Model::getMainMaterial() const {
    if (_partMaterials.size() > 0) {
        return _partMaterials[0].get();
    }
    return _material.get();
}

void Model::setMaterial(UPtr<Material> material, int partIndex)
{
    GP_ASSERT(partIndex == -1 || (partIndex >= 0 && partIndex < (int)getMeshPartCount()));

    if (partIndex == -1)
    {
        _material = std::move(material);
    }
    else if (partIndex >= 0 && partIndex < (int)getMeshPartCount())
    {
        // Ensure mesh part count is up-to-date.
        validatePartCount();

        // Release existing part material and part binding.
        if (_partMaterials.size() > partIndex)
        {
        }
        else
        {
            // Allocate part arrays for the first time.
            while (_partMaterials.size() < getMeshPartCount())
            {
                _partMaterials.resize(getMeshPartCount());
            }
        }

        _partMaterials[partIndex] = std::move(material);
    }
}

Material* Model::setMaterial(const char* vshPath, const char* fshPath, const char* defines, int partIndex)
{
    // Try to create a Material with the given parameters.
    UPtr<Material> material = Material::create(vshPath, fshPath, defines);
    if (material.get() == NULL)
    {
        GP_ERROR("Failed to create material for model.");
        return NULL;
    }

    // Assign the material to us.
    setMaterial(std::move(material), partIndex);

    // Release the material since we now have a reference to it.
    //material->release();

    return getMaterial(partIndex);
}

Material* Model::setMaterial(const char* materialPath, int partIndex)
{
    // Try to create a Material from the specified material file.
    UPtr<Material> material = Material::create(materialPath);
    if (material.get() == NULL)
    {
        GP_ERROR("Failed to create material for model.");
        return NULL;
    }

    // Assign the material to us
    setMaterial(std::move(material), partIndex);

    // Release the material since we now have a reference to it
    //material->release();

    return getMaterial(partIndex);
}

bool Model::hasMaterial(unsigned int partIndex) const
{
    return (partIndex < _partMaterials.size() && _partMaterials[partIndex].get());
}

MeshSkin* Model::getSkin() const
{
    return _skin.get();
}

void Model::setSkin(UPtr<MeshSkin> skin)
{
    _skin = std::move(skin);
}

void Model::setNode(Node* node)
{
    Drawable::setNode(node);

    // Re-bind node related material parameters
    //if (node)
    //{
    //    if (_material)
    //    {
    //       setMaterialNodeBinding(_material);
    //    }

    //    for (unsigned int i = 0; i < _partMaterials.size(); ++i)
    //    {
    //        if (_partMaterials[i])
    //        {
    //            setMaterialNodeBinding(_partMaterials[i]);
    //        }
    //    }

    //}
}

unsigned int Model::draw(RenderInfo* view)
{
    GP_ASSERT(_mesh.get());
    int pmsize = _partMaterials.size();
    if (pmsize < 128) {
        Material* partMaterials[128] = { 0 };
        for (int i = 0; i < pmsize; ++i) {
            partMaterials[i] = _partMaterials[i].get();
        }
        return _mesh->draw(view, this, _material.get(), partMaterials, _partMaterials.size());
    }
    else {
        std::vector<Material*> partMaterials(pmsize);
        for (int i = 0; i < pmsize; ++i) {
            partMaterials[i] = _partMaterials[i].get();
        }
        return _mesh->draw(view, this, _material.get(), partMaterials.data(), _partMaterials.size());
    }
}

//void Model::setMaterialNodeBinding(Material *material)
//{
//    GP_ASSERT(material);
//
//    if (_node)
//    {
//        material->setNodeBinding(getNode());
//    }
//}

UPtr<Drawable> Model::clone(NodeCloneContext& context)
{
    UPtr<Model> model = Model::create(uniqueFromInstant(getMesh()));
    if (!model.get())
    {
        GP_ERROR("Failed to clone model.");
        return UPtr<Drawable>(NULL);
    }

    if (getSkin())
    {
        model->setSkin(getSkin()->clone(context));
    }
    if (getMaterial())
    {
        /*UPtr<Material> materialClone = getMaterial()->clone(context);
        if (!materialClone.get())
        {
            GP_ERROR("Failed to clone material for model.");
            return model.dynamicCastTo<Drawable>();
        }*/
        model->setMaterial(uniqueFromInstant(getMaterial()));
        //materialClone->release();
    }

    GP_ASSERT(getMeshPartCount() == model->getMeshPartCount());
    for (unsigned int i = 0; i < _partMaterials.size(); ++i)
    {
        if (_partMaterials[i].get())
        {
            //UPtr<Material> materialClone = _partMaterials[i]->clone(context);
            model->setMaterial(uniqueFromInstant(_partMaterials[i].get()), i);
            //materialClone->release();
        }
    }

    return model.dynamicCastTo<Drawable>();
}

void Model::validatePartCount()
{
    GP_ASSERT(_mesh.get());
    unsigned int partCount = _mesh->getPartCount();

    if (_partMaterials.size() != partCount)
    {
        _partMaterials.resize(partCount);
    }
}

bool Model::doRaycast(RayQuery& query) {
    GP_ASSERT(_mesh.get());
    return _mesh->doRaycast(query);
}

const BoundingSphere* Model::getBoundingSphere()
{
    GP_ASSERT(_mesh.get());
    return &_mesh->getBoundingSphere();
}

Serializable* Model::createObject() {
    return new Model();
}


std::string Model::getClassName() {
    return "mgp::Model";
}


void Model::onSerialize(Serializer* serializer) {
    serializer->writeInt("renderLayer", this->getRenderLayer(), 0);
    serializer->writeInt("lightMask", this->getLightMask(), 0);

    if (_mesh.get()) {
        AssetManager::getInstance()->save(_mesh.get());
    }
    if (_mesh.get()) serializer->writeString("mesh", _mesh->getId().c_str(), "");
    else serializer->writeString("mesh", "", "");
        
    if (_skin.get()) {
        AssetManager::getInstance()->save(_skin.get());
    }
    if (_skin.get()) serializer->writeString("skin", _skin->getId().c_str(), "");
    else serializer->writeString("skin", "", "");

    if (_material.get()) {
        AssetManager::getInstance()->save(_material.get());
    }
    if (_material.get()) serializer->writeString("material", _material->getId().c_str(), "");
    else serializer->writeString("material", "", "");

    serializer->writeList("partMaterials", _partMaterials.size());
    for (int i=0; i<_partMaterials.size(); ++i) {
        if (_partMaterials[i].get()) {
            AssetManager::getInstance()->save(_partMaterials[i].get());
            serializer->writeString(NULL, _partMaterials[i]->getId().c_str(), NULL);
        }
        else serializer->writeString(NULL, "", NULL);
    }
    serializer->finishColloction();
}

void Model::onDeserialize(Serializer* serializer) {
    setRenderLayer((Drawable::RenderLayer)serializer->readInt("renderLayer", 0));
    setLightMask(serializer->readInt("lightMask", 0));

    std::string mesh;
    serializer->readString("mesh", mesh, "");
    if (mesh.size() > 0) {
        _mesh = AssetManager::getInstance()->load<Mesh>(mesh, AssetManager::rt_mesh);
    }

    std::string skin;
    serializer->readString("skin", skin, "");
    if (skin.size() > 0) {
        _skin = AssetManager::getInstance()->load<MeshSkin>(skin, AssetManager::rt_skin);
    }

    std::string material;
    serializer->readString("material", material, "");
    if (material.size() > 0) {
        _material = AssetManager::getInstance()->load<Material>(material, AssetManager::rt_materail);
    }

    int materialsCount = serializer->readList("partMaterials");
    for (int i=0; i<materialsCount; ++i) {
        std::string material;
        serializer->readString(NULL, material, NULL);
        if (material.size() > 0) {
            UPtr<Material> m = AssetManager::getInstance()->load<Material>(material, AssetManager::rt_materail);
            if (m.get()) this->setMaterial(std::move(m), i);
        }
    }
    serializer->finishColloction();
}


LodModel::LodModel() {

}

LodModel::~LodModel() {
    _lods.clear();
}

unsigned int LodModel::draw(RenderInfo *view) {
    if (_lods.size() == 0) return 0;
    Vector3 pos = getNode()->getTranslationWorld();
    Vector3 cpos = view->camera->getNode()->getTranslationWorld();
    double dis = pos.distance(cpos);
    for (UPtr<Model>& model : _lods) {
        if (dis < model->getLodLimit()) {
            return model->draw(view);
        }
    }
    return 0;
}

}
