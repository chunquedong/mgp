

uniform mat4 u_worldViewProjectionMatrix;

in vec3 a_position;



void main()
{
    vec4 position = vec4(a_position, 1.0);
    gl_Position = u_worldViewProjectionMatrix * position;
}
