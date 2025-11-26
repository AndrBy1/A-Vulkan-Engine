#version 450

layout(location = 0) in vec3 positions;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal; 
layout(location = 3) in vec2 uv;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 texCoord;

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
	//mat4 modelMatrix = objectBuffer.objects[gl_InstanceIndex].model;
	//mat4 transformMatrix = (cameraData.viewproj * modelMatrix);
	//gl_Position = transformMatrix * vec4(positions, 1.0f);
	outColor = color;
	texCoord = uv;
}