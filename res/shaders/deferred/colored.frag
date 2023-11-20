#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif


///////////////////////////////////////////////////////////
uniform vec4 u_diffuseColor;
uniform sampler2D u_lightAcc;
uniform vec2 u_viewport;

#if defined(VERTEX_COLOR)
    in vec3 v_color;
#endif

out vec4 FragColor;

#include "../_common.frag"

void main()
{
    #if defined(CLIP_PLANE)
        if(v_clipDistance < 0.0) discard;
    #endif

    vec4 _baseColor;

    #if defined(VERTEX_COLOR)
        _baseColor = vec4(v_color, 1.0);
    #else
        _baseColor = u_diffuseColor;
    #endif
    vec2 viewport = textureSize(u_lightAcc, 0);
    vec2 texCoord = vec2(gl_FragCoord.x / viewport.x, gl_FragCoord.y / viewport.y);

    vec4 lightAcc = texture(u_lightAcc, texCoord);
    vec3 specularAcc = lightAcc.rgb;
    vec3 diffuseAcc = lightAcc.a * _baseColor.rgb;

    FragColor.rgb = specularAcc + diffuseAcc;
    FragColor.a = _baseColor.a;

	applyCommonFrag();
}
