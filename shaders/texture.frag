#version 450

//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 texCoord;
//output write
layout (location = 0) out vec4 outFragColor;


struct PointLight{
  vec4 position; //ignore w
  vec4 color; //w is intensity
}; 

//uniforms and ssbos
layout(set = 0, binding = 0) uniform GlobalUbo{ 
    mat4 projection; //combined projection and view matrix
    mat4 view; //combined projection and view matrix
    mat4 invView;
    vec4 ambientLightColor; //w is intensity
    PointLight pointLights[10]; //specialization constants is a method of passing constant values to shaders at pipeline creation time
    int numLights;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D texSampler; //texture sampler


void main()
{
	vec3 color = texture(texSampler,texCoord).xyz;
	outFragColor = vec4(color,1.0f);
}
