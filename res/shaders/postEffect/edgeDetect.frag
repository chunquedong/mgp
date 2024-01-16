#ifdef GL_ES
precision mediump float;
#endif

in vec2 v_texCoord;
uniform vec4 u_diffuseColor;

out vec4 fragColor;

uniform sampler2D u_texture;
uniform float u_filter;

float luminance(vec4 color) {
    return color.a == 0.0 ? 0.0 : 1.0;
    //return  0.2125 * color.r + 0.7154 * color.g + 0.0721 * color.b; 
}

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(u_texture, 0));
    float resultX = 0.0;
    float resultY = 0.0;

    float sobelX[9] = float[9](
        -1.0, 0.0, 1.0,
        -2.0, 0.0, 2.0,
        -1.0, 0.0, 1.0
    );

    float sobelY[9] = float[9](
        -1.0, -2.0, -1.0,
        0.0,  0.0,  0.0,
        1.0,  2.0,  1.0
    );

    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            float value = luminance(texture(u_texture, v_texCoord + offset));

            int sobelPos = (x+1) + (y+1) * 3;
            resultX += value * sobelX[sobelPos];
            resultY += value * sobelY[sobelPos];
        }
    }

    float edgeValue = sqrt((resultX * resultX) + (resultY * resultY));
    vec4 edgeColor = (u_filter < edgeValue) ? u_diffuseColor : vec4(0.0);

    fragColor = edgeColor;
}