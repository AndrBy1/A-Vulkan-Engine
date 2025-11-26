#include "simple_render_system.hpp"
#include <stdexcept>

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <iostream>

namespace ave {

    struct SimplePushConstantData{ //this matches the push in shaders
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f}; //use alignas to make right space in memory
    };

    SimpleRenderSystem::SimpleRenderSystem(AveDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts) : aveDevice{device}, setLayouts{setLayouts} {
        /*
        for(VkDescriptorSetLayout layout : setLayouts){
            createPipelineLayout(layout);
            //std::cout << "Descriptor Set Layout: " << layout << std::endl;
        }*/
        createPipelineLayout();
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() {
        vkDestroyPipelineLayout(aveDevice.device(), pipelineLayout, nullptr);
    }

    void SimpleRenderSystem::createPipelineLayout(){

        VkPushConstantRange pushConstantRange{};
        //this signals that we want access to push constant data in both vertex and frag shaders
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; 
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);
        
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        pipelineLayoutInfo.pSetLayouts = setLayouts.data();
        //push constants are a way to efficiently send a small amount of data to shader programs
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(aveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }


    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass){
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipline layout");

        PipelineConfigInfo pipelineConfig{};
        AvePipeline::defaultPipelineConfigInfo(pipelineConfig); //betwe to use swapchain width and height since it might not match the windows
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        avePipeline = std::make_unique<AvePipeline>(aveDevice, "shaders/shader.vert.spv", "shaders/shader.frag.spv", pipelineConfig);
    }

    void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo){

        avePipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1, 
            &frameInfo.globalDescriptorSet,
            0, nullptr
        );

        for(auto& obj: frameInfo.gameObjects){
            //std::cout << "Rendering GameObject ID: " << obj.first << std::endl;
            if(obj.second.model == nullptr) continue;
            VkDescriptorSet textureSet = obj.second.model->getTextureDescriptor();
            if(textureSet != VK_NULL_HANDLE){
                vkCmdBindDescriptorSets(
                    frameInfo.commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout,
                    1, 1, 
                    &textureSet,
                    0, nullptr
                );
            }
            SimplePushConstantData push{};
            push.modelMatrix = obj.second.transform.mat4();
            push.normalMatrix = obj.second.transform.normalMatrix();

            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

            obj.second.model->bind(frameInfo.commandBuffer);
            obj.second.model->draw(frameInfo.commandBuffer);
        }
        /*
        for(auto& kv: frameInfo.gameObjects){
            auto& obj = kv.second;
            if(obj.model == nullptr) continue;
            SimplePushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }*/
    }

}