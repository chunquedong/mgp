#include "base/Base.h"
#include "scene/Node.h"
//#include "audio/AudioSource.h"
#include "Scene.h"
#include "BoneJoint.h"
//#include "physics/PhysicsRigidBody.h"
//#include "physics/PhysicsVehicle.h"
//#include "physics/PhysicsVehicleWheel.h"
//#include "physics/PhysicsGhostObject.h"
//#include "physics/PhysicsCharacter.h"
#include "objects/Terrain.h"
#include "platform/Toolkit.h"
#include "Drawable.h"
//#include "ui/Form.h"
#include "base/Ref.h"
#include "material/MaterialParameter.h"

#define SCENEOBJECT_NAME ""
#define SCENEOBJECT_STATIC true
#define SCENEOBJECT_ENABLED true
#define SCENEOBJECT_POSITION Vector3::zero()
#define SCENEOBJECT_EULER_ANGLES Vector3::zero()
#define SCENEOBJECT_SCALE Vector3::one()

// Node dirty flags
#define NODE_DIRTY_WORLD 1
#define NODE_DIRTY_BOUNDS 2
#define NODE_DIRTY_HIERARCHY 4
#define NODE_DIRTY_ALL (NODE_DIRTY_WORLD | NODE_DIRTY_BOUNDS | NODE_DIRTY_HIERARCHY)

namespace mgp
{

Node::Node(const char* id)
    : _scene(NULL), _parent(NULL), _enabled(true), _tags(NULL),
    _userObject(NULL),
    _dirtyBits(NODE_DIRTY_ALL), _static(false), 
    _childCount(0), _prevSibling(NULL)
{
#ifdef GP_SCRIPT
    GP_REGISTER_SCRIPT_EVENTS();
#endif
    if (id)
    {
        _name = id;
    }
}

Node::~Node()
{
    if (getDrawable())
        getDrawable()->setNode(NULL);
    //if (getAudioSource())
    //    getAudioSource()->setNode(NULL);

    //SAFE_RELEASE(_userObject);
    SAFE_DELETE(_tags);
    //setAgent(NULL);

    /*for (Component *com : _components) {
        com->setNode(NULL);
        Refable*ref = dynamic_cast<Refable*>(com);
        if (ref) {
            SAFE_RELEASE(ref);
        } else {
            SAFE_DELETE(com);
        }
    }*/
    _components.clear();

    for (Node* child = getFirstChild(); child != NULL; child = child->getNextSibling()) {
        child->_parent = NULL;
        child->remove();
    }
    _firstChild.clear();
    _nextSibling.clear();
    _prevSibling = NULL;
    _parent = NULL;
}

UPtr<Node> Node::create(const char* id)
{
    return UPtr<Node>(new Node(id));
}

UPtr<Node> Node::createForComponent(UPtr<Component> comp, const char* id) {
    UPtr<Node> node(new Node(id));
    node->addComponent(std::move(comp));
    return node;
}

const char* Node::getTypeName() const
{
    return "Node";
}

const char* Node::getName() const
{
    return _name.c_str();
}

void Node::setName(const char* id)
{
    if (id)
    {
        _name = id;
    }
}

Node::Type Node::getType() const
{
    return Node::NODE;
}

void Node::insertChild(UPtr<Node> child) {
    GP_ASSERT(child.get());

    if (child->_parent == this)
    {
        // This node is already present in our hierarchy
        return;
    }

    // If the item belongs to another hierarchy, remove it first.
    if (child->_parent)
    {
        child->_parent->removeChild(child.get());
    }

    child->_parent = this;

    if (_firstChild.get())
    {
        _firstChild->_prevSibling = child.get();
        child->_nextSibling = std::move(_firstChild);
        _firstChild = std::move(child);
    }
    else
    {
        _firstChild = std::move(child);
    }

    ++_childCount;

    setBoundsDirty();

    if (_dirtyBits & NODE_DIRTY_HIERARCHY)
    {
        hierarchyChanged();
    }
}

void Node::addChild(UPtr<Node> child)
{
    GP_ASSERT(child.get());

    if (child->_parent == this)
    {
        // This node is already present in our hierarchy
        return;
    }
    //child->addRef();

    // If the item belongs to another hierarchy, remove it first.
    if (child->_parent)
    {
        child->_parent->removeChild(child.get());
    }

    child->_parent = this;

    // Add child to the end of the list.
    // NOTE: This is different than the original behavior which inserted nodes
    // into the beginning of the list. Although slightly slower to add to the
    // end of the list, it makes scene traversal and drawing order more
    // predictable, so I've changed it.
    if (_firstChild.get())
    {
        Node* n = _firstChild.get();
        while (n->_nextSibling.get())
            n = n->_nextSibling.get();

        child->_prevSibling = n;
        n->_nextSibling = std::move(child);
    }
    else
    {
        _firstChild = std::move(child);
    }

    ++_childCount;

    setBoundsDirty();

    if (_dirtyBits & NODE_DIRTY_HIERARCHY)
    {
        hierarchyChanged();
    }
}

UPtr<Node> Node::removeChild(Node* child)
{
    if (child == NULL || child->_parent != this)
    {
        // The child is not in our hierarchy.
        return UPtr<Node>();
    }
    return child->remove();
    //SAFE_RELEASE(child);
}

void Node::removeAllChildren()
{
    _dirtyBits &= ~NODE_DIRTY_HIERARCHY;
    while (_firstChild.get())
    {
        removeChild(_firstChild.get());
    }
    _dirtyBits |= NODE_DIRTY_HIERARCHY;
    hierarchyChanged();
}

UPtr<Node> Node::remove()
{
    auto res = uniqueFromInstant(this);

    // Re-link our neighbours.
    if (_nextSibling.get())
    {
        _nextSibling->_prevSibling = _prevSibling;
    }
    if (_prevSibling)
    {
        _prevSibling->_nextSibling = std::move(_nextSibling);
    }
    
    // Update our parent.
    Node* parent = _parent;
    if (parent)
    {
        if (this == parent->_firstChild.get())
        {
            //delete this
            parent->_firstChild = std::move(_nextSibling);
        }
        --parent->_childCount;
    }
    _nextSibling.clear();
    _prevSibling = NULL;
    _parent = NULL;

    if (parent && parent->_dirtyBits & NODE_DIRTY_HIERARCHY)
    {
        parent->hierarchyChanged();
    }
    
    return res;
}

Node* Node::getFirstChild() const
{
    return _firstChild.get();
}

Node* Node::getNextSibling() const
{
    return _nextSibling.get();
}

Node* Node::getPreviousSibling() const
{
    return _prevSibling;
}

Node* Node::getParent() const
{
    return _parent;
}

unsigned int Node::getChildCount() const
{
    return _childCount;
}

Node* Node::getRootNode() const
{
    Node* n = const_cast<Node*>(this);
    while (n->getParent())
    {
        n = n->getParent();
    }
    return n;
}

Node* Node::findNode(const char* id, bool recursive, bool exactMatch) const
{
    return findNode(id, recursive, exactMatch, false);
}

Node* Node::findNode(const char* id, bool recursive, bool exactMatch, bool skipSkin) const
{
    GP_ASSERT(id);

    // If not skipSkin hierarchy, try searching the skin hierarchy
    if (!skipSkin)
    {
        // If the drawable is a model with a mesh skin, search the skin's hierarchy as well.
        Node* rootNode = NULL;
        Model* model = dynamic_cast<Model*>(getDrawable());
        if (model)
        {
            if (model->getSkin() != NULL && (rootNode = model->getSkin()->_rootNode) != NULL)
            {
                if ((exactMatch && rootNode->_name == id) || (!exactMatch && rootNode->_name.find(id) == 0))
                    return rootNode;

                Node* match = rootNode->findNode(id, true, exactMatch, true);
                if (match)
                {
                    return match;
                }
            }
        }
    }
    // Search immediate children first.
    for (Node* child = getFirstChild(); child != NULL; child = child->getNextSibling()) {
        // Does this child's ID match?
        if ((exactMatch && child->_name == id) || (!exactMatch && child->_name.find(id) == 0))
        {
            return child;
        }
    }
    // Recurse.
    if (recursive)
    {
        for (Node* child = getFirstChild(); child != NULL; child = child->getNextSibling()) {
            Node* match = child->findNode(id, true, exactMatch, skipSkin);
            if (match)
            {
                return match;
            }
        }
    }
    return NULL;
}

unsigned int Node::findNodes(const char* id, std::vector<Node*>& nodes, bool recursive, bool exactMatch) const
{
    return findNodes(id, nodes, recursive, exactMatch, false);
}

unsigned int Node::findNodes(const char* id, std::vector<Node*>& nodes, bool recursive, bool exactMatch, bool skipSkin) const
{
    GP_ASSERT(id);

    // If the drawable is a model with a mesh skin, search the skin's hierarchy as well.
    unsigned int count = 0;

    if (!skipSkin)
    {
        Node* rootNode = NULL;
        Model* model = dynamic_cast<Model*>(getDrawable());
        if (model)
        {
            if (model->getSkin() != NULL && (rootNode = model->getSkin()->_rootNode) != NULL)
            {
                if ((exactMatch && rootNode->_name == id) || (!exactMatch && rootNode->_name.find(id) == 0))
                {
                    nodes.push_back(rootNode);
                    ++count;
                }
                count += rootNode->findNodes(id, nodes, recursive, exactMatch, true);
            }
        }
    }

    // Search immediate children first.
    for (Node* child = getFirstChild(); child != NULL; child = child->getNextSibling()) {
        // Does this child's ID match?
        if ((exactMatch && child->_name == id) || (!exactMatch && child->_name.find(id) == 0))
        {
            nodes.push_back(child);
            ++count;
        }
    }
    // Recurse.
    if (recursive)
    {
        for (Node* child = getFirstChild(); child != NULL; child = child->getNextSibling()) {
            count += child->findNodes(id, nodes, recursive, exactMatch, skipSkin);
        }
    }

    return count;
}

void Node::getAllDrawable(std::vector<Drawable*> &list) {
    auto d = getDrawable();
    if (d) list.push_back(d);

    // Recurse.
    for (Node* child = getFirstChild(); child != NULL; child = child->getNextSibling()) {
        child->getAllDrawable(list);
    }
}

Scene* Node::getScene() const
{
    if (_scene)
        return _scene;

    // Search our parent for the scene
    if (_parent)
    {
        Scene* scene = _parent->getScene();
        if (scene)
            return scene;
    }
    return NULL;
}

bool Node::hasTag(const char* name) const
{
    GP_ASSERT(name);
    return (_tags ? _tags->find(name) != _tags->end() : false);
}

const char* Node::getTag(const char* name) const
{
    GP_ASSERT(name);

    if (!_tags)
        return NULL;

    std::map<std::string, std::string>::const_iterator itr = _tags->find(name);
    return (itr == _tags->end() ? NULL : itr->second.c_str());
}

void Node::setTag(const char* name, const char* value)
{
    GP_ASSERT(name);

    if (value == NULL)
    {
        // Removing tag
        if (_tags)
        {
            _tags->erase(name);
            if (_tags->size() == 0)
            {
                SAFE_DELETE(_tags);
            }
        }
    }
    else
    {
        // Setting tag
        if (_tags == NULL)
        {
            _tags = new std::map<std::string, std::string>();
        }
        (*_tags)[name] = value;
    }
}

void Node::setEnabled(bool enabled)
{
    if (_enabled != enabled)
    {
        /*if (getCollisionObject())
        {
            getCollisionObject()->setEnabled(enabled);
        }*/
        _enabled = enabled;
    }
}

bool Node::isEnabled() const
{
    return _enabled;
}

bool Node::isEnabledInHierarchy() const
{
    if (!_enabled)
       return false;

   Node* node = _parent;
   while (node)
   {
       if (!node->_enabled)
       {
           return false;
       }
       node = node->_parent;
   }
   return true;
}

void Node::update(float elapsedTime)
{
    for (Node* child = getFirstChild(); child != NULL; child = child->getNextSibling()) {
        if (child->isEnabled())
        {
            child->update(elapsedTime);
        }
    }

    for (auto it = _components.begin(); it != _components.end(); ++it) {
        Drawable *d = dynamic_cast<Drawable*>(it->get());
        if (d) d->update(elapsedTime);
    }
#ifdef GP_SCRIPT
    fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(Node, update), dynamic_cast<void*>(this), elapsedTime);
#endif
}

bool Node::isStatic() const
{
    //auto _collisionObject = getCollisionObject();
    //return (_collisionObject && _collisionObject->isStatic());
    return false;
}

const Matrix& Node::getWorldMatrix() const
{
    if (_dirtyBits & NODE_DIRTY_WORLD)
    {
        // Clear our dirty flag immediately to prevent this block from being entered if our
        // parent calls our getWorldMatrix() method as a result of the following calculations.
        _dirtyBits &= ~NODE_DIRTY_WORLD;

        if (!isStatic())
        {
            // If we have a parent, multiply our parent world transform by our local
            // transform to obtain our final resolved world transform.
            Node* parent = getParent();
            //auto _collisionObject = getCollisionObject();
            //if (parent && (!_collisionObject || _collisionObject->isKinematic()))
            if (parent)
            {
                Matrix::multiply(parent->getWorldMatrix(), getMatrix(), &_world);
            }
            else
            {
                _world = getMatrix();
            }

            // Our world matrix was just updated, so call getWorldMatrix() on all child nodes
            // to force their resolved world matrices to be updated.
            /*for (size_t i=0; i<_children.size(); ++i) {
                Node *child = _children[i];
                child->getWorldMatrix();
            }*/
        }
    }
    return _world;
}

const Matrix& Node::getInverseTransposeWorldMatrix() const
{
    static Matrix invTransWorld;
    invTransWorld = getWorldMatrix();
    invTransWorld.invert();
    invTransWorld.transpose();
    return invTransWorld;
}

Vector3 Node::getTranslationWorld() const
{
    Vector3 translation;
    getWorldMatrix().getTranslation(&translation);
    return translation;
}

void Node::setWorldPosition(Vector3& v)
{
    Vector3 translation;
    getWorldMatrix().getTranslation(&translation);
    Vector3 d = v - translation;
    translate(d);
}

Vector3 Node::getForwardVectorWorld() const
{
    Vector3 vector;
    getWorldMatrix().getForwardVector(&vector);
    return vector;
}

Vector3 Node::getRightVectorWorld() const
{
    Vector3 vector;
    getWorldMatrix().getRightVector(&vector);
    return vector;
}

Vector3 Node::getUpVectorWorld() const
{
    Vector3 vector;
    getWorldMatrix().getUpVector(&vector);
    return vector;
}

void Node::hierarchyChanged()
{
    // When our hierarchy changes our world transform is affected, so we must dirty it.
    _dirtyBits |= NODE_DIRTY_HIERARCHY;
    transformChanged();
}

void Node::transformChanged()
{
    // Our local transform was changed, so mark our world matrices dirty.
    _dirtyBits |= NODE_DIRTY_WORLD | NODE_DIRTY_BOUNDS;

    // Notify our children that their transform has also changed (since transforms are inherited).
    for (Node* child = getFirstChild(); child != NULL; child = child->getNextSibling()) {
        if (Transform::isTransformChangedSuspended())
        {
            // If the DIRTY_NOTIFY bit is not set
            if (!child->isDirty(Transform::DIRTY_NOTIFY))
            {
                child->transformChanged();
                suspendTransformChange(child);
            }
        }
        else
        {
            child->transformChanged();
        }
    }
    Transform::transformChanged();
}

void Node::setBoundsDirty()
{
    // Mark ourself and our parent nodes as dirty
    _dirtyBits |= NODE_DIRTY_BOUNDS;

    // Mark our parent bounds as dirty as well
    if (_parent)
        _parent->setBoundsDirty();
}

Animation* Node::getAnimation(const char* id) const
{
    Animation* animation = ((AnimationTarget*)this)->getAnimation(id);
    if (animation)
        return animation;
    
    // See if this node has a model, then drill down.
    Model* model = dynamic_cast<Model*>(getDrawable());
    if (model)
    {
        // Check to see if there's any animations with the ID on the joints.
        MeshSkin* skin = model->getSkin();
        if (skin)
        {
            Node* rootNode = skin->_rootNode;
            if (rootNode)
            {
                animation = rootNode->getAnimation(id);
                if (animation)
                    return animation;
            }
        }

        // Check to see if any of the model's material parameter's has an animation
        // with the given ID.
        Material* material = model->getMaterial();
        if (material)
        {
            // How to access material parameters? hidden on the Material::MaterialParamBinding.
            std::vector<MaterialParameter*>::iterator itr = material->_parameters.begin();
            for (; itr != material->_parameters.end(); itr++)
            {
                GP_ASSERT(*itr);
                animation = ((MaterialParameter*)(*itr))->getAnimation(id);
                if (animation)
                    return animation;
            }
        }
    }

    // look through form for animations.
    Drawable* form = (getDrawable());
    if (form)
    {
        animation = form->getAnimation(id);
        if (animation)
            return animation;
    }

    // Look through this node's children for an animation with the specified ID.
    for (Node* child = getFirstChild(); child != NULL; child = child->getNextSibling()) {
        animation = child->getAnimation(id);
        if (animation)
            return animation;
    }
    
    return NULL;
}

Camera* Node::getCamera() const
{
    return getComponent<Camera>();
}

void Node::setCamera(UPtr<Camera> camera)
{
    addComponent<Camera>(std::move(camera));
}

Light* Node::getLight() const
{
    return getComponent<Light>();
}

void Node::setLight(UPtr<Light> light)
{
    addComponent<Light>(std::move(light));
}

Drawable* Node::getDrawable() const
{
    return getComponent<Drawable>();
}

void Node::setDrawable(UPtr<Drawable> drawable)
{
    addComponent<Drawable>(std::move(drawable));
}

//PhysicsCollisionObject* Node::getCollisionObject() const {
//    return getComponent<PhysicsCollisionObject>();
//}

const BoundingSphere& Node::getBoundingSphere() const
{
    if (_dirtyBits & NODE_DIRTY_BOUNDS)
    {
        _dirtyBits &= ~NODE_DIRTY_BOUNDS;

        const Matrix& worldMatrix = getWorldMatrix();

        // Start with our local bounding sphere
        // TODO: Incorporate bounds from entities other than mesh (i.e. particleemitters, audiosource, etc)
        auto _drawable = getDrawable();
        auto _light = getLight();
        auto _camera = getCamera();
        bool empty = true;
        Terrain* terrain = dynamic_cast<Terrain*>(_drawable);
        if (terrain)
        {
            _bounds.set(terrain->getBoundingBox());
            empty = false;
        }
        else if (_drawable) {
            auto bs = _drawable->getBoundingSphere();
            if (bs && !bs->isEmpty()) {
                if (empty) {
                    _bounds.set(*bs);
                    empty = false;
                }
                else {
                    _bounds.merge(*bs);
                }
            }
        }

        if (_light)
        {
            switch (_light->getLightType())
            {
            case Light::POINT:
                if (empty)
                {
                    _bounds.set(Vector3::zero(), _light->getRange());
                    empty = false;
                }
                else
                {
                    _bounds.merge(BoundingSphere(Vector3::zero(), _light->getRange()));
                }
                break;
            case Light::SPOT:
                // TODO: Implement spot light bounds
                break;
            }
        }
        if (empty)
        {
            // Empty bounding sphere, set the world translation with zero radius
            worldMatrix.getTranslation(&_bounds.center);
            _bounds.radius = 0;
        }

        // Transform the sphere (if not empty) into world space.
        if (!empty)
        {
            bool applyWorldTransform = true;
            Model* model = dynamic_cast<Model*>(_drawable);
            if (model && model->getSkin())
            {
                // Special case: If the root joint of our mesh skin is parented by any nodes, 
                // multiply the world matrix of the root joint's parent by this node's
                // world matrix. This computes a final world matrix used for transforming this
                // node's bounding volume. This allows us to store a much smaller bounding
                // volume approximation than would otherwise be possible for skinned meshes,
                // since joint parent nodes that are not in the matrix palette do not need to
                // be considered as directly transforming vertices on the GPU (they can instead
                // be applied directly to the bounding volume transformation below).
                GP_ASSERT(model->getSkin()->getRootJoint());
                Node* jointParent = model->getSkin()->getRootJoint()->getParent();
                if (jointParent)
                {
                    // TODO: Should we protect against the case where joints are nested directly
                    // in the node hierachy of the model (this is normally not the case)?
                    Matrix boundsMatrix;
                    Matrix::multiply(getWorldMatrix(), jointParent->getWorldMatrix(), &boundsMatrix);
                    _bounds.transform(boundsMatrix);
                    applyWorldTransform = false;
                }
            }
            if (applyWorldTransform)
            {
                _bounds.transform(getWorldMatrix());
            }
        }

        // Merge this world-space bounding sphere with our childrens' bounding volumes.
        for (Node* child = getFirstChild(); child != NULL; child = child->getNextSibling()) {
            const BoundingSphere& childSphere = child->getBoundingSphere();
            if (!childSphere.isEmpty())
            {
                if (empty)
                {
                    _bounds.set(childSphere);
                    empty = false;
                }
                else
                {
                    _bounds.merge(childSphere);
                }
            }
        }
    }

    return _bounds;
}

void Node::moveChildrenTo(Node* that) {
    while (this->_firstChild.get())
    {
        auto child = this->removeChild(this->_firstChild.get());
        that->addChild(std::move(child));
    }
}

UPtr<Node> Node::clone() const
{
    NodeCloneContext context;
    return UPtr<Node>(cloneRecursive(context));
}

UPtr<Node> Node::cloneSingleNode(NodeCloneContext &context) const
{
    UPtr<Node> copy = Node::create(getName());
    context.registerClonedNode(this, copy.get());
    cloneInto(copy.get(), context);
    return copy;
}

UPtr<Node> Node::cloneRecursive(NodeCloneContext &context) const
{
    UPtr<Node> copy = cloneSingleNode(context);
    GP_ASSERT(copy.get());

    // Add child nodes
    for (Node* child = getFirstChild(); child != NULL; child = child->getNextSibling()) {
        UPtr<Node> childCopy = child->cloneRecursive(context);
        GP_ASSERT(childCopy.get());
        copy->addChild(std::move(childCopy));
        //childCopy->release();
    }

    return copy;
}

void Node::cloneInto(Node* node, NodeCloneContext& context) const
{
    GP_ASSERT(node);

    Transform::cloneInto(node, context);

    if (Drawable* drawable = getDrawable())
    {
        UPtr<Drawable> clone = drawable->clone(context);
        node->setDrawable(std::move(clone));
        /*Refable* ref = dynamic_cast<Refable*>(clone);
        if (ref)
            ref->release();*/
    }
    if (Camera* camera = getCamera())
    {
        UPtr<Camera> clone = camera->clone(context);
        node->setCamera(std::move(clone));
        /*Refable* ref = dynamic_cast<Refable*>(clone);
        if (ref)
            ref->release();*/
    }
    if (Light* light = getLight())
    {
        UPtr<Light> clone = light->clone(context);
        node->setLight(std::move(clone));
        /*Refable* ref = dynamic_cast<Refable*>(clone);
        if (ref)
            ref->release();*/
    }
    /*if (AudioSource* audio = getAudioSource())
    {
        AudioSource* clone = audio->clone(context);
        node->setAudioSource(clone);
        Ref* ref = dynamic_cast<Ref*>(clone);
        if (ref)
            ref->release();
    }*/
    if (_tags)
    {
        node->_tags = new std::map<std::string, std::string>(_tags->begin(), _tags->end());
    }

    node->_world = _world;
    node->_bounds = _bounds;

    // TODO: Clone the rest of the node data.
}

void Node::_addComponent(UPtr<Component> comp) {
    if (comp.get()) {
        comp->setNode(this);
        _components.push_back(std::move(comp));
    }
}

Refable* Node::getUserObject() const
{
    return _userObject.get();
}

void Node::setUserObject(UPtr<Refable> obj)
{
    _userObject = std::move(obj);
}


Serializable* Node::createObject()
{
    return new Node();
}

std::string Node::getClassName()
{
    return "mgp::Node";
}

void Node::onSerialize(Serializer* serializer)
{
    serializer->writeString("name", _name.c_str(), SCENEOBJECT_NAME);
    serializer->writeBool("enabled", isEnabled(), SCENEOBJECT_ENABLED);
    serializer->writeBool("static", isStatic(), SCENEOBJECT_STATIC);
    serializer->writeVector("position", getTranslation(), SCENEOBJECT_POSITION);
    serializer->writeVector("eulerAngles", getEulerAngles(), SCENEOBJECT_EULER_ANGLES);
    serializer->writeVector("scale", getScale(), SCENEOBJECT_SCALE);
    if (getChildCount() > 0)
    {
        serializer->writeList("children", getChildCount());
        for (Node* child = getFirstChild(); child != NULL; child = child->getNextSibling()) {
            serializer->writeObject(nullptr, child);
        }
        serializer->finishColloction();
    }
    if (_components.size() > 0)
    {
        serializer->writeList("components", _components.size());
        for (auto &component : _components)
        {
            Serializable *s = dynamic_cast<Serializable*>(component.get());
            if (s) {
                serializer->writeObject(nullptr, s);
            }
        }
        serializer->finishColloction();
    }

    /*auto co = getCollisionObject();
    if (co) {
        serializer->writeString("collisionObject", co->getName().c_str(), "");
    }
    else {
        serializer->writeString("collisionObject", "", "");
    }*/
}

void Node::onDeserialize(Serializer* serializer)
{
    serializer->readString("name", _name, SCENEOBJECT_NAME);
    _enabled = serializer->readBool("enabled", SCENEOBJECT_STATIC);
    _static = serializer->readBool("static", SCENEOBJECT_STATIC);
    Vector3 position = serializer->readVector("position", SCENEOBJECT_POSITION);
    this->setTranslation(position);
    Vector3 _eulerAngles = serializer->readVector("eulerAngles", SCENEOBJECT_EULER_ANGLES);
    Quaternion rotation;
    Quaternion::createFromEuler(_eulerAngles.x, _eulerAngles.y, _eulerAngles.z, &rotation);
    this->setRotation(rotation);
    Vector3 scale = serializer->readVector("scale", SCENEOBJECT_SCALE);
    this->setScale(scale);

    size_t childCount = serializer->readList("children");
    if (childCount > 0)
    {
        for (size_t i = 0; i < childCount; i++) {
            auto ptr = serializer->readObject(nullptr);
            //ptr->addRef();
            this->addChild(UPtr<Node>(dynamic_cast<Node*>(ptr)));
        }
    }
    serializer->finishColloction();
    size_t componentCount =  serializer->readList("components");
    if (componentCount > 0)
    {
        //_components.res(componentCount);
        for (size_t i = 0; i < _components.size(); i++)
        {
            auto ptr = serializer->readObject(nullptr);
            //ptr->addRef();
            UPtr<Component> comp(dynamic_cast<Component*>(ptr));
            comp->setNode(this);
            _components.push_back(std::move(comp));
        }
    }
    serializer->finishColloction();

    //std::string collisionObject;
    //serializer->readString("collisionObject", collisionObject, SCENEOBJECT_NAME);
    //PhysicsCollisionObject::load(collisionObject, this);
}


NodeCloneContext::NodeCloneContext()
{
}

NodeCloneContext::~NodeCloneContext()
{
}

Animation* NodeCloneContext::findClonedAnimation(const Animation* animation)
{
    GP_ASSERT(animation);

    std::map<const Animation*, Animation*>::iterator it = _clonedAnimations.find(animation);
    return it != _clonedAnimations.end() ? it->second : NULL;
}

void NodeCloneContext::registerClonedAnimation(const Animation* original, Animation* clone)
{
    GP_ASSERT(original);
    GP_ASSERT(clone);

    _clonedAnimations[original] = clone;
}

Node* NodeCloneContext::findClonedNode(const Node* node)
{
    GP_ASSERT(node);

    std::map<const Node*, Node*>::iterator it = _clonedNodes.find(node);
    return it != _clonedNodes.end() ? it->second : NULL;
}

void NodeCloneContext::registerClonedNode(const Node* original, Node* clone)
{
    GP_ASSERT(original);
    GP_ASSERT(clone);

    _clonedNodes[original] = clone;
}

}
