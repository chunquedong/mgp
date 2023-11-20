uniform vec4 u_matrixPalette[SKINNING_JOINT_COUNT * 3];

in vec4 a_blendWeights;
in vec4 a_blendIndices;

vec4 _skinnedPosition;

void skinPosition(float blendWeight, int matrixIndex)
{
    vec4 position = vec4(a_position, 1.0);
    vec4 tmp;
    tmp.x = dot(position, u_matrixPalette[matrixIndex]);
    tmp.y = dot(position, u_matrixPalette[matrixIndex + 1]);
    tmp.z = dot(position, u_matrixPalette[matrixIndex + 2]);
    tmp.w = position.w;
    _skinnedPosition += blendWeight * tmp;
}

vec4 getPosition()
{
    _skinnedPosition = vec4(0.0);
    float blendWeight = a_blendWeights[0];
    int matrixIndex = int (a_blendIndices[0]) * 3;
    skinPosition(blendWeight, matrixIndex);
    blendWeight = a_blendWeights[1];
    matrixIndex = int(a_blendIndices[1]) * 3;
    skinPosition(blendWeight, matrixIndex);
    blendWeight = a_blendWeights[2];
    matrixIndex = int(a_blendIndices[2]) * 3;
    skinPosition(blendWeight, matrixIndex);
    blendWeight = a_blendWeights[3];
    matrixIndex = int(a_blendIndices[3]) * 3;
    skinPosition(blendWeight, matrixIndex);
    return _skinnedPosition;    
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

    vec3 getTangentSpaceVector(vec3 vector)
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
        return getTangentSpaceVector(a_normal);
    }

    #if defined(BUMPED)

        vec3 getTangent()
        {
            return getTangentSpaceVector(a_tangent);
        }

        vec3 getBinormal()
        {
            vec3 binormal = cross(a_tangent, a_normal);
            return getTangentSpaceVector(binormal);
        }

    #endif

#endif

