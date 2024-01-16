#ifdef GL_ES
precision mediump float;
#endif

// Uniforms
uniform sampler2D u_texture;


// Inputs
in vec2 v_texCoord;
out vec4 FragColor;

void main()
{
    vec2 offset = 1.0 / vec2(textureSize(u_texture, 0));
    vec2 offsets[9] = vec2[](
        vec2(-offset.x,  offset.y), // 左上
        vec2( 0.0f,    offset.y), // 正上
        vec2( offset.x,  offset.y), // 右上
        vec2(-offset.x,  0.0f),   // 左
        vec2( 0.0f,    0.0f),   // 中
        vec2( offset.x,  0.0f),   // 右
        vec2(-offset.x, -offset.y), // 左下
        vec2( 0.0f,   -offset.y), // 正下
        vec2( offset.x, -offset.y)  // 右下
    );

    float kernel[9] = float[](
      1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
      2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
      1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0  
    );

    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
    {
        vec3 sampleTex = vec3(texture(u_texture, v_texCoord.st + offsets[i]));
        col += sampleTex * kernel[i];
    }
    FragColor = vec4(col, 1.0);
}