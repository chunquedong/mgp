
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
#ifndef SHADOW_CASCADE_COUNT
    #define SHADOW_CASCADE_COUNT 2
#endif

#if defined(LIGHTING)

    #if defined(BUMPED) || defined(SIMPLE_BUMPED)
        uniform sampler2D u_normalmapTexture;
    #endif

    uniform mat4 u_inverseTransposeWorldViewMatrix;

    #if (DIRECTIONAL_LIGHT_COUNT > 0)
        uniform vec3 u_directionalLightColor[DIRECTIONAL_LIGHT_COUNT];
        #if defined(SHADOW)
            uniform sampler2D u_directionalLightShadowMap[DIRECTIONAL_LIGHT_COUNT];
            uniform mat4 u_directionalLightSpaceMatrix[DIRECTIONAL_LIGHT_COUNT*SHADOW_CASCADE_COUNT];
            uniform float u_directionalLightCascadeDistance[DIRECTIONAL_LIGHT_COUNT*SHADOW_CASCADE_COUNT];
        #endif
        uniform vec3 u_directionalLightDirection[DIRECTIONAL_LIGHT_COUNT];
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
        uniform vec3 u_spotLightDirection[SPOT_LIGHT_COUNT];
    #endif

#endif
