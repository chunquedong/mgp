
#include "_lighting_def.glsl"

///////////////////////////////////////////////////////////

uniform mat4 u_projectionMatrix;

#if defined(TEXTURE_REPEAT)
    uniform vec2 u_textureRepeat;
#endif

#if defined(TEXTURE_OFFSET)
    uniform vec2 u_textureOffset;
#endif

///////////////////////////////////////////////////////////
in vec3 a_position;
in vec2 a_texCoord;

out vec2 v_texCoord;

///////////////////////////////////////////////////////////

#if defined(SKINNING)
    #include "_skinning.vert"
#else
    #include "_skinning-none.vert" 
#endif

#if defined(LIGHTING)
    #include "_lighting.vert"
#endif

#include "_common.vert"

void main()
{
    vec4 position = getPosition();

    gl_Position = u_projectionMatrix * position;

    #if defined(LIGHTING)
        applyLight(position);
    #endif 
    
    v_texCoord = a_texCoord;
    
    #if defined(TEXTURE_REPEAT)
        v_texCoord *= u_textureRepeat;
    #endif
    
    #if defined(TEXTURE_OFFSET)
        v_texCoord += u_textureOffset;
    #endif
    
    applyCommonVert();
}
