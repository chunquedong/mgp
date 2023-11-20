#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

#ifdef EQUIRECTANGULAR_MAP
    uniform sampler2D u_diffuseTexture;
#else
    uniform samplerCube u_diffuseTexture;
#endif

in vec3 v_worldPos;
out vec4 FragColor;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
#ifdef EQUIRECTANGULAR_MAP
    vec2 uv = SampleSphericalMap(normalize(v_worldPos)); // make sure to normalize localPos
    FragColor = texture(u_diffuseTexture, uv);
#else
    FragColor = texture(u_diffuseTexture, v_worldPos);
#endif

    //FragColor = vec4(v_texCoord, 1.0);
}