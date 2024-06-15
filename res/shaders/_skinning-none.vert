#ifdef MORPH_TARGET_COUNT
    #include "_morph.vert"
#endif

#ifdef INSTANCED
    in mat4 a_instanceMatrix;
#else
    uniform mat4 u_worldViewMatrix;
#endif

vec4 getPosition()
{
    vec3 pos = a_position;
#ifdef MORPH_TARGET_COUNT
    pos = getMorphPosition(pos);
#endif

#ifdef INSTANCED
    return a_instanceMatrix * vec4(pos, 1.0);
#else
    return u_worldViewMatrix * vec4(pos, 1.0);
#endif
}

#if defined(LIGHTING)
    #if !defined(NORMAL_MAP)
        in vec3 a_normal;
    #endif

    #if defined(BUMPED)
        in vec3 a_tangent;
        //in vec3 a_binormal;
    #endif


    vec3 getViewSpaceVector(vec3 normal) {
        // Transform the normal, tangent and binormals to view space.
        #ifdef INSTANCED
            mat4 it = transpose(inverse(a_instanceMatrix));
            mat3 inverseTransposeWorldViewMatrix = mat3(it[0].xyz, it[1].xyz, it[2].xyz);
        #else
            mat3 inverseTransposeWorldViewMatrix = mat3(u_inverseTransposeWorldViewMatrix[0].xyz, u_inverseTransposeWorldViewMatrix[1].xyz, u_inverseTransposeWorldViewMatrix[2].xyz);
        #endif
        vec3 normalVector = normalize(inverseTransposeWorldViewMatrix * normal);
        return normalVector;
    }
    
    vec3 getNormal()
    {
        #if defined(NORMAL_MAP)
            return vec3(0.0, 0.0, 0.0);
        #else
            return getViewSpaceVector(a_normal);
        #endif
    }

    #if defined(BUMPED)
        vec3 getTangent()
        {
            return getViewSpaceVector(a_tangent);
        }

        vec3 getBinormal()
        {
            return getViewSpaceVector(cross(a_tangent, a_normal));
        }
    #endif

#endif