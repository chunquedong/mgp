#if MORPH_TARGET_COUNT
    #include "_morph.vert"
#endif

#ifdef INSTANCED
    in mat4 a_instanceMatrix;
#endif

vec4 getPosition()
{
    vec3 pos = a_position;
#if MORPH_TARGET_COUNT
    pos = getMorphPosition(pos);
#endif

#ifdef INSTANCED
    return a_instanceMatrix * vec4(pos, 1.0);
#else
    return vec4(pos, 1.0);
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
    
    vec3 getNormal()
    {
        return a_normal;
    }

    #if defined(BUMPED)
        vec3 getTangent()
        {
            return a_tangent;
        }

        vec3 getBinormal()
        {
            return cross(a_tangent, a_normal);
        }
    #endif

#endif