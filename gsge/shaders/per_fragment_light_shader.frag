#version 460

// #extension SPV_KHR_shader_draw_parameters : enable
// #extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3 fragNormal_WorldSpace;
layout(location = 1) in vec3 fragPosition_WorldSpace;
layout(location = 2) in vec3 fragLightVector_WorldSpace;

layout(location = 0) out vec3 outColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 normal;
    vec3 lightPosition;
    vec3 viewPosition;
} ubo;

const vec3 pointLightColor = {1, 1, 1};
const float pointLightPower = 140;
const float ambientLightPower = 0.015;
const vec3 materialDiffuseColor = {0.01f, 0.3f, 1.0f};

// Light attenuation terms
const float Kc = 1.0;   // constant term
const float Kl = 0.7;  // linear term
const float Kd = 1.8;  // quadratic term

void main() {
    
    vec3 directionToLight = normalize(ubo.lightPosition - fragPosition_WorldSpace);    
    float distanceToLight = length(ubo.lightPosition - fragPosition_WorldSpace);
    vec3 norm = normalize(fragNormal_WorldSpace);        
    float cosTheta = max(dot(norm, directionToLight),0);         
    float attenuation = 1.0 / (Kc + Kl * distanceToLight + Kd * distanceToLight* distanceToLight );    
    vec3 diffuse = materialDiffuseColor * pointLightColor * pointLightPower * cosTheta * attenuation;      
    vec3 ambient = materialDiffuseColor * ambientLightPower;

    // specular
    vec3 viewDir = normalize(ubo.viewPosition-fragPosition_WorldSpace);
    vec3 halfwayDir = normalize(viewDir + directionToLight);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
    vec3 specular = pointLightColor * spec * pointLightColor;

    outColor = diffuse + ambient + specular;    
}