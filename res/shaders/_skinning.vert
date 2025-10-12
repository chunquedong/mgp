#ifdef MORPH_TARGET_COUNT
    #include "_morph.vert"
#endif

#ifdef INSTANCED
    in mat4 a_instanceMatrix;
#else
    uniform mat4 u_worldViewMatrix;
#endif

uniform vec4 u_matrixPalette[SKINNING_JOINT_COUNT * 3];

in vec4 a_blendWeights;
in vec4 a_blendIndices;

vec4 _skinnedPosition;

void skinPosition(vec3 pos, float blendWeight, int matrixIndex)
{
    vec4 position = vec4(pos, 1.0);
    vec4 tmp;
    tmp.x = dot(position, u_matrixPalette[matrixIndex]);
    tmp.y = dot(position, u_matrixPalette[matrixIndex + 1]);
    tmp.z = dot(position, u_matrixPalette[matrixIndex + 2]);
    tmp.w = position.w;
    _skinnedPosition += blendWeight * tmp;
}

vec4 getPosition()
{
    vec3 pos = a_position;
#ifdef MORPH_TARGET_COUNT
    pos = getMorphPosition(pos);
#endif

    _skinnedPosition = vec4(0.0);
    float blendWeight = a_blendWeights[0];
    int matrixIndex = int (a_blendIndices[0]) * 3;
    skinPosition(pos, blendWeight, matrixIndex);

    blendWeight = a_blendWeights[1];
    matrixIndex = int(a_blendIndices[1]) * 3;
    skinPosition(pos, blendWeight, matrixIndex);

    blendWeight = a_blendWeights[2];
    matrixIndex = int(a_blendIndices[2]) * 3;
    skinPosition(pos, blendWeight, matrixIndex);

    blendWeight = a_blendWeights[3];
    matrixIndex = int(a_blendIndices[3]) * 3;
    skinPosition(pos, blendWeight, matrixIndex);

    vec4 pos4 = _skinnedPosition;
#ifdef INSTANCED
    pos4 = a_instanceMatrix * pos4;
#else
    pos4 = u_worldViewMatrix * pos4;
#endif
    return pos4;
}

#if defined(LIGHTING)


    #if !defined(NORMAL_MAP)
        in vec3 a_normal;
    #endif

    #if defined(BUMPED)
        in vec3 a_tangent;
        //in vec3 a_binormal;
    #endif

    vec3 _skinnedNormal;

    void skinTangentSpaceVector(vec3 vector, float blendWeight, int matrixIndex)
    {
        vec3 tmp;
        tmp.x = dot(vector, u_matrixPalette[matrixIndex].xyz);
        tmp.y = dot(vector, u_matrixPalette[matrixIndex + 1].xyz);
        tmp.z = dot(vector, u_matrixPalette[matrixIndex + 2].xyz);
        _skinnedNormal += blendWeight * tmp;
    }

    vec3 getViewSpaceVector(vec3 vector)
    {
        _skinnedNormal = vec3(0.0);
        // Transform normal to view space using matrix palette with four matrices used to transform a vertex.
        float blendWeight = a_blendWeights[0];
        int matrixIndex = int (a_blendIndices[0]) * 3;
        skinTangentSpaceVector(vector, blendWeight, matrixIndex);
        blendWeight = a_blendWeights[1];
        matrixIndex = int(a_blendIndices[1]) * 3;
        skinTangentSpaceVector(vector, blendWeight, matrixIndex);
        blendWeight = a_blendWeights[2];
        matrixIndex = int(a_blendIndices[2]) * 3;
        skinTangentSpaceVector(vector, blendWeight, matrixIndex);
        blendWeight = a_blendWeights[3];
        matrixIndex = int(a_blendIndices[3]) * 3;
        skinTangentSpaceVector(vector, blendWeight, matrixIndex);
        return _skinnedNormal;
    }

    vec3 getNormal()
    {
        return getViewSpaceVector(a_normal);
    }

    #if defined(BUMPED)

        vec3 getTangent()
        {
            return getViewSpaceVector(a_tangent);
        }

        vec3 getBinormal()
        {
            vec3 binormal = cross(a_tangent, a_normal);
            return getViewSpaceVector(binormal);
        }

    #endif

#endif

