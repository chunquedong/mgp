#ifdef GL_ES
precision mediump float;
#endif

in vec2 v_texCoord;
out float fragColor;

uniform sampler2D u_texture;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(u_texture, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_texture, v_texCoord + offset).r;
        }
    }
    fragColor = result / (4.0 * 4.0);
}