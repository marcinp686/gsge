#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 normal;
    vec3 lightPosition;
} ubo;

layout(location = 0) in vec3 inPosition;    
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;

const vec3 materialDiffuseColor = { 0.3, 1, 1 };
const vec3 lightColor = { 1, 1, 1 };
const float lightPower = 2;
const float ambientLightPower = 0.025;

void main() {

    vec3 directionToLight = normalize( ubo.lightPosition - inPosition );   
    vec3 normalWorldSpace = normalize( mat3(ubo.normal) * inNormal );    
    float lightIntensity = ambientLightPower + max( dot( normalWorldSpace, directionToLight ),0 );
        
    gl_Position = ubo.proj * ubo.view * ubo.normal * vec4( inPosition, 1.0 );
    fragColor = materialDiffuseColor * lightIntensity * lightColor;        
}