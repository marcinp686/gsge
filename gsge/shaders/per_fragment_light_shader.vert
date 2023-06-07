#version 460

// #extension SPV_KHR_shader_draw_parameters : enable

layout(location = 0) in vec3 inPosition;    
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 fragNormal_WorldSpace;
layout(location = 1) out vec3 fragPosition_WorldSpace;
layout(location = 2) out vec3 fragLightVector_WorldSpace;

layout(std140, set=0, binding = 1) readonly buffer ObjectBuffer
{
    mat4 objects[];
} objectBuffer;   
    
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 normal;
    vec3 lightPosition;
    vec3 viewPosition;
} ubo;

void main() { 
    
    mat4 inTransform = objectBuffer.objects[gl_BaseInstance];

    fragPosition_WorldSpace = vec3(inTransform * vec4( inPosition, 1.0 ));
    fragNormal_WorldSpace = mat3(inverse(transpose(inTransform))) * inNormal;       
    fragLightVector_WorldSpace = ubo.lightPosition;    
    
    gl_Position = ubo.proj * ubo.view * inTransform * vec4(inPosition, 1.0);  
}