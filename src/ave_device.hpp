#pragma once

#include "ave_window.hpp"


// std lib headers
#include <string>
#include <vector>

namespace ave {

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
  uint32_t graphicsFamily;
  uint32_t presentFamily;
  bool graphicsFamilyHasValue = false;
  bool presentFamilyHasValue = false;
  bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

class AveDevice {
 public:
#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  AveDevice(AveWindow &window);
  ~AveDevice();

  // Not copyable or movable
  AveDevice(const AveDevice &) = delete;
  AveDevice& operator=(const AveDevice &) = delete;
  AveDevice(AveDevice &&) = delete;
  AveDevice &operator=(AveDevice &&) = delete;

	//VkCommandPool is a Vulkan object that manages the memory and lifecycle of command buffers, which are used to record and submit rendering commands to the GPU
  VkCommandPool getCommandPool() { return commandPool; }
	//VkDevice is a logical representation of a physical GPU, used to interact with the GPU and manage resources
  VkDevice device() { return device_; }
	//VkSurfaceKHR is an abstraction for a platform-specific surface that can be used for rendering and presentation. 
  //Surface is a Vulkan object that represents a surface to present rendered images to, such as a window or display
  VkSurfaceKHR surface() { return surface_; }
	//VkQueue is a handle to a queue on a device, used to submit command buffers for execution
  VkQueue graphicsQueue() { return graphicsQueue_; }
  VkQueue presentQueue() { return presentQueue_; }

	//SwapChainSupportDetails is a struct that contains information about 
  //the swap chain support of a physical device, including its capabilities, supported formats, and present modes
  SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
	//findMemoryType is a function that finds a suitable memory type for a given type filter and memory properties
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
  VkFormat findSupportedFormat(
      const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

  // Buffer Helper Functions
  void createBuffer(
      VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkBuffer &buffer,
      VkDeviceMemory &bufferMemory);
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void copyBufferToImage(
      VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

  void createImageWithInfo(
      const VkImageCreateInfo &imageInfo,
      VkMemoryPropertyFlags properties,
      VkImage &image,
      VkDeviceMemory &imageMemory);

  VkPhysicalDeviceProperties properties; 
    
  //tecnically doesn't belong here but swapchain and image use this. 
  //VkImageView createImageView(VkImage image, VkFormat format); 
  void createImageView(VkImage image, VkFormat format, VkImageView& view);

 private:
  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createCommandPool();

  // helper functions
  bool isDeviceSuitable(VkPhysicalDevice device);
  std::vector<const char *> getRequiredExtensions();
  bool checkValidationLayerSupport();
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  void hasGflwRequiredInstanceExtensions();
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  AveWindow &window;
  VkCommandPool commandPool;

  VkDevice device_;
  VkSurfaceKHR surface_;
  VkQueue graphicsQueue_;
  VkQueue presentQueue_;

  const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

}  // namespace