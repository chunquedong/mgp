#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

///////////////////////////////////////////////////////////
// Uniforms
uniform sampler2D u_diffuseTexture;

///////////////////////////////////////////////////////////
// Varyings
in vec2 v_texCoord;

out vec4 FragColor;

///////////////////////////////////////////////////////////
// Variables
vec4 _baseColor;

#define ALBEDO_MAP

#include "_common.frag"

#include "_lighting_def.glsl"
#if defined(LIGHTING)
    #include "_lighting.frag"
#endif

#ifdef TEXTURE_DISCARD_ALPHA
uniform float u_alphaCutoff;
#endif

void main()
{
    #if defined(CLIP_PLANE)
        if(v_clipDistance < 0.0) discard;
    #endif
 
    _baseColor = texture(u_diffuseTexture, v_texCoord);
 
    FragColor.a = _baseColor.a;

    #if defined(TEXTURE_DISCARD_ALPHA)
    if (FragColor.a < u_alphaCutoff)
        discard;
    #endif

    #if defined(LIGHTING)
        FragColor.rgb = getLitPixel();
    #else
        FragColor.rgb = _baseColor.rgb;
    #endif

	applyCommonFrag();
}
