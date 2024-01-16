#ifdef GL_ES
    #ifdef GL_FRAGMENT_PRECISION_HIGH
        precision highp float;
    #else
        precision mediump float;
    #endif
#endif

out vec4 FragColor;
in vec2 v_texCoord;

uniform sampler2D u_texture;

float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{             
    vec2 tex_offset = 1.0 / textureSize(u_texture, 0); // gets size of single texel
    vec3 result = texture(u_texture, v_texCoord).rgb * weight[0]; // current fragment's contribution

#ifdef GAUSS_HORIZONTAL
        for(int i = 1; i < 5; ++i)
        {
            result += texture(u_texture, v_texCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(u_texture, v_texCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
#else
        for(int i = 1; i < 5; ++i)
        {
            result += texture(u_texture, v_texCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(u_texture, v_texCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
#endif
    FragColor = vec4(result, 1.0);
}