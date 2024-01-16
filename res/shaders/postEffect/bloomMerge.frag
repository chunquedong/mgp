#ifdef GL_ES
precision mediump float;
#endif

out vec4 FragColor;
in vec2 v_texCoord;

uniform sampler2D u_main;
uniform sampler2D u_texture;
const float exposure = 1.0;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(u_main, v_texCoord).rgb;      
    vec3 bloomColor = texture(u_texture, v_texCoord).rgb;
    hdrColor += bloomColor; // additive blending

    FragColor = vec4(hdrColor, 1.0f);
}