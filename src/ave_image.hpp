// this deals with loading images and creating image views and samplers for textures in Vulkan
// swapchain needs to be initialized before using this object. 

#pragma once


#include "ave_buffer.hpp"
#include "ave_descriptors.hpp"

#include <string>

namespace ave {
    class AveImage {
    public:
        AveImage(AveDevice& device);
        ~AveImage();

        void createTextureImage(const std::string& imagePath); //load an image and upload it to a Vulkan image object. 
        //void createVertexBuffer(); //might use the ones in model instead of using here. 

        VkDescriptorImageInfo descriptorInfo(VkImageLayout imageLayout);

        //for debugging
        VkImageView getImageView() const { return textureImageView; }

    private:
        void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

        // to set up a sampler object. 
        // Sampler objects define how to sample textures in shaders, including filtering modes, addressing modes, and mipmapping settings.
        void createTextureSampler();

        //VkImageView createImageView(VkImage image, VkFormat format);
        void createTextureImageView();

        VkImage textureImage = VK_NULL_HANDLE;
        VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
        VkImageView textureImageView = VK_NULL_HANDLE; // might create this in swap chain instead or not use
        VkSampler textureSampler = VK_NULL_HANDLE;

        AveDevice& aveDevice;

    };
}