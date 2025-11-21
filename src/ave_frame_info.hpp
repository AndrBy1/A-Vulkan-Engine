#pragma once

// Do I need these? 
#include "ave_camera.hpp"
#include "ave_game_object.hpp"

#include <vulkan/vulkan.h>

namespace ave {

    #define MAX_LIGHTS 10

    struct PointLight{
        glm::vec4 position{}; //ignore w
        glm::vec4 color{}; //w is intensity
    };

    struct GlobalUbo{ //UBO is a struct that matches the layout of the uniform buffer in the shader
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::mat4 inverseView{1.f};
        glm::vec4 ambientLight{1.f, 1.f, 1.f, .02f};
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
    };

    struct FrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        AveCamera& camera;
        VkDescriptorSet globalDescriptorSet;
        AveGameObject::Map& gameObjects;
    };

}