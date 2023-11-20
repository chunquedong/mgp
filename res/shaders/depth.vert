

///////////////////////////////////////////////////////////
uniform mat4 u_worldViewProjectionMatrix;
in vec3 a_position;

#if defined(SKINNING)
    #include "_skinning.vert"
#else
    #include "_skinning-none.vert" 
#endif


#include "_common.vert"

void main()
{
    vec4 position = getPosition();
    gl_Position = u_worldViewProjectionMatrix * position;

    applyCommonVert();
}
