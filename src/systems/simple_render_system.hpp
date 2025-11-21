#pragma once //pragma once prevents multiple inclusions of the same header file similar to #ifndef guards

#include "ave_camera.hpp"
#include "ave_pipeline.hpp"
#include "ave_device.hpp"
#include "ave_game_object.hpp"
#include "ave_frame_info.hpp"
#include "ave_model.hpp"
#include <memory>
#include <vector>

namespace ave {
    class SimpleRenderSystem {
        public:

        SimpleRenderSystem(AveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~SimpleRenderSystem();

        //becuase this is managing Vulkan objects for pipeline layout and command buffers, we should delete copy constructors 
        SimpleRenderSystem(const SimpleRenderSystem&) = delete; // Disable copy constructor
        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

        void renderGameObjects(FrameInfo& frameInfo);

        VkPipelineLayout& getPipelineLayout() { return pipelineLayout; }

        private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        //order here matters
        AveDevice& aveDevice;
        //unique_ptr is a smart pointer that manages the lifetime of an object
        std::unique_ptr<AvePipeline> avePipeline;
        VkPipelineLayout pipelineLayout;
    };
}

//rendere manages swapchain, command buffers and draw frame
//simple render system sets up pipeline pipeline layout, simple push constants struct and render game objects