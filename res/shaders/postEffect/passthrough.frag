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

void main()
{             
    FragColor = texture(u_texture, v_texCoord);
}
