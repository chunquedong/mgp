#ifdef GL_ES
precision mediump float;
#endif

uniform mat4 u_projectionMatrix;
uniform mat4 u_inverseProjectionMatrix;

uniform sampler2D u_texture;
in vec2 v_texCoord;
out float FragColor;

uniform sampler2D u_texNoise;

#define SSAO_KERNEL_SIZE 12
uniform vec3 u_samples[SSAO_KERNEL_SIZE];

//uniform vec2 u_viewport;

#define LINEARIZE_DEPTH_FUNC
#include "../_common.frag"

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
float radius = 1.0;

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(800.0f/4.0f, 600.0f/4.0f); 
vec2 texSize;

vec3 vecgetViewSpacePosition(vec2 offset) {
    vec2 texPos = v_texCoord+(offset/texSize.xy);
    float depth = texture(u_texture, texPos).r;
    vec4 pos = vec4(texPos.x*2.0-1.0, texPos.y*2.0-1.0, depth*2.0-1.0, 1.0);
    vec4 vpos = u_inverseProjectionMatrix * pos;
    return vec3(vpos.xyz/vpos.w);
}

vec3 calcNormal(vec3 curPos) {
    vec3 right =  vecgetViewSpacePosition(vec2(1.0, 0.0));
    vec3 top =    vecgetViewSpacePosition(vec2(0.0, 1.0));
    vec3 n = normalize( cross((right-curPos),  (top-curPos)) );
    return n;
}

void main()
{
    texSize = textureSize(u_texture, 0);
    //vec2 texSize = u_viewport;

    // Get input for SSAO algorithm
    vec3 fragPos = vecgetViewSpacePosition(vec2(0.0, 0.0));
    vec3 normal = calcNormal(fragPos);
    vec3 randomVec = texture(u_texNoise, v_texCoord * noiseScale).xyz;

    // Create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // Iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < SSAO_KERNEL_SIZE; ++i)
    {
        // get sample position
        vec3 sample = TBN * u_samples[i]; // From tangent to view-space
        sample = fragPos + sample * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sample, 1.0);
        offset = u_projectionMatrix * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xy = offset.xy * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = texture(u_texture, offset.xy).r; // Get depth value of kernel sample
        sampleDepth = -linearizeDepth(sampleDepth);
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth ));
        occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / float(SSAO_KERNEL_SIZE));
    
    FragColor = occlusion;
}
