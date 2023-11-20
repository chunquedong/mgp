
#if defined(LIGHTMAP)
    in vec2 a_texCoord1;
    out vec2 v_texCoord1;
#endif

#if defined(CLIP_PLANE)
    uniform mat4 u_worldMatrix;
    uniform vec4 u_clipPlane;
    out float v_clipDistance;
#endif


void applyCommonVert() {
    // Pass the lightmap texture coordinate
    #if defined(LIGHTMAP)
        v_texCoord1 = a_texCoord1;
    #endif

    #if defined(CLIP_PLANE)
        v_clipDistance = dot(u_worldMatrix * position, u_clipPlane);
    #endif
}
