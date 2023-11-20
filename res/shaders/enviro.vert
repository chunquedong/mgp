

uniform mat4 u_worldViewProjectionMatrix;
uniform mat4 u_inverseTransposeWorldMatrix;
uniform mat4 u_worldMatrix;

in vec3 a_position;
in vec3 a_normal;

out vec3 v_normalVector;
out vec3 v_position;

void main(void) {
  gl_Position = u_worldViewProjectionMatrix * vec4(a_position, 1.0);
  v_position = vec3(u_worldMatrix * vec4(a_position, 1.0));
  v_normalVector = (u_inverseTransposeWorldMatrix * vec4(a_normal, 1.0)).xyz;
}