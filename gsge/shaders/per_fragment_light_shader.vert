#version 450

layout(location = 0) in vec3 inPosition;    
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 fragNormal_WorldSpace;
layout(location = 1) out vec3 fragPosition_WorldSpace;
layout(location = 2) out vec3 fragLightVector_WorldSpace;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 normal;
    vec3 lightPosition;
} ubo;

void main() { 
    
    fragPosition_WorldSpace = vec3(ubo.normal * vec4( inPosition, 1.0 ));
    fragNormal_WorldSpace = mat3(inverse(transpose(ubo.normal))) * inNormal;       
    fragLightVector_WorldSpace = ubo.lightPosition;    
    
    gl_Position = ubo.proj * ubo.view * ubo.normal * vec4(inPosition, 1.0);  
}