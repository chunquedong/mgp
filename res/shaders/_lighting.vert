

#if !defined(BUMPED) && !defined(NORMAL_MAP)
    out vec3 v_normalVector;
#endif

#if defined(BUMPED) && (DIRECTIONAL_LIGHT_COUNT > 0)
    out vec3 v_directionalLightDirection[DIRECTIONAL_LIGHT_COUNT];
#endif

#if (POINT_LIGHT_COUNT > 0)
    out vec3 v_vertexToPointLightDirection[POINT_LIGHT_COUNT];
#endif

#if (SPOT_LIGHT_COUNT > 0)
    out vec3 v_vertexToSpotLightDirection[SPOT_LIGHT_COUNT];
    #if defined(BUMPED)
        out vec3 v_spotLightDirection[SPOT_LIGHT_COUNT];
    #endif
#endif

#if !defined(NO_SPECULAR)
    out vec3 v_cameraDirection;
#endif

#if defined(SHADOW)
    out vec4 v_position;
#endif

#if defined(SIMPLE_BUMPED)
    out vec3 v_positionViewSpace;
#endif


#if defined(BUMPED)
void initLightDirection(vec4 position, mat3 tangentSpaceTransformMatrix)
{
    vec3 cameraPosition = vec3(0.0, 0.0, 0.0);

    #if !defined(NO_SPECULAR) || (POINT_LIGHT_COUNT > 0) || (SPOT_LIGHT_COUNT > 0) || defined(SHADOW)
        vec4 positionWorldViewSpace = u_worldViewMatrix * position;
    #endif
    
    #if (DIRECTIONAL_LIGHT_COUNT > 0)
    for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; ++i)
    {
        // Transform light direction to tangent space
        v_directionalLightDirection[i] = tangentSpaceTransformMatrix * u_directionalLightDirection[i];
    }
    #endif
    
    #if (POINT_LIGHT_COUNT > 0)
    for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
    {
        // Compute the vertex to light direction, in tangent space
        v_vertexToPointLightDirection[i] = tangentSpaceTransformMatrix * (u_pointLightPosition[i] - positionWorldViewSpace.xyz);
    }
    #endif
    
    #if (SPOT_LIGHT_COUNT > 0)
    for (int i = 0; i < SPOT_LIGHT_COUNT; ++i)
    {
        // Compute the vertex to light direction, in tangent space
	    v_vertexToSpotLightDirection[i] = tangentSpaceTransformMatrix * (u_spotLightPosition[i] - positionWorldViewSpace.xyz);
        v_spotLightDirection[i] = tangentSpaceTransformMatrix * u_spotLightDirection[i];
    }
    #endif
    
    #if !defined(NO_SPECULAR)
        // Compute camera direction and transform it to tangent space.
        v_cameraDirection = tangentSpaceTransformMatrix * (cameraPosition - positionWorldViewSpace.xyz);
    #endif
}
#else
void initLightDirection(vec4 position)
{
    vec3 cameraPosition = vec3(0.0, 0.0, 0.0);

    #if !defined(NO_SPECULAR) || (POINT_LIGHT_COUNT > 0) || (SPOT_LIGHT_COUNT > 0) || defined(SHADOW)
	    vec4 positionWorldViewSpace = u_worldViewMatrix * position;
    #endif

    #if (POINT_LIGHT_COUNT > 0)
    for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
    {
        // Compute the light direction with light position and the vertex position.
        v_vertexToPointLightDirection[i] = u_pointLightPosition[i] - positionWorldViewSpace.xyz;
    }
    #endif

    #if (SPOT_LIGHT_COUNT > 0)
    for (int i = 0; i < SPOT_LIGHT_COUNT; ++i)
    {
        // Compute the light direction with light position and the vertex position.
	    v_vertexToSpotLightDirection[i] = u_spotLightPosition[i] - positionWorldViewSpace.xyz;
    }
    #endif

    #if !defined(NO_SPECULAR)
	    v_cameraDirection = cameraPosition - positionWorldViewSpace.xyz;
    #endif
}

#endif

void applyLight(vec4 position) {
    vec3 normal = getNormal();
    // Transform the normal, tangent and binormals to view space.
    mat3 inverseTransposeWorldViewMatrix = mat3(u_inverseTransposeWorldViewMatrix[0].xyz, u_inverseTransposeWorldViewMatrix[1].xyz, u_inverseTransposeWorldViewMatrix[2].xyz);
    vec3 normalVector = normalize(inverseTransposeWorldViewMatrix * normal);
    
    #if defined(BUMPED)
        vec3 tangent = getTangent();
        vec3 binormal = getBinormal();
        vec3 tangentVector  = normalize(inverseTransposeWorldViewMatrix * tangent);
        vec3 binormalVector = normalize(inverseTransposeWorldViewMatrix * binormal);
        mat3 tangentSpaceTransformMatrix = mat3(tangentVector.x, binormalVector.x, normalVector.x, tangentVector.y, binormalVector.y, normalVector.y, tangentVector.z, binormalVector.z, normalVector.z);
        initLightDirection(position, tangentSpaceTransformMatrix);
    #else
        v_normalVector = normalVector;
        initLightDirection(position);
    #endif

    #if ((DIRECTIONAL_LIGHT_COUNT > 0) && defined(SHADOW))
        v_position = position;
    #endif

    #if defined(SIMPLE_BUMPED)
        vec4 positionViewSpace = u_worldViewMatrix * position;
        v_positionViewSpace = positionViewSpace.xyz / positionViewSpace.w;
    #endif
}
