#include "_lighting_def.glsl"



#if defined(VERTEX_COLOR)
    in vec3 a_color;
    out vec3 v_color;
#endif

///////////////////////////////////////////////////////////
uniform mat4 u_projectionMatrix;
in vec3 a_position;

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

    #if defined (LIGHTING)
        applyLight(position);
    #endif
    
    // Pass the vertex color
    #if defined(VERTEX_COLOR)
	    v_color = a_color;
    #endif

    applyCommonVert();
}
