#pragma once //pragma once prevents multiple inclusions of the same header file similar to #ifndef guards
#include "ave_window.hpp"
#include "ave_device.hpp"
#include "ave_swap_chain.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace ave {
    class AveRenderer {
        public:

        AveRenderer(AveWindow& window, AveDevice& device);
        ~AveRenderer();

        //becuase this is managing Vulkan objects for pipeline layout and command buffers, we should delete copy constructors 
        AveRenderer(const AveRenderer&) = delete; // Disable copy constructor
        AveRenderer& operator=(const AveRenderer&) = delete;

        //need to be able to accesss swap chain render pass
        VkRenderPass getSwapChainRenderPass() const {return aveSwapChain->getRenderPass();}
        float getAspectRatio() const {return aveSwapChain->extentAspectRatio();}

        bool isFrameInProgress() const {return isFrameStarted;}

        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(isFrameStarted && "Cannot get command buiffer when frame not in progess");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return currentFrameIndex;
        }

        //one function to begin the frame, second to end it
        //begin frame and beginSwaipChainRenderPass isn't combined into a single function because we want application to main control over this functionality 
        //so later we can easily integrate shadows, reflection, post processing
        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        //order here matters
        AveWindow& aveWindow;
        AveDevice& aveDevice;
        std::unique_ptr<AveSwapChain> aveSwapChain; //use unique pointer can easily create new width and height by constructing a new object
        //unique_ptr is a smart pointer that manages the lifetime of an object
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex;
        bool isFrameStarted;
    };
}

//rendere manages swapchain, command buffers and draw frame
//simple render system sets up pipeline pipeline layout, simple push constants struct and render game objects