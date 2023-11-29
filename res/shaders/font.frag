#ifdef GL_ES
#extension GL_OES_standard_derivatives : enable
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

///////////////////////////////////////////////////////////
// Uniforms
uniform sampler2D u_texture;

#ifdef DISTANCE_FIELD
    uniform vec2 u_cutoff;
    #ifdef FONT_OUTLINE
        uniform vec2 u_outline;
    #endif
#endif

///////////////////////////////////////////////////////////
// Varyings
in vec2 v_texCoord;
in vec4 v_color;

out vec4 FragColor;

void main()
{ 
    #ifdef DISTANCE_FIELD
        float distance = texture(u_texture, v_texCoord).r;
        //float smoothing = fwidth(distance);
        //float alpha = smoothstep(0.5 - smoothing * u_cutoff.x, 0.5 + smoothing * u_cutoff.y, distance);
        float alpha = smoothstep(u_cutoff.x-u_cutoff.y, u_cutoff.x+u_cutoff.y, distance);

        #ifdef FONT_OUTLINE
            vec4 outlineCol;
            outlineCol.a = smoothstep(u_outline.x-u_outline.y, u_outline.x+u_outline.y, distance);
            outlineCol.rgb = vec3(1.0, 1.0, 1.0) - v_color.rgb;
            FragColor.rgb = (outlineCol.rgb * outlineCol.a)*(1.0-alpha) + v_color.rgb * alpha;
            FragColor.a = (alpha + outlineCol.a*(1.0-alpha)) * v_color.a;
        #else
            FragColor = v_color;
            FragColor.a = alpha * v_color.a;
        #endif
    #else
    
        FragColor = v_color;
        FragColor.a = texture(u_texture, v_texCoord).a * v_color.a;
    
    #endif
}