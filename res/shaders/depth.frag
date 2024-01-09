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
    float depthValue = texture(u_texture, v_texCoord).r;

#if defined(DEPTH_TO_ALPHA)
    FragColor = vec4(1.0, 1.0, 1.0, depthValue);
#else
    FragColor = vec4(vec3(depthValue), 1.0);
#endif
}