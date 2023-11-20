
#if defined(LIGHTMAP)
    uniform sampler2D u_lightmapTexture;
    in vec2 v_texCoord1;
#endif

#if defined(CLIP_PLANE)
    in float v_clipDistance;
#endif

#if defined(MODULATE_COLOR)
    uniform vec4 u_modulateColor;
#endif

#if defined(MODULATE_ALPHA)
    uniform float u_modulateAlpha;
#endif

#if defined(LINEARIZE_DEPTH) || defined(LINEARIZE_DEPTH_FUNC) || defined(SHADOW)
    uniform float u_nearPlane;
    uniform float u_farPlane;

    float linearizeDepth(float depth)
    {
        float z = depth * 2.0 - 1.0;
        return (2.0 * u_nearPlane * u_farPlane) / (u_farPlane + u_nearPlane - z * (u_farPlane - u_nearPlane));    
    }
#endif

//HDR to LDR
vec3 hdrTonemapping(vec3 color) {
    return color / (color + vec3(1.0));
}

// tone mapping
vec3 hdrTonemapping2(vec3 hdrColor) {
    const float exposure = 1.0;
    return vec3(1.0) - exp(-hdrColor * exposure);
}

vec3 gammaCorrect(vec3 color) {
    return  pow(color, vec3(1.0/2.2)); 
}

void applyCommonFrag() {
    #if defined(LIGHTMAP)
        vec4 lightColor = texture(u_lightmapTexture, v_texCoord1);
        FragColor.rgb *= lightColor.rgb;
	#endif

    #if defined(LDR)
        vec3 color = FragColor.rgb;
        color = hdrTonemapping(color);
        color = gammaCorrect(color);
        FragColor.rgb = color;
    #endif

    #if defined(MODULATE_COLOR)
        FragColor *= u_modulateColor;
    #endif

    #if defined(MODULATE_ALPHA)
        FragColor.a *= u_modulateAlpha;
    #endif
}