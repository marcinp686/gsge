#version 450
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
} ubo;

const vec3 pointLightColor = {1, 1, 1};
const float pointLightPower = 2;
const float ambientLightPower = 0.025;
const vec3 materialDiffuseColor = {0.3,1,1};

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
    vec3 ambient = materialDiffuseColor * ambientLightPower * attenuation;

    outColor = diffuse + ambient;    
}