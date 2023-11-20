#include "base/Base.h"
#include "BoneJoint.h"
#include "MeshSkin.h"
#include "Model.h"

namespace mgp
{

BoneJoint::BoneJoint(const char* id)
    : Node(id), _jointMatrixDirty(true)
{
}

BoneJoint::~BoneJoint()
{
}

UPtr<BoneJoint> BoneJoint::create(const char* id)
{
    return UPtr<BoneJoint>(new BoneJoint(id));
}

UPtr<Node> BoneJoint::cloneSingleNode(NodeCloneContext &context) const
{
    UPtr<BoneJoint> copy = BoneJoint::create(getName());
    GP_ASSERT(copy.get());
    context.registerClonedNode(this, copy.get());
    copy->_bindPose = _bindPose;
    Node::cloneInto(copy.get(), context);
    return copy.dynamicCastTo<Node>();
}

Node::Type BoneJoint::getType() const
{
    return Node::JOINT;
}

const char* BoneJoint::getTypeName() const
{
    return "Joint";
}

Scene* BoneJoint::getScene() const
{
    // Overrides Node::getScene() to search the node our skins.
    for (const SkinReference* itr = &_skin; itr && itr->skin; itr = itr->next)
    {
        Model* model = itr->skin ? itr->skin->getModel() : NULL;
        if (model)
        {
            Node* node = model->getNode();
            if (node)
            {
                Scene* scene = node->getScene();
                if (scene)
                    return scene;
            }
        }
    }

    return Node::getScene();
}

void BoneJoint::transformChanged()
{
    Node::transformChanged();
    _jointMatrixDirty = true;
}

void BoneJoint::updateJointMatrix(const Matrix& bindShape, Vector4* matrixPalette)
{
    // Note: If more than one MeshSkin influences this Joint, we need to skip
    // the _jointMatrixDirty optimization since updateJointMatrix() may be
    // called multiple times a frame with different bindShape matrices (and
    // different matrixPallete pointers).
    if (_skin.next || _jointMatrixDirty)
    {
        _jointMatrixDirty = false;

        static Matrix t;
        Matrix::multiply(Node::getWorldMatrix(), getInverseBindPose(), &t);
        Matrix::multiply(t, bindShape, &t);

        GP_ASSERT(matrixPalette);
        matrixPalette[0].set(t.m[0], t.m[4], t.m[8], t.m[12]);
        matrixPalette[1].set(t.m[1], t.m[5], t.m[9], t.m[13]);
        matrixPalette[2].set(t.m[2], t.m[6], t.m[10], t.m[14]);
    }
}

const Matrix& BoneJoint::getInverseBindPose() const
{
    return _bindPose;
}

void BoneJoint::setInverseBindPose(const Matrix& m)
{
    _bindPose = m;
    _jointMatrixDirty = true;
}

void BoneJoint::addSkin(MeshSkin* skin)
{
    if (!_skin.skin)
    {
        // Store skin in root reference
        _skin.skin = skin;
    }
    else
    {
        // Add a new SkinReference to the end of our list
        SkinReference* ref = &_skin;
        while (ref->next)
        {
            ref = ref->next;
        }
        ref->next = new SkinReference();
        ref->next->skin = skin;
    }
}

void BoneJoint::removeSkin(MeshSkin* skin)
{
    if (_skin.skin == skin)
    {
        // Skin is our root referenced skin
        _skin.skin = NULL;

        // Shift the next skin reference down to the root
        if (_skin.next)
        {
            SkinReference* tmp = _skin.next;
            _skin.skin = tmp->skin;
            _skin.next = tmp->next;
            tmp->next = NULL; // prevent deletion
            SAFE_DELETE(tmp);
        }
    }
    else
    {
        // Search for the entry referencing this skin
        SkinReference* ref = &_skin;
        while (SkinReference* tmp = ref->next)
        {
            if (tmp->skin == skin)
            {
                // Link this refernce out
                ref->next = tmp->next;
                tmp->next = NULL; // prevent deletion
                SAFE_DELETE(tmp);
                break;
            }
            ref = tmp;
        }
    }
}

void BoneJoint::write(Stream* file) {
    file->writeStr(this->getName());
    file->write((char*)&_matrix.m, sizeof(_bindPose.m));
    file->write((char*)&_bindPose.m, sizeof(_bindPose.m));
    file->writeUInt16(this->getChildCount());

    for (Node* child = getFirstChild(); child != NULL; child = child->getNextSibling()) {
        BoneJoint *join = dynamic_cast<BoneJoint*>(child);
        if (join) {
            join->write(file);
        }
    }
}
BoneJoint* BoneJoint::read(Stream* file) {
    std::string id = file->readStr();
    BoneJoint *join = new BoneJoint(id.c_str());
    file->read(&join->_matrix, sizeof(Matrix), 1);
    file->read(&join->_bindPose, sizeof(Matrix), 1);

    int size = file->readUInt16();
    for (int i=0; i<size; ++i) {
        BoneJoint *child = BoneJoint::read(file);
        join->addChild(UPtr<Node>(child));
    }

    return join;
}

BoneJoint::SkinReference::SkinReference()
    : skin(NULL), next(NULL)
{
}

BoneJoint::SkinReference::~SkinReference()
{
    SAFE_DELETE(next);
}

}
