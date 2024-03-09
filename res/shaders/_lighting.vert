

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

out vec3 v_cameraDirection;

#if defined(SIMPLE_BUMPED) || defined(SHADOW)
    out vec3 v_positionViewSpace;
#endif


#if defined(BUMPED)
void initLightDirection(vec4 positionWorldViewSpace, mat3 tangentSpaceTransformMatrix)
{
    vec3 cameraPosition = vec3(0.0, 0.0, 0.0);
    
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
    
    // Compute camera direction and transform it to tangent space.
    v_cameraDirection = tangentSpaceTransformMatrix * (cameraPosition - positionWorldViewSpace.xyz);
}
#else
void initLightDirection(vec4 positionWorldViewSpace)
{
    vec3 cameraPosition = vec3(0.0, 0.0, 0.0);

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

	v_cameraDirection = cameraPosition - positionWorldViewSpace.xyz;
}

#endif

void applyLight(vec4 positionViewSpace) {
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
        initLightDirection(positionViewSpace, tangentSpaceTransformMatrix);
    #elif defined(NORMAL_MAP)
        initLightDirection(positionViewSpace);
    #else
        v_normalVector = normalVector;
        initLightDirection(positionViewSpace);
    #endif

    #if defined(SIMPLE_BUMPED) || defined(SHADOW)
        v_positionViewSpace = positionViewSpace.xyz / positionViewSpace.w;
    #endif
}
