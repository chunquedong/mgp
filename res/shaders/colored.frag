#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif


///////////////////////////////////////////////////////////
uniform vec4 u_diffuseColor;

#if defined(MODULATE_COLOR)
    uniform vec4 u_modulateColor;
#endif

#if defined(MODULATE_ALPHA)
    uniform float u_modulateAlpha;
#endif

#if defined(VERTEX_COLOR)
    in vec3 v_color;
#endif

///////////////////////////////////////////////////////////

vec4 _baseColor;

out vec4 FragColor;

///////////////////////////////////////////////////////////

#include "_common.frag"

#include "_lighting_def.glsl"
#if defined(LIGHTING)
    #include "_lighting.frag"
#endif


void main()
{
    #if defined(CLIP_PLANE)
        if(v_clipDistance < 0.0) discard;
    #endif
 
    #if defined(LIGHTING)
        #if defined(VERTEX_COLOR)
            _baseColor.rgb = v_color;
            _baseColor.a = 1.0;
        #else
            _baseColor = u_diffuseColor;
        #endif
        FragColor.a = _baseColor.a;
        FragColor.rgb = getLitPixel();
    #else
        #if defined(VERTEX_COLOR)
            FragColor.rgb = v_color;
            FragColor.a = 1.0;
        #else
            FragColor = u_diffuseColor;
        #endif
    #endif

	applyCommonFrag();
}
