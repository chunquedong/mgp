

///////////////////////////////////////////////////////////
// Uniforms
uniform mat4 u_projectionMatrix;

///////////////////////////////////////////////////////////
// Attributes
in vec3 a_position;
in vec2 a_texCoord;
in vec4 a_color;


///////////////////////////////////////////////////////////
// Varyings
out vec2 v_texCoord;
out vec4 v_color;


void main()
{
    gl_Position = u_projectionMatrix * vec4(a_position, 1.0);
    v_texCoord = a_texCoord;
    v_color = a_color;
}
