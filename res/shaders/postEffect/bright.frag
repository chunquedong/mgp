#ifdef GL_ES
precision mediump float;
#endif

in vec2 v_texCoord;

out vec4 FragColor;

uniform sampler2D u_texture;

uniform float u_brightLimit;

void main()
{             
    vec3 color = texture(u_texture, v_texCoord).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > u_brightLimit)
        FragColor = vec4(color, 1.0);
    else
        FragColor = vec4(0.0);
}
