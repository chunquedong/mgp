layout (location = 0) in vec3 a_position;

uniform mat4 u_worldViewProjectionMatrix;


out vec3 v_worldPos;


void main()
{
  v_worldPos = a_position;
  vec4 pos = u_worldViewProjectionMatrix * vec4(a_position, 1.0);

#ifdef SKY_BOX
    gl_Position = pos.xyww;
#else
    gl_Position = pos;
#endif
}