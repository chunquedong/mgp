#ifdef GL_ES
    #ifdef GL_FRAGMENT_PRECISION_HIGH
        precision highp float;
    #else
        precision mediump float;
    #endif
#endif

in vec2 v_texCoord;

out vec4 FragColor;

uniform sampler2D u_texture;

#include "../_common.frag"

void main()
{             
    FragColor = texture(u_texture, v_texCoord);

    vec3 color = FragColor.rgb;

#ifdef TONE_MAPPING
    color = hdrTonemapping(color);
#else
    color = hdrTonemapping2(color);
#endif

#ifdef GAMMA
    color = gammaCorrect(color);
#endif

    FragColor.rgb = color;
}
