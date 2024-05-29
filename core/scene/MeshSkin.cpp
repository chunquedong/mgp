#include "MeshSkin.h"
#include "base/Base.h"
#include "Node.h"

// The number of rows in each palette matrix.
#define PALETTE_ROWS 3

namespace mgp
{

void BoneJoint::write(Stream* file) {
    file->writeStr(this->_name);
    file->write((char*)&_bindPose.m, sizeof(_bindPose.m));
}
bool BoneJoint::read(Stream* file) {
    _name = file->readStr();
    file->read(&_bindPose, sizeof(Matrix), 1);
    return true;
}

MeshSkin::MeshSkin()
    : _rootJoint(0), _matrixPalette(NULL)
{
}

MeshSkin::~MeshSkin()
{
    SAFE_DELETE_ARRAY(_matrixPalette);
}

unsigned int MeshSkin::getJointCount() const
{
    return (unsigned int)_joints.size();
}

BoneJoint* MeshSkin::getJoint(unsigned int index)
{
    GP_ASSERT(index < _joints.size());
    return &(_joints[index]);
}

UPtr<MeshSkin> MeshSkin::clone(NodeCloneContext &context) const
{
    MeshSkin* skin = new MeshSkin();
    const unsigned int jointCount = getJointCount();
    skin->setJointCount(jointCount);
    
    //skin->_rootJoint = _rootJoint;
    skin->_rootJointName = _rootJointName;
    for (unsigned int i = 0; i < jointCount; ++i)
    {  
        BoneJoint* newJoint = skin->getJoint(i);
        //*newJoint = _joints[i];
        newJoint->_bindPose = _joints[i]._bindPose;
        newJoint->_name = _joints[i]._name;
    }
    
    return UPtr<MeshSkin>(skin);
}

void MeshSkin::setJointCount(unsigned int jointCount)
{
    // Resize the joints vector and initialize to NULL.
    _joints.resize(jointCount);
    for (unsigned int i = 0; i < jointCount; i++)
    {
        _joints[i]._node = NULL;
    }

    // Rebuild the matrix palette. Each matrix is 3 rows of Vector4.
    SAFE_DELETE_ARRAY(_matrixPalette);

    if (jointCount > 0)
    {
        _matrixPalette = new Vector4[jointCount * PALETTE_ROWS];
        for (unsigned int i = 0; i < jointCount * PALETTE_ROWS; i+=PALETTE_ROWS)
        {
            _matrixPalette[i+0].set(1.0f, 0.0f, 0.0f, 0.0f);
            _matrixPalette[i+1].set(0.0f, 1.0f, 0.0f, 0.0f);
            _matrixPalette[i+2].set(0.0f, 0.0f, 1.0f, 0.0f);
        }
    }
}

Vector4* MeshSkin::getMatrixPalette(const Matrix* viewMatrix, Node* node)
{
    if (node) {
        if (_rootJoint.get() == NULL && _rootJointName.size() > 0) {
            Node* parent = node;
            while (parent->getParent()) {
                parent = parent->getParent();
            }
            bindNode(parent);
        }
    }
    GP_ASSERT(_matrixPalette);

    for (size_t i = 0, count = _joints.size(); i < count; i++)
    {
        BoneJoint* joint = (BoneJoint*) & (_joints[i]);

        Matrix t;
        Matrix::multiply(joint->_node->getWorldMatrix(), joint->_bindPose, &t);

        Matrix::multiply(*viewMatrix, t, &t);
        
        //Matrix::multiply(t, bindShape, &t);

        Vector4* matrixPalette = &_matrixPalette[i * PALETTE_ROWS];
        matrixPalette[0].set(t.m[0], t.m[4], t.m[8], t.m[12]);
        matrixPalette[1].set(t.m[1], t.m[5], t.m[9], t.m[13]);
        matrixPalette[2].set(t.m[2], t.m[6], t.m[10], t.m[14]);
    }
    return _matrixPalette;
}

unsigned int MeshSkin::getMatrixPaletteSize() const
{
    return (unsigned int)_joints.size() * PALETTE_ROWS;
}

Node* MeshSkin::getRootJoint() const
{
    return _rootJoint.get();
}

void MeshSkin::setRootJoint(Node* joint)
{
    _rootJoint = joint;
    _rootJointName = _rootJoint->getName();
}

void MeshSkin::bindByRootJoint() {
    Node* rootNode = _rootJoint.get();
    GP_ASSERT(rootNode);

    for (int i = 0; i < _joints.size(); ++i) {
        Node* node = rootNode->findNode(_joints[i]._name.c_str());
        GP_ASSERT(node);
        _joints[i]._node = node;
    }
}

void MeshSkin::bindNode(Node* parent) {
    Node* root = parent->findNode(_rootJointName.c_str());
    if (root) {
        setRootJoint(root);
        bindByRootJoint();
    }
}

void MeshSkin::clearBind() {
    _rootJoint.clear();
    for (int i = 0; i < _joints.size(); ++i) {
        _joints[i]._node.clear();
    }
}

void MeshSkin::write(Stream* file) {
    if (_rootJoint.get()) {
        file->writeStr(_rootJoint->getName());
    }
    else {
        file->writeStr("");
    }
    //file->write((char*)&_bindShape.m, sizeof(_bindShape.m));
    file->writeUInt16(_joints.size());
    for (BoneJoint& join : _joints) {
        join.write(file);
    }
}
bool MeshSkin::read(Stream* file) {
    MeshSkin* skin = this;
    _rootJointName = file->readStr();

    //file->read((char*)&skin->_bindShape.m, sizeof(skin->_bindShape.m));
    int size = file->readUInt16();
    skin->setJointCount(size);
    for (int i = 0; i < size; ++i) {
        _joints[i].read(file);
    }
    //rebindJoins();
    return true;
}

}
