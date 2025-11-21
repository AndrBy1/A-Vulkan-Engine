#include "ave_renderer.hpp"

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

    AveRenderer::AveRenderer(AveWindow &window, AveDevice& device) : aveWindow{window}, aveDevice{device} {
        recreateSwapChain();
        createCommandBuffers(); 
    }

    AveRenderer::~AveRenderer() {
        freeCommandBuffers();
    }

    void AveRenderer::recreateSwapChain(){
        auto extent = aveWindow.getExtent();
        while(extent.width == 0 || extent.height == 0){
            extent = aveWindow.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(aveDevice.device());

        if(aveSwapChain == nullptr){
            aveSwapChain = std::make_unique<AveSwapChain>(aveDevice, extent);
        }else{
            
            //can't copy unique pointer so use std::move
            //std::move is used to enable move semantics which allows data to move from one object to another. Moved from object is left unspecified state.
            std::shared_ptr<AveSwapChain> oldSwapChain = std::move(aveSwapChain);
            aveSwapChain = std::make_unique<AveSwapChain>(aveDevice, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*aveSwapChain.get())){
                throw std::runtime_error("Swap chain image (or depth) format has changed!");
            }
        }

        //aveSwapChain = std::make_unique<AveSwapChain>(aveDevice, extent);

        //if render pass compatible do nothing
    }

    
    void AveRenderer::createCommandBuffers(){
        commandBuffers.resize(AveSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; // Command buffer allocation info
        //primary command buffer can be submitted to a queue for execution, can't be called by other command buffer
        //secondary command buffer can not be submitted but can be called by other command buffers
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = aveDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(aveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void AveRenderer::freeCommandBuffers(){
        vkFreeCommandBuffers(aveDevice.device(), aveDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }


    VkCommandBuffer AveRenderer::beginFrame(){
        assert(!isFrameStarted && "Can't call beginFrame while already in progress");

        //function fetches index of the frame which would render to next
        //handles cpu gpu synchronization surrounding double or triple bufferings, return determines if process succesful
        auto result = aveSwapChain->acquireNextImage(&currentImageIndex);

        //error can return if window out of size
        if(result == VK_ERROR_OUT_OF_DATE_KHR){
            recreateSwapChain();
            return nullptr; //frame not started
        }

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            throw  std::runtime_error("failed to acquire swap chain image!");
        }

        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        //std::cout << "swap_chain commandBuffer: "<< commandBuffers[imageIndex] << std::endl;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        return commandBuffer;
    }
    void AveRenderer::endFrame(){
        assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
        auto commandBuffer = getCurrentCommandBuffer();
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }


        auto result = aveSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || aveWindow.wasWindowResized()){
            aveWindow.resetWindowResizedFlag();
            recreateSwapChain();
        }else if(result != VK_SUCCESS){
            throw std::runtime_error("failed to present swap chain image!");
        }

        isFrameStarted = false;

        currentFrameIndex = (currentFrameIndex + 1) % AveSwapChain::MAX_FRAMES_IN_FLIGHT;

    }
    void AveRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Cant begin render pass on command buffer from a different frame");
        

        //VkRenderPassBeginInfo is a struct used to specify parameters when beginning render pass. 
        //render pass groups sequence of rendering ops that share attachments like color and depth buffers, tells the graphics pipeline what layout to expect for an output frame buffer as well as some other info
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO; // Render pass begin info
        renderPassInfo.renderPass = aveSwapChain->getRenderPass();
        renderPassInfo.framebuffer = aveSwapChain->getFrameBuffer(currentImageIndex);
        //render area defines the area where the shader will load and stores will take place
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = aveSwapChain->getSwapChainExtent();

        //VkClearValue defines the initial clear values, depthStencil is the depth and stencil clear values to use when clearing image or attachment
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        // Begin the render pass
        //VK_SUBPASS_CONTENTS_INLINE signals that the commands will be recorded directly into the primary command buffer, no secondary command buffers will be used
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        //viewport describes how coordinates get mapped into final framebuffer space. It defines rectangular region where final image is displayed. 
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(aveSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(aveSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, aveSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }
    void AveRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer){
        assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Cant end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(commandBuffer);
    }
}