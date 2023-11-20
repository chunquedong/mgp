
#include "_lighting_def.glsl"


///////////////////////////////////////////////////////////
// Uniforms
uniform mat4 u_worldViewProjectionMatrix;

///////////////////////////////////////////////////////////
// Attributes
in vec3 a_position;
in vec2 a_texCoord0;

///////////////////////////////////////////////////////////
// Varyings

out vec2 v_texCoord0;
#if LAYER_COUNT > 0
    out vec2 v_texCoordLayer0;
#endif
#if LAYER_COUNT > 1
    out vec2 v_texCoordLayer1;
#endif
#if LAYER_COUNT > 2
    out vec2 v_texCoordLayer2;
#endif

#if defined(SKINNING)
    #include "_skinning.vert"
#else
    #include "_skinning-none.vert" 
#endif

#if defined(LIGHTING)
    #include "_lighting.vert"
#endif

void main()
{
    // Transform position to clip space.
    vec4 position = vec4(a_position, 1.0);
    gl_Position = u_worldViewProjectionMatrix * position;

    #if defined(LIGHTING)

        #if !defined(NORMAL_MAP) 
            v_normalVector = normalize((u_inverseTransposeWorldViewMatrix * vec4(a_normal.x, a_normal.y, a_normal.z, 0)).xyz);
        #endif

        initLightDirection(position);

    #endif

    // Pass base texture coord
    v_texCoord0 = a_texCoord0;

    // Pass repeated texture coordinates for each layer
    #if LAYER_COUNT > 0
        v_texCoordLayer0 = a_texCoord0 * TEXTURE_REPEAT_0;
    #endif
    #if LAYER_COUNT > 1
        v_texCoordLayer1 = a_texCoord0 * TEXTURE_REPEAT_1;
    #endif
    #if LAYER_COUNT > 2
        v_texCoordLayer2 = a_texCoord0 * TEXTURE_REPEAT_2;
    #endif
}
