
#if !defined(BUMPED)
    in vec3 v_normalVector;
#endif


#if (DIRECTIONAL_LIGHT_COUNT > 0)
    in vec3 v_directionalLightDirection[DIRECTIONAL_LIGHT_COUNT];
#endif

#if (POINT_LIGHT_COUNT > 0)
    in vec3 v_vertexToPointLightDirection[POINT_LIGHT_COUNT];
#endif

#if (SPOT_LIGHT_COUNT > 0)
    in vec3 v_vertexToSpotLightDirection[SPOT_LIGHT_COUNT];
    #if defined(BUMPED)
        in vec3 v_spotLightDirection[SPOT_LIGHT_COUNT];
    #endif
#endif

#if !defined(NO_SPECULAR)
    in vec3 v_cameraDirection;

    #if !defined(PBR)
        uniform float u_specularExponent;
    #endif
#endif

#if defined(SHADOW)
    in vec4 v_position;
#endif

#if defined(SIMPLE_BUMPED)
    in vec3 v_positionViewSpace;
#endif

#if defined(SIMPLE_BUMPED)
// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anyways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap(vec3 normalVector)
{
    vec3 tangentNormal = texture(u_normalmapTexture, v_texCoord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(v_positionViewSpace);
    vec3 Q2  = dFdy(v_positionViewSpace);
    vec2 st1 = dFdx(v_texCoord);
    vec2 st2 = dFdy(v_texCoord);

    vec3 N   = normalize(normalVector);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    //vec3 T  = normalize((Q1*st2.t - Q2*st1.t)/(st1.s*st2.t - st2.s*st1.t));
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
#endif

#if defined(PBR)
    #include "_pbr.frag"
#else
    uniform vec3 u_ambientColor;

    vec3 computeLighting(vec3 cameraDirection, vec3 normalVector, vec3 lightDirection, vec3 lightColor, float attenuation)
    {
        float diffuse = max(dot(normalVector, lightDirection), 0.0);
        vec3 diffuseColor = lightColor * _baseColor.rgb * diffuse * attenuation;

        #if !defined(NO_SPECULAR)

            // Phong shading
            //vec3 vertexToEye = cameraDirection;
            //vec3 specularAngle = normalize(normalVector * diffuse * 2.0 - lightDirection);  
            //vec3 specularColor = vec3(pow(clamp(dot(specularAngle, vertexToEye), 0.0, 1.0), u_specularExponent)); 

            // Blinn-Phong shading
            vec3 vertexToEye = cameraDirection;
            vec3 halfVector = normalize(lightDirection + vertexToEye);
            float specularAngle = clamp(dot(normalVector, halfVector), 0.0, 1.0);
            vec3 specularColor = vec3(pow(specularAngle, u_specularExponent)) * attenuation;

            return diffuseColor + specularColor;

        #else
        
            return diffuseColor;
        
        #endif
    }
#endif

#if defined(SHADOW)

    int findCascadeLayer(int lightIndex) {
        float depthValue = linearizeDepth(gl_FragCoord.z);

        for (int i = SHADOW_CASCADE_COUNT-1; i >= 0; --i) {
            if (depthValue > u_directionalLightCascadeDistance[(lightIndex * SHADOW_CASCADE_COUNT)+i]) {
                return i;
            }
        }
        return 0;
    }

    float sampleShadowMap(int globalIndex, vec2 projCoords) {
        if (projCoords.x < 0 || projCoords.y < 0 || projCoords.x > 1.0 || projCoords.y > 1.0) return 1.0;

        float closestDepth = texture(u_directionalLightShadowMap[globalIndex], projCoords.xy).r;
        return closestDepth;
    }

    float shadowCalculation(int lightIndex, vec3 normal, vec3 lightDir)
    {
        int cascadeLayer = findCascadeLayer(lightIndex);
        int globalIndex = (lightIndex * SHADOW_CASCADE_COUNT) + cascadeLayer;
        vec4 fragPosLightSpace = u_directionalLightSpaceMatrix[globalIndex] * v_position;

        // perform perspective divide
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
        if(projCoords.z > 1.0) {
            return 0.0;
        }
        // Transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
    #if 0
        float closestDepth = sampleShadowMap(globalIndex, projCoords.xy);
        float currentDepth = projCoords.z;
        float shadow = currentDepth-0.005 > closestDepth  ? 1.0 : 0.0;
        return shadow;
    #else
        // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = sampleShadowMap(globalIndex, projCoords.xy); 
        // Get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        // Calculate bias (based on depth map resolution and slope)
        //vec3 normal = normalize(fs_in.Normal);
        //vec3 lightDir = normalize(lightPos - fs_in.FragPos);
        float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
        // Check whether current frag pos is in shadow
        // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
        // PCF
        float shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(u_directionalLightShadowMap[globalIndex], 0);
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = sampleShadowMap(globalIndex, projCoords.xy + vec2(x, y) * texelSize); 
                shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;
        
        return shadow;
    #endif
    }
#endif

vec3 getLitPixel()
{
    #if defined(BUMPED)
        vec3 normalVector = normalize(texture(u_normalmapTexture, v_texCoord).rgb * 2.0 - 1.0);
    #elif defined(SIMPLE_BUMPED)
        vec3 normalVector = getNormalFromMap(v_normalVector);
    #else
        vec3 normalVector = normalize(v_normalVector);
    #endif

    vec3 cameraDirection = normalize(v_cameraDirection);
    
    #if defined(PBR)
        initPbrParam();
        vec3 ambientColor = getAmbientColor(cameraDirection, normalVector);
    #else
        vec3 ambientColor = _baseColor.rgb * u_ambientColor;
    #endif

    vec3 combinedColor = ambientColor;

    #if defined(PBR)
        combinedColor += emissive;
    #endif

    // Directional light contribution
    #if (DIRECTIONAL_LIGHT_COUNT > 0)
    for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; ++i)
    {
        #if defined(BUMPED)
            vec3 lightDirection = normalize(v_directionalLightDirection[i] * 2.0);
        #else
            vec3 lightDirection = normalize(u_directionalLightDirection[i] * 2.0);
        #endif

        vec3 lightRes = computeLighting(cameraDirection, normalVector, -lightDirection, u_directionalLightColor[i], 1.0);

        #if defined(SHADOW)
            float shadow = shadowCalculation(i, normalVector, -lightDirection);
            shadow = min(shadow, 0.75); // reduce shadow strength a little: allow some diffuse/specular light in shadowed regions
            combinedColor += (1.0 - shadow) * lightRes;
        #else
            combinedColor += lightRes;
        #endif
    }
    #endif

    // Point light contribution
    #if (POINT_LIGHT_COUNT > 0)
    for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
    {
        vec3 ldir = v_vertexToPointLightDirection[i] * u_pointLightRangeInverse[i];
        float attenuation = clamp(1.0 - dot(ldir, ldir), 0.0, 1.0);
        combinedColor += computeLighting(cameraDirection, normalVector, normalize(v_vertexToPointLightDirection[i]), u_pointLightColor[i], attenuation);
    }
    #endif

    // Spot light contribution
    #if (SPOT_LIGHT_COUNT > 0)
    for (int i = 0; i < SPOT_LIGHT_COUNT; ++i)
    {
        // Compute range attenuation
        vec3 ldir = v_vertexToSpotLightDirection[i] * u_spotLightRangeInverse[i];
        float attenuation = clamp(1.0 - dot(ldir, ldir), 0.0, 1.0);
        vec3 vertexToSpotLightDirection = normalize(v_vertexToSpotLightDirection[i]);

        #if defined(BUMPED)
            vec3 spotLightDirection = normalize(v_spotLightDirection[i] * 2.0);
        #else
            vec3 spotLightDirection = normalize(u_spotLightDirection[i] * 2.0);
        #endif

        // "-lightDirection" is used because light direction points in opposite direction to spot direction.
        float spotCurrentAngleCos = dot(spotLightDirection, -vertexToSpotLightDirection);

		// Apply spot attenuation
        attenuation *= smoothstep(u_spotLightOuterAngleCos[i], u_spotLightInnerAngleCos[i], spotCurrentAngleCos);
        combinedColor += computeLighting(cameraDirection, normalVector, vertexToSpotLightDirection, u_spotLightColor[i], attenuation);
    }
    #endif

    return combinedColor;
}
