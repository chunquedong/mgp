#include "MeshFactory.h"

using namespace mgp;

UPtr<Mesh> MeshFactory::createQuad(float x, float y, float width, float height, float s1, float t1, float s2, float t2)
{
    float x2 = x + width;
    float y2 = y + height;

    float vertices[] =
    {
        x, y2, 0,   0, 0, 1,    s1, t2,
        x, y, 0,    0, 0, 1,    s1, t1,
        x2, y2, 0,  0, 0, 1,    s2, t2,
        x2, y, 0,   0, 0, 1,    s2, t1,
    };

    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::NORMAL, 3),
        VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
    };
    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 3), 4);
    if (mesh.get() == NULL)
    {
        GP_ERROR("Failed to create mesh.");
        return UPtr<Mesh>(NULL);
    }

    mesh->_primitiveType = Mesh::TRIANGLE_STRIP;
    mesh->getVertexBuffer()->setData((char*)vertices, sizeof(vertices));

    return mesh;
}

UPtr<Mesh> MeshFactory::createQuadFullscreen()
{
    float x = -1.0f;
    float y = -1.0f;
    float x2 = 1.0f;
    float y2 = 1.0f;

    float vertices[] =
    {
        x, y2,   0, 1,
        x, y,    0, 0,
        x2, y2,  1, 1,
        x2, y,   1, 0
    };

    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 2),
        VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
    };
    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 2), 4);
    if (mesh.get() == NULL)
    {
        GP_ERROR("Failed to create mesh.");
        return UPtr<Mesh>(NULL);
    }
    mesh->setId("QuadFullscreen");
    mesh->_primitiveType = Mesh::TRIANGLE_STRIP;
    mesh->getVertexBuffer()->setData((char*)vertices, sizeof(vertices));

    return mesh;
}

UPtr<Mesh> MeshFactory::createPlane() {
    float vertices[] = {
        -1, 0, -1, 0, 1, 0,
        -1, 0, 1, 0, 1, 0,
        1, 0, -1, 0, 1, 0,
        1, 0, 1, 0, 1, 0,
    };

    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::NORMAL, 3),
    };

    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 2), 4);
    if (mesh.get() == NULL)
    {
        GP_ERROR("Failed to create mesh.");
        return UPtr<Mesh>(NULL);
    }
    mesh->setId("Plane");
    mesh->_primitiveType = Mesh::TRIANGLE_STRIP;
    mesh->getVertexBuffer()->setData((char*)vertices, sizeof(vertices));

    return mesh;
}

UPtr<Mesh> MeshFactory::createQuad3D(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4)
{
    // Calculate the normal vector of the plane.
    Vector3 v1, v2, n;
    Vector3::subtract(p2, p1, &v1);
    Vector3::subtract(p3, p2, &v2);
    Vector3::cross(v1, v2, &n);
    n.normalize();

    float vertices[] =
    {
        (float)p1.x, (float)p1.y, (float)p1.z, (float)n.x, (float)n.y, (float)n.z, 0, 1,
        (float)p2.x, (float)p2.y, (float)p2.z, (float)n.x, (float)n.y, (float)n.z, 0, 0,
        (float)p3.x, (float)p3.y, (float)p3.z, (float)n.x, (float)n.y, (float)n.z, 1, 1,
        (float)p4.x, (float)p4.y, (float)p4.z, (float)n.x, (float)n.y, (float)n.z, 1, 0
    };

    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::NORMAL, 3),
        VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
    };

    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 3), 4);
    if (mesh.get() == NULL)
    {
        GP_ERROR("Failed to create mesh.");
        return UPtr<Mesh>(NULL);
    }

    mesh->_primitiveType = Mesh::TRIANGLE_STRIP;
    mesh->getVertexBuffer()->setData((char*)vertices, sizeof(vertices));

    return mesh;
}

UPtr<Mesh> MeshFactory::createLines(Vector3* points, unsigned int pointCount)
{
    GP_ASSERT(points);
    GP_ASSERT(pointCount);

    float* vertices = new float[pointCount*3];
    memcpy(vertices, points, pointCount*3*sizeof(float));

    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3)
    };
    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 1), pointCount);
    if (mesh.get() == NULL)
    {
        GP_ERROR("Failed to create mesh.");
        SAFE_DELETE_ARRAY(vertices);
        return UPtr<Mesh>(NULL);
    }

    mesh->_primitiveType = Mesh::LINE_STRIP;
    mesh->getVertexBuffer()->setData((char*)vertices, sizeof(vertices));

    SAFE_DELETE_ARRAY(vertices);
    return mesh;
}

UPtr<Mesh> MeshFactory::createCube(float size)
{
    float a = size * 0.5f;
    float vertices[] =
    {
        -a, -a,  a,    0.0,  0.0,  1.0,   0.0, 0.0,
         a, -a,  a,    0.0,  0.0,  1.0,   1.0, 0.0,
        -a,  a,  a,    0.0,  0.0,  1.0,   0.0, 1.0,
         a,  a,  a,    0.0,  0.0,  1.0,   1.0, 1.0,
        -a,  a,  a,    0.0,  1.0,  0.0,   0.0, 0.0,
         a,  a,  a,    0.0,  1.0,  0.0,   1.0, 0.0,
        -a,  a, -a,    0.0,  1.0,  0.0,   0.0, 1.0,
         a,  a, -a,    0.0,  1.0,  0.0,   1.0, 1.0,
        -a,  a, -a,    0.0,  0.0, -1.0,   0.0, 0.0,
         a,  a, -a,    0.0,  0.0, -1.0,   1.0, 0.0,
        -a, -a, -a,    0.0,  0.0, -1.0,   0.0, 1.0,
         a, -a, -a,    0.0,  0.0, -1.0,   1.0, 1.0,
        -a, -a, -a,    0.0, -1.0,  0.0,   0.0, 0.0,
         a, -a, -a,    0.0, -1.0,  0.0,   1.0, 0.0,
        -a, -a,  a,    0.0, -1.0,  0.0,   0.0, 1.0,
         a, -a,  a,    0.0, -1.0,  0.0,   1.0, 1.0,
         a, -a,  a,    1.0,  0.0,  0.0,   0.0, 0.0,
         a, -a, -a,    1.0,  0.0,  0.0,   1.0, 0.0,
         a,  a,  a,    1.0,  0.0,  0.0,   0.0, 1.0,
         a,  a, -a,    1.0,  0.0,  0.0,   1.0, 1.0,
        -a, -a, -a,   -1.0,  0.0,  0.0,   0.0, 0.0,
        -a, -a,  a,   -1.0,  0.0,  0.0,   1.0, 0.0,
        -a,  a, -a,   -1.0,  0.0,  0.0,   0.0, 1.0,
        -a,  a,  a,   -1.0,  0.0,  0.0,   1.0, 1.0
    };
    short indices[] =
    {
        0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7, 8, 9, 10, 10, 9, 11, 12, 13, 14, 14, 13, 15, 16, 17, 18, 18, 17, 19, 20, 21, 22, 22, 21, 23
    };
    unsigned int vertexCount = 24;
    unsigned int indexCount = 36;
    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::NORMAL, 3),
        VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
    };
    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 3), vertexCount);
    if (mesh.get() == NULL)
    {
        GP_ERROR("Failed to create mesh.");
        return UPtr<Mesh>(NULL);
    }
    mesh->setId("Cube");
    mesh->getVertexBuffer()->setData((char*)vertices, sizeof(vertices));
    mesh->setIndex(Mesh::TRIANGLES, indexCount);
    mesh->getIndexBuffer()->setData((char*)indices, sizeof(indices));
    return mesh;
}

UPtr<Mesh> MeshFactory::createCube2(float size)
{
    float a = size * 0.5f;
    float vertices[] =
    {
        -a, -a,  a,
         a, -a,  a,
        -a,  a,  a,
         a,  a,  a,
        -a,  a,  a,
         a,  a,  a,
        -a,  a, -a,
         a,  a, -a,
        -a,  a, -a,
         a,  a, -a,
        -a, -a, -a,
         a, -a, -a,
        -a, -a, -a,
         a, -a, -a,
        -a, -a,  a,
         a, -a,  a,
         a, -a,  a,
         a, -a, -a,
         a,  a,  a,
         a,  a, -a,
        -a, -a, -a,
        -a, -a,  a,
        -a,  a, -a,
        -a,  a,  a,
    };
    short indices[] =
    {
        0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7, 8, 9, 10, 10, 9, 11, 12, 13, 14, 14, 13, 15, 16, 17, 18, 18, 17, 19, 20, 21, 22, 22, 21, 23
    };
    unsigned int vertexCount = 24;
    unsigned int indexCount = 36;
    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
    };
    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 1), vertexCount);
    if (mesh.get() == NULL)
    {
        GP_ERROR("Failed to create mesh.");
        return UPtr<Mesh>(NULL);
    }
    mesh->setId("Cube2");
    mesh->getVertexBuffer()->setData((char*)vertices, sizeof(vertices));
    mesh->setIndex(Mesh::TRIANGLES, indexCount);
    mesh->getIndexBuffer()->setData((char*)indices, sizeof(indices));
    return mesh;
}

UPtr<Mesh> MeshFactory::createTorus(int radial_resolution, int tubular_resolution, float radius, float thickness)
{
    std::vector<float> vertices;
    std::vector<uint16_t> indices;

    // generate vertices
    for (size_t i = 0; i < radial_resolution; i++) {
        for (size_t j = 0; j < tubular_resolution; j++) {
            float u = (float)j / tubular_resolution * MATH_PI * 2.0;
            float v = (float)i / radial_resolution * MATH_PI * 2.0;
            float x = (radius + thickness * std::cos(v)) * std::cos(u);
            float y = (radius + thickness * std::cos(v)) * std::sin(u);
            float z = thickness * std::sin(v);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    // add quad faces
    for (int i = 0; i < radial_resolution; i++) {
        int i_next = (i + 1) % radial_resolution;
        for (int j = 0; j < tubular_resolution; j++) {
            int j_next = (j + 1) % tubular_resolution;
            int i0 = i * tubular_resolution + j;
            int i1 = i * tubular_resolution + j_next;
            int i2 = i_next * tubular_resolution + j_next;
            int i3 = i_next * tubular_resolution + j;
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    unsigned int vertexCount = vertices.size() / 3;
    unsigned int indexCount = indices.size();
    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        //VertexFormat::Element(VertexFormat::NORMAL, 3),
        //VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
    };
    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 1), vertexCount);
    if (mesh.get() == NULL)
    {
        GP_ERROR("Failed to create mesh.");
        return UPtr<Mesh>(NULL);
    }
    mesh->getVertexBuffer()->setData((char*)vertices.data(), vertexCount * 3 * sizeof(float));
    mesh->setIndex(Mesh::TRIANGLES, indexCount);
    mesh->getIndexBuffer()->setData((char*)indices.data(), indices.size() * sizeof(uint16_t));

    return mesh;
}

UPtr<Mesh> MeshFactory::createSimpleCube()
{
    float vertices[] =
    {
        -1.0, 1.0, -1.0, -1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0,
      1.0, -1.0, -1.0, 1.0, -1.0,

      -1.0, -1.0, 1.0, -1.0, -1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, -1.0, -1.0,
      1.0, 1.0, -1.0, -1.0, 1.0,

      1.0, -1.0, -1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
      -1.0, 1.0, -1.0, -1.0,

      -1.0, -1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, -1.0,
      1.0, -1.0, -1.0, 1.0,

      -1.0, 1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 1.0,
      1.0, -1.0, 1.0, -1.0,

      -1.0, -1.0, -1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0, -1.0, -1.0,
      -1.0, 1.0, 1.0, -1.0, 1.0,
    };

    unsigned int vertexCount = 36;
    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
    };
    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 1), vertexCount);
    if (mesh.get() == NULL)
    {
        GP_ERROR("Failed to create mesh.");
        return UPtr<Mesh>(NULL);
    }
    mesh->setId("SimpleCube");
    mesh->getVertexBuffer()->setData((char*)vertices, sizeof(vertices));
    mesh->_primitiveType = Mesh::TRIANGLES;
    return mesh;
}

UPtr<Mesh> MeshFactory::createSpherical(int subdivision) {
    int width_segments = subdivision;
    int height_segments = subdivision;

    float phi_start = 0, phi_length = MATH_PI * 2, theta_start = 0, theta_length = MATH_PI;
    float theta_end = std::min(theta_start + theta_length, (float)MATH_PI);
    float radius = 1.0;

    float *_vertices = new float[((width_segments + 1) * (height_segments + 1) * 6)];
    int i = 0;
    std::vector<uint16_t> indices;

    int index = 0;
    std::vector<std::vector<int> > grid;

    // generate vertices, normals and uvs
    for (int iy = 0; iy <= height_segments; iy++) {
        std::vector<int> vertices_row;
        float v = iy / (double)height_segments;
        // special case for the poles
        int uOffset = 0;
        if (iy == 0 && theta_start == 0) {
            uOffset = 0.5 / width_segments;
        }
        else if (iy == height_segments && theta_end == MATH_PI) {
            uOffset = -0.5 / width_segments;
        }

        for (int ix = 0; ix <= width_segments; ix++) {
            float u = ix / (double)width_segments;
            // vertex
            float x = -radius * cos(phi_start + u * phi_length) * sin(theta_start + v * theta_length);
            float y = radius * cos(theta_start + v * theta_length);
            float z = radius * sin(phi_start + u * phi_length) * sin(theta_start + v * theta_length);

            _vertices[i++] = x;
            _vertices[i++] = y;
            _vertices[i++] = z;

            // normal
            _vertices[i++] = x;
            _vertices[i++] = y;
            _vertices[i++] = z;

            // uv
            //uvs.push( u + uOffset, 1 - v );
            vertices_row.push_back(index++);
        }

        grid.push_back(vertices_row);
    }

    // indices
    for (int iy = 0; iy < height_segments; iy++) {
        for (int ix = 0; ix < width_segments; ix++) {
            int a = grid[iy][ix + 1];
            int b = grid[iy][ix];
            int c = grid[iy + 1][ix];
            int d = grid[iy + 1][ix + 1];
            if (iy != 0 || theta_start > 0) {
                indices.push_back(a);
                indices.push_back(b);
                indices.push_back(d);
            }
            if (iy != height_segments - 1 || theta_end < MATH_PI) {
                indices.push_back(b);
                indices.push_back(c);
                indices.push_back(d);
            }
        }
    }

    unsigned int vertexCount = (width_segments + 1) * (height_segments + 1);
    unsigned int indexCount = indices.size();
    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::NORMAL, 3),
        //VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
    };
    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 2), vertexCount);
    if (mesh.get() == NULL)
    {
        GP_ERROR("Failed to create mesh.");
        return UPtr<Mesh>(NULL);
    }
    mesh->getVertexBuffer()->setData((char*)_vertices, vertexCount*6*sizeof(float));
    mesh->setIndex(Mesh::TRIANGLES, indexCount);
    mesh->getIndexBuffer()->setData((char*)indices.data(), indices.size()*sizeof(uint16_t));

    SAFE_DELETE_ARRAY(_vertices);
    return mesh;
}

UPtr<Mesh> MeshFactory::createCone(float radius, float height) {
    int subdivision = 10;
    std::vector<float> vertices;
    std::vector<uint16_t> indices;

    vertices.reserve(subdivision * 3 + 9);
    indices.reserve(subdivision * 6);

    vertices.push_back(0);
    vertices.push_back(0);
    vertices.push_back(0);

    vertices.push_back(0);
    vertices.push_back(0);
    vertices.push_back(height);

    float delta = MATH_PI * 2 / (double)subdivision;
    float theta = 0;
    int i = 0;
    for (; i < subdivision; ++i) {
        theta += delta;
        // vertex
        float x = radius * cos(theta);
        float y = radius * sin(theta);
        float z = 0;

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);

        if (theta > 0) {
            int index = i + 2;
            indices.push_back(index-1);
            indices.push_back(0);
            indices.push_back(index);

            indices.push_back(index);
            indices.push_back(1);
            indices.push_back(index-1);
        }
    }

    indices.push_back(i + 2 - 1);
    indices.push_back(0);
    indices.push_back(2);
    
    indices.push_back(2);
    indices.push_back(1);
    indices.push_back(i + 2 - 1);
    

    unsigned int vertexCount = vertices.size()/3;
    unsigned int indexCount = indices.size();
    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        //VertexFormat::Element(VertexFormat::NORMAL, 3),
        //VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
    };
    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 1), vertexCount);
    if (mesh.get() == NULL)
    {
        GP_ERROR("Failed to create mesh.");
        return UPtr<Mesh>(NULL);
    }
    mesh->getVertexBuffer()->setData((char*)vertices.data(), vertexCount * 3 * sizeof(float));
    mesh->setIndex(Mesh::TRIANGLES, indexCount);
    mesh->getIndexBuffer()->setData((char*)indices.data(), indices.size() * sizeof(uint16_t));

    return mesh;
}

UPtr<Mesh> MeshFactory::createCylinder(float radius, float height) {
    int subdivision = 10;
    std::vector<float> vertices;
    std::vector<uint16_t> indices;

    vertices.reserve(subdivision * 3 * 2 + 9);
    indices.reserve(subdivision * 3 * 4 + 9);

    vertices.push_back(0);
    vertices.push_back(0);
    vertices.push_back(0);

    vertices.push_back(0);
    vertices.push_back(0);
    vertices.push_back(-height);

    float delta = MATH_PI * 2 / (double)subdivision;
    float theta = 0;
    int i = 0;
    for (; i <= subdivision; ++i) {
        theta += delta;

        // vertex
        float x = radius * cos(theta);
        float y = radius * sin(theta);

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(0);

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(-height);

        if (theta > 0) {
            int index1 = 2 + ((i-1)*2);
            int index2 = 2 + (i * 2);
            indices.push_back(index2);
            indices.push_back(0);
            indices.push_back(index1);

            int index3 = 2 + ((i - 1) * 2) + 1;
            int index4 = 2 + (i * 2) + 1;
            indices.push_back(index3);
            indices.push_back(1);
            indices.push_back(index4);


            indices.push_back(index2);
            indices.push_back(index1);
            indices.push_back(index4);

            indices.push_back(index1);
            indices.push_back(index3);
            indices.push_back(index4);
        }
    }


    unsigned int vertexCount = vertices.size() / 3;
    unsigned int indexCount = indices.size();
    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        //VertexFormat::Element(VertexFormat::NORMAL, 3),
        //VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
    };
    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 1), vertexCount);
    if (mesh.get() == NULL)
    {
        GP_ERROR("Failed to create mesh.");
        return UPtr<Mesh>(NULL);
    }
    mesh->getVertexBuffer()->setData((char*)vertices.data(), vertexCount * 3 * sizeof(float));
    mesh->setIndex(Mesh::TRIANGLES, indexCount);
    mesh->getIndexBuffer()->setData((char*)indices.data(), indices.size() * sizeof(uint16_t));

    return mesh;
}

UPtr<Mesh> MeshFactory::createBoundingBox(const BoundingBox& box)
{
    Vector3 corners[8];
    box.getCorners(corners);

    float vertices[] =
    {
        (float)corners[7].x, (float)corners[7].y, (float)corners[7].z,
        (float)corners[6].x, (float)corners[6].y, (float)corners[6].z,
        (float)corners[1].x, (float)corners[1].y, (float)corners[1].z,
        (float)corners[0].x, (float)corners[0].y, (float)corners[0].z,
        (float)corners[7].x, (float)corners[7].y, (float)corners[7].z,
        (float)corners[4].x, (float)corners[4].y, (float)corners[4].z,
        (float)corners[3].x, (float)corners[3].y, (float)corners[3].z,
        (float)corners[0].x, (float)corners[0].y, (float)corners[0].z,
        (float)corners[0].x, (float)corners[0].y, (float)corners[0].z,
        (float)corners[1].x, (float)corners[1].y, (float)corners[1].z,
        (float)corners[2].x, (float)corners[2].y, (float)corners[2].z,
        (float)corners[3].x, (float)corners[3].y, (float)corners[3].z,
        (float)corners[4].x, (float)corners[4].y, (float)corners[4].z,
        (float)corners[5].x, (float)corners[5].y, (float)corners[5].z,
        (float)corners[2].x, (float)corners[2].y, (float)corners[2].z,
        (float)corners[1].x, (float)corners[1].y, (float)corners[1].z,
        (float)corners[6].x, (float)corners[6].y, (float)corners[6].z,
        (float)corners[5].x, (float)corners[5].y, (float)corners[5].z
    };

    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3)
    };
    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 1), 18);
    if (mesh.get() == NULL)
    {
        GP_ERROR("Failed to create mesh.");
        return UPtr<Mesh>(NULL);
    }

    mesh->_primitiveType = Mesh::LINE_STRIP;
    mesh->getVertexBuffer()->setData((char*)vertices, sizeof(vertices));

    return mesh;
}