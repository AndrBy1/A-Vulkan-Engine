#include "point_light_system.hpp"
#include <stdexcept>

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <iostream>
#include <map>
#include <stdexcept>

namespace ave {

    struct PointLightPushConstants{
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

    PointLightSystem::PointLightSystem(AveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : aveDevice{device} {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem() {
        vkDestroyPipelineLayout(aveDevice.device(), pipelineLayout, nullptr);
    }

    void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout){

        VkPushConstantRange pushConstantRange{};
        //this signals that we want access to push constant data in both vertex and frag shaders
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; 
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstants);
        
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        //push constants are a way to efficiently send a small amount of data to shader programs
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(aveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }


    void PointLightSystem::createPipeline(VkRenderPass renderPass){
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipline layout");

        PipelineConfigInfo pipelineConfig{};
        AvePipeline::defaultPipelineConfigInfo(pipelineConfig); //betwe to use swapchain width and height since it might not match the windows
        AvePipeline::enableAlphaBlending(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        avePipeline = std::make_unique<AvePipeline>(aveDevice, "shaders/point_light.vert.spv", "shaders/point_light.frag.spv", pipelineConfig);

    }

    void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo){
        
        auto rotateLight = glm::rotate(glm::mat4(1.f), frameInfo.frameTime, {0.f, -1.f, 0.f});
        int lightIndex = 0;
        for(auto& kv : frameInfo.gameObjects){
            auto& obj = kv.second;
            if(obj.pointLight == nullptr) continue; //skip non point light objects

            assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

            //update light position
            obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));

            //copy light to ubo
            ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
            ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
            lightIndex++;
        }
        ubo.numLights = lightIndex;
    }

    void PointLightSystem::render(FrameInfo& frameInfo){
        //sort lights by distance to camera so that translucency works both ways
        std::map<float, AveGameObject::id_t> sorted; //map of float to gameobject pointer
        for(auto& kv : frameInfo.gameObjects){
            auto& obj = kv.second;
            if(obj.pointLight == nullptr) continue;

            //calculate distance
            auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
            float disSquared = glm::dot(offset, offset);
            sorted[disSquared] = obj.getId(); //map distance to gameobject pointer
        }

        avePipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1, 
            &frameInfo.globalDescriptorSet,
            0, nullptr
        );

        //iterate through sorted lights in reverse order
        for(auto it = sorted.rbegin(); it != sorted.rend(); ++it){
            //use game obj id to find light object 
            auto& obj = frameInfo.gameObjects.at(it->second);
            //if(obj.pointLight == nullptr) continue; 
            PointLightPushConstants push{};
            push.position = glm::vec4(obj.transform.translation, 1.f);
            push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
            push.radius = obj.transform.scale.x;

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PointLightPushConstants),
                &push
            );
            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0); //draw 6 vertices, 1 instance, first vertex 0, first instance 0
        }
        
    }
}