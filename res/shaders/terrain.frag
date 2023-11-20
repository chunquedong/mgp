#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

#include "_lighting_def.glsl"

///////////////////////////////////////////////////////////
// Uniforms

#if defined(DEBUG_PATCHES)
    uniform float u_row;
    uniform float u_column;
#endif

#if (LAYER_COUNT > 0)
    uniform sampler2D u_surfaceLayerMaps[SAMPLER_COUNT];
#endif

#if defined (NORMAL_MAP)
    uniform sampler2D u_normalMap;
#endif

///////////////////////////////////////////////////////////
// Varyings
#if defined(LIGHTING)
    #include "_lighting.frag"
#endif

in vec2 v_texCoord0;

#if (LAYER_COUNT > 0)
    in vec2 v_texCoordLayer0;
#endif
#if (LAYER_COUNT > 1)
    in vec2 v_texCoordLayer1;
#endif
#if (LAYER_COUNT > 2)
    in vec2 v_texCoordLayer2;
#endif

out vec4 FragColor;

///////////////////////////////////////////////////////////
// Variables
vec4 _baseColor;

#if (LAYER_COUNT > 1)
    void blendLayer(sampler2D textureMap, vec2 texCoord, float alphaBlend)
    {
        vec3 diffuse = texture(textureMap,  mod(texCoord, vec2(1,1))).rgb;
        _baseColor.rgb = _baseColor.rgb * (1.0 - alphaBlend) + diffuse * alphaBlend;
    }
#endif


void main()
{
    #if (LAYER_COUNT > 0)
        // Sample base texture
        _baseColor.rgb = texture(u_surfaceLayerMaps[TEXTURE_INDEX_0], mod(v_texCoordLayer0, vec2(1,1))).rgb;
        _baseColor.a = 1.0;
    #else
        // If no layers are defined, simply use a white color
        _baseColor = vec4(1, 1, 1, 1);
    #endif

    #if (LAYER_COUNT > 1)
        blendLayer(u_surfaceLayerMaps[TEXTURE_INDEX_1], v_texCoordLayer1, texture(u_surfaceLayerMaps[BLEND_INDEX_1], v_texCoord0)[BLEND_CHANNEL_1]);
    #endif
    #if (LAYER_COUNT > 2)
        blendLayer(u_surfaceLayerMaps[TEXTURE_INDEX_2], v_texCoordLayer2, texture(u_surfaceLayerMaps[BLEND_INDEX_2], v_texCoord0)[BLEND_CHANNEL_2]);
    #endif

    #if defined(DEBUG_PATCHES)
        float tint = mod(u_row + mod(u_column, 2.0), 2.0);
        _baseColor.rgb = _baseColor.rgb * 0.75 + vec3(1.0-tint, tint, 0) * 0.25;
    #endif

    #if defined(LIGHTING)
        #if defined(NORMAL_MAP)
            v_normalVector = texture(u_normalMap, v_texCoord0).xyz * 2.0 - 1.0;
            v_normalVector = (u_inverseTransposeWorldViewMatrix * vec4(v_normalVector.x, v_normalVector.y, v_normalVector.z, 0)).xyz;
        #endif
        FragColor.a = _baseColor.a;
        FragColor.rgb = getLitPixel();
    #else
        FragColor.rgb = _baseColor.rgb;
    #endif
}
