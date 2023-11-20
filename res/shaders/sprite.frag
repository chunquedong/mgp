#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

///////////////////////////////////////////////////////////
// Uniforms
uniform sampler2D u_texture;

///////////////////////////////////////////////////////////
// Varyings
in vec2 v_texCoord;
in vec4 v_color;
out vec4 FragColor;

void main()
{
    FragColor = v_color * texture(u_texture, v_texCoord);
}
