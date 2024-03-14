#ifndef MESHSKIN_H_
#define MESHSKIN_H_

#include "math/Matrix.h"
#include "scene/Transform.h"
//#include "scene/Component.h"
#include "scene/Node.h"
#include "base/Resource.h"

namespace mgp
{
class Node;

class BoneJoint {
public:
    std::string _name;
    SPtr<Node> _node;
    //inverseBindMatrices
    Matrix _bindPose;

    void write(Stream* file);
    bool read(Stream* file);
};

/**
 * Defines the skin for a mesh.
 *
 * A skin is used to support skinning otherwise known as 
 * vertex blending. This allows for a Model's mesh to support
 * a skeleton on joints that will influence the vertex position
 * and which the joints can be animated.
 */
class MeshSkin : public Resource
{
public:
    /**
     * Returns the number of joints in this MeshSkin.
     */
    unsigned int getJointCount() const;

    /**
     * Returns the joint at the given index.
     * 
     * @param index The index.
     * 
     * @return The joint.
     */
    BoneJoint* getJoint(unsigned int index);

    /**
     * Returns the root most joint for this MeshSkin.
     *
     * @return The root joint.
     */
    Node* getRootJoint() const;

    /**
     * Sets the root joint for this MeshSkin.
     *
     * The specified Joint must belong to the joint list for this MeshSkin.
     *
     * @param joint The root joint.
     */
    void setRootJoint(Node* joint);

    /**
     * Returns the pointer to the Vector4 array for the purpose of binding to a shader.
     * 
     * @return The pointer to the matrix palette.
     */
    Vector4* getMatrixPalette(const Matrix* viewMatrix, Node* node);

    /**
     * Returns the number of elements in the matrix palette array.
     * Each element is a Vector4* that represents a row.
     * Each matrix palette is represented by 3 rows of Vector4.
     * 
     * @return The matrix palette size.
     */
    unsigned int getMatrixPaletteSize() const;


    void write(Stream* file);
    bool read(Stream* file);


//private:

    /**
     * Constructor.
     */
    MeshSkin();

    /**
     * Constructor.
     */
    MeshSkin(const MeshSkin&) = delete;

    /**
     * Destructor.
     */
    ~MeshSkin();
    
    /**
     * Hidden copy assignment operator.
     */
    MeshSkin& operator=(const MeshSkin&) = delete;

    /**
     * Clones the MeshSkin and the joints that it references.
     * 
     * @param context The clone context.
     * 
     * @return The newly created MeshSkin.
     */
    UPtr<MeshSkin> clone(NodeCloneContext &context) const;

    /**
     * Sets the number of joints that can be stored in this skin.
     * This method allocates the necessary memory.
     * 
     * @param jointCount The new size of the joint vector.
     */
    void setJointCount(unsigned int jointCount);

    void resetBind();
private:
    void bindNode(Node* node);

    void rebindJoins();
private:
    std::string _rootJointName;

    std::vector<BoneJoint> _joints;

    //for calculate node boundBox
    SPtr<Node> _rootJoint;

    // Pointer to the array of palette matrices.
    // This array is passed to the vertex shader as a uniform.
    // Each 4x3 row-wise matrix is represented as 3 Vector4's.
    // The number of Vector4's is (_joints.size() * 3).
    Vector4* _matrixPalette;
};

}

#endif
