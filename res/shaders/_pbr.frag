// IBL
#if defined(IBL)
    uniform samplerCube u_irradianceMap;
    uniform samplerCube u_prefilterMap;
    uniform sampler2D u_brdfLUT;
#endif

vec3  albedo;
float metallic;
float roughness;
float ao;
vec3  emissive;

uniform vec3 u_albedo;
uniform float u_metallic;
uniform float u_roughness;
uniform float u_ao;
uniform vec3 u_emissive;

#ifdef METALLIC_ROUGHNESS_MAP
    uniform sampler2D u_metallic_roughness_map;
    float getMetallic() {
        return u_metallic * texture(u_metallic_roughness_map, v_texCoord).b;
    }
    float getRoughness() {
        float roug = texture(u_metallic_roughness_map, v_texCoord).g;
        return u_roughness * roug;
    }
#else
    float getMetallic() {
        return u_metallic;
    }
    float getRoughness() {
        return u_roughness;
    }
#endif

#ifdef OCCLUSION_MAP
    uniform sampler2D u_occlusion_texture;
    float getAo() {
        return u_ao * texture(u_occlusion_texture, v_texCoord).r;
    }
#else
    float getAo() {
        return u_ao;
    }
#endif

#ifdef EMISSIVE_MAP
    uniform sampler2D u_emissive_map;
    vec3 getEmissive() {
        return u_emissive * texture(u_emissive_map, v_texCoord).rgb;
    }
#else
    vec3 getEmissive() {
        return u_emissive;
    }
#endif

#ifdef ALBEDO_MAP
    //uniform sampler2D u_albedoMap;
    vec3 getAlbedo() {
        return u_albedo * pow(_baseColor.rgb, vec3(2.2));
    }
#else
    vec3 getAlbedo() {
        return u_albedo;
    }
#endif

void initPbrParam() {
    albedo = getAlbedo();
    metallic = getMetallic();
    roughness = getRoughness();
    ao = getAo();
    emissive = getEmissive();
}

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------

#if defined(NO_ALBEDO)
    vec4
#else
    vec3
#endif
computeLighting(vec3 cameraDirection, vec3 normalVector, vec3 lightDirection, vec3 lightColor, float attenuation) {
    vec3 N = normalVector;
    vec3 V = cameraDirection;
    vec3 L = lightDirection;
    vec3 H = normalize(V + L);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 radiance = lightColor * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
        
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;
    
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;	  

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);        

#if defined(NO_ALBEDO)
    vec4 Lo = vec4(specular * radiance * NdotL, 0.0);
    //Lo.rgb = vec3(specular* NdotL);
    Lo.w = (kD.g / PI) * radiance.g * NdotL;
    return Lo;
#else
    // add to outgoing radiance Lo
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    //Lo = vec3(roughness, 0.0, 0.0);
    return Lo;
#endif
}

#if defined(IBL)
vec3 getAmbientColor(vec3 cameraDirection, vec3 normalVector) {
    vec3 N = normalVector;
    vec3 V = cameraDirection;
    vec3 R = reflect(-V, N);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(u_irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(u_prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(u_brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambientColor = (kD * diffuse + specular) * ao;
    //ambientColor = irradiance;
    return ambientColor;
}
#else
vec3 getAmbientColor(vec3 cameraDirection, vec3 normalVector) {
    vec3 ambientColor = vec3(0.03) * albedo * ao;
    return ambientColor;
}
#endif