    
#ifndef DIRECTIONAL_LIGHT_COUNT
    #define DIRECTIONAL_LIGHT_COUNT 0
#endif
#ifndef SPOT_LIGHT_COUNT
    #define SPOT_LIGHT_COUNT 0
#endif
#ifndef POINT_LIGHT_COUNT
    #define POINT_LIGHT_COUNT 0
#endif
#if (DIRECTIONAL_LIGHT_COUNT > 0) || (POINT_LIGHT_COUNT > 0) || (SPOT_LIGHT_COUNT > 0) || defined(IBL)
    #define LIGHTING
#endif

#if defined(LIGHTING)

    #if (DIRECTIONAL_LIGHT_COUNT > 0)
        uniform vec3 u_directionalLightColor[DIRECTIONAL_LIGHT_COUNT];
        #if !defined(BUMPED)
            uniform vec3 u_directionalLightDirection[DIRECTIONAL_LIGHT_COUNT];
        #endif
    #endif

    #if (POINT_LIGHT_COUNT > 0)
        uniform vec3 u_pointLightColor[POINT_LIGHT_COUNT];
        uniform vec3 u_pointLightPosition[POINT_LIGHT_COUNT];
        uniform float u_pointLightRangeInverse[POINT_LIGHT_COUNT];
    #endif

    #if (SPOT_LIGHT_COUNT > 0)
        uniform vec3 u_spotLightColor[SPOT_LIGHT_COUNT];
        uniform float u_spotLightRangeInverse[SPOT_LIGHT_COUNT];
        uniform float u_spotLightInnerAngleCos[SPOT_LIGHT_COUNT];
        uniform float u_spotLightOuterAngleCos[SPOT_LIGHT_COUNT];
        #if !defined(BUMPED)
            uniform vec3 u_spotLightDirection[SPOT_LIGHT_COUNT];
        #endif
    #endif

#endif


#define NO_ALBEDO
#include "../_pbr.frag"

uniform sampler2D u_texture;
in vec2 v_texCoord;
out vec4 FragColor;


vec4 getLitPixel()
{
    vec3 normalVector = texture(u_texture, v_texCoord).xyz;
    vec4 combinedColor = vec4(0.0, 0.0, 0.0, 0.0);

    albedo = vec3(0.5, 0.5, 0.5);
    metallic = 0.5;
    roughness = 0.5;
    ao = 1.0;

    vec3 cameraDirection = vec3(0.0, 0.0, 1.0);

    // Directional light contribution
    #if (DIRECTIONAL_LIGHT_COUNT > 0)
    for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; ++i)
    {
        #if defined(BUMPED)
            vec3 lightDirection = normalize(v_directionalLightDirection[i] * 2.0);
        #else
            vec3 lightDirection = normalize(u_directionalLightDirection[i] * 2.0);
        #endif
        combinedColor += computeLighting(cameraDirection, normalVector, -lightDirection, u_directionalLightColor[i], 1.0);
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

void main()
{
    FragColor = getLitPixel();
    FragColor.a = 1.0;
}