#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

in vec3 v_normalVector;

out vec4 FragColor;

void main()
{
    FragColor = vec4(v_normalVector, 1.0);
}
