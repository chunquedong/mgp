#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

uniform vec4 u_diffuseColor;

out vec4 FragColor;

void main()
{
    FragColor = u_diffuseColor;
}
