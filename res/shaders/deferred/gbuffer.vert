
uniform mat4 u_projectionMatrix;
uniform mat4 u_inverseTransposeWorldViewMatrix;

in vec3 a_position;

#define LIGHTING

#if defined(SKINNING)
    #include "../_skinning.vert"
#else
    #include "../_skinning-none.vert" 
#endif

out vec3 v_normalVector;

void main()
{
    vec4 position = getPosition();
    gl_Position = u_projectionMatrix * position;

    vec3 normal = getNormal();
    // Transform the normal, tangent and binormals to view space.
    mat3 inverseTransposeWorldViewMatrix = mat3(u_inverseTransposeWorldViewMatrix[0].xyz, u_inverseTransposeWorldViewMatrix[1].xyz, u_inverseTransposeWorldViewMatrix[2].xyz);
    vec3 normalVector = normalize(inverseTransposeWorldViewMatrix * normal);

    v_normalVector = normalVector;
}
