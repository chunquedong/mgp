#ifdef GL_ES
precision mediump float;
#endif

uniform samplerCube u_skybox;
uniform vec3 u_cameraPosition;

uniform vec3 u_albedo;
uniform float u_roughness;
uniform float u_metallic;

in vec3 v_position;
in vec3 v_normalVector;
out vec4 FragColor;



const float textureOffset = 1.0 / 400.0;
const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main(void) {

  vec3 offsets[6] = vec3[](
      vec3( textureOffset,    textureOffset,  0.0f),
      vec3( -textureOffset,   -textureOffset,  0.0f),
      vec3(-textureOffset,  0.0f,  textureOffset),
      vec3( textureOffset,  0.0f,  -textureOffset),
      vec3( 0.0f,    textureOffset,   textureOffset),
      vec3( 0.0f,    -textureOffset,  -textureOffset)
  );

  vec3 V = normalize(u_cameraPosition - v_position);
  vec3 N = normalize(v_normalVector);
  vec3 R = reflect(-V, N);
  vec3 specular = textureLod(u_skybox, R, u_roughness*10.0).rgb;
  
  float level = 13.0;
  vec3 irradiance = textureLod(u_skybox, R, level).rgb;
  for(int i = 0; i < 6; i++)
  {
    irradiance += vec3(textureLod(u_skybox, R + offsets[i], level));
  }
  irradiance /= 7.0;

  vec3 F0 = vec3(0.04); 
  F0 = mix(F0, u_albedo, u_metallic);
  vec3 fresnel = fresnelSchlick(dot(V,N), F0);

  vec3 kS = fresnel;
  vec3 kD = vec3(1.0) - kS;
  kD *= 1.0 - u_metallic;

  vec3 diffuse = irradiance * u_albedo;

  vec3 c = (kD * diffuse + specular*(kS+vec3(0.1)));

  FragColor = vec4(c, 1.0);
  //FragColor = vec4(vec3(kD), 1.0);
}
