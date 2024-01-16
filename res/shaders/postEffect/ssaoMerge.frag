#ifdef GL_ES
precision mediump float;
#endif

in vec2 v_texCoord;

out vec4 FragColor;

uniform sampler2D u_main;
uniform sampler2D u_ssao;

void main()
{             
    vec4 color4 = texture(u_main, v_texCoord);
    vec3 color = color4.rgb;
    float ao = texture(u_ssao, v_texCoord).r;

    color -= vec3(0.3);
    color += vec3(0.3 * ao);

    FragColor = vec4(color, color4.a);
}
