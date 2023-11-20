#ifndef MESH_BUILDER_H_
#define MESH_BUILDER_H_

#include "Mesh.h"

namespace mgp
{
class MeshFactory {
public:
    static UPtr<Mesh> createPlane();

    /**
     * Creates a new textured 3D quad.
     *
     * The specified points should describe a triangle strip with the first 3 points
     * forming a triangle wound in counter-clockwise direction, with the second triangle
     * formed from the last three points in clockwise direction.
     *
     * @param p1 The first point.
     * @param p2 The second point.
     * @param p3 The third point.
     * @param p4 The fourth point.
     * 
     * @return The created mesh.
     * @script{create}
     */
    static UPtr<Mesh> createQuad3D(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4);

    /**
     * Constructs a new textured 2D quad.
     * 
     * @param x The x coordinate.
     * @param y The y coordinate.
     * @param width The width of the quad.
     * @param height The height of the quad.
     * @param s1 The S texture coordinate of the bottom left point.
     * @param t1 The T texture coordinate of the bottom left point.
     * @param s2 The S texture coordinate of the top right point.
     * @param t2 The T texture coordinate of the top right point.
     * 
     * @return The newly created mesh.
     * @script{create}
     */
    static UPtr<Mesh> createQuad(float x, float y, float width, float height, float s1 = 0.0f, float t1 = 0.0f, float s2 = 1.0f, float t2 = 1.0f);

    /**
     * Creates a new full-screen 2D quad.
     *
     * The returned mesh's vertex format includes a 2-element (x,y) position
     * and a 2-element texture coordinate.
     *
     * This method returns a mesh describing a fullscreen quad using
     * normalized device coordinates for vertex positions.
     * 
     * @return The newly created mesh.
     * @script{create}
     */
    static UPtr<Mesh> createQuadFullscreen();

    /**
     * Creates lines between 2 or more points passed in as a Vector3 array.
     *
     * The mesh contains only position data using lines to connect the vertices.
     * This is useful for drawing basic color elements into a scene.
     * 
     * @param points The array of points.
     * @param pointCount The number of points.
     * 
     * @return The newly created mesh.
     * @script{create}
     */
    static UPtr<Mesh> createLines(Vector3* points, unsigned int pointCount);

    static UPtr<Mesh> createCube(float size = 1.0f);
    static UPtr<Mesh> createSimpleCube();
    static UPtr<Mesh> createSpherical(int subdivision = 64);

    /**
     * Creates a bounding box mesh when passed a BoundingBox.
     *
     * The mesh contains only position data using lines to connect the vertices.
     *
     * @param box The BoundingBox that will be used to create the mesh.
     * 
     * @return The newly created bounding box mesh.
     * @script{create}
     */
    static UPtr<Mesh> createBoundingBox(const BoundingBox& box);
};

}

#endif