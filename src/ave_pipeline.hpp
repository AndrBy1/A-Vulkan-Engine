#pragma once

#include "ave_device.hpp"

#include <string>
#include <vector>
namespace ave{
    //contains the data on how we want to configure pipeline, want this here bc we want application layer code to be easily able to configure pipeline easily and share config between mult pipelines
    struct PipelineConfigInfo{
        PipelineConfigInfo() = default; // allow default construction, needed for C++20
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
        //https://pastebin.com/EmsJWHzb
        
        //some shaders expect vertex data to be laid out in a certain way, binding descriptions describe how to read vertex data from vertex buffers, 
        //attribute descriptions describe how to extract vertex attributes from vertex data
        //binding descriptions describe at which rate to load data from memory throughout the vertices, ex: per-vertex or per-instance
        //attribute descriptions describe how to handle vertex input attributes within a vertex, ex: position, color, normal, texture coordinates
        //we make these vectors so that we can have multiple bindings and attributes
        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class AvePipeline{
        public: 
        AvePipeline(AveDevice& device, const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo);
        ~AvePipeline();

        AvePipeline(const AvePipeline&) = delete; // Disable copy constructor
        AvePipeline& operator=(const AvePipeline&) = delete; // Disable copy assignment

        void bind(VkCommandBuffer commandBuffer);

        //defaultPipelineConfigInfo is a static function that fills in a PipelineConfigInfo struct with default values for the pipeline configuration, 
        //useful for quickly setting up a pipeline with default settings and then modifying only the necessary fields
        static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
        static void enableAlphaBlending(PipelineConfigInfo& configInfo);

        private:
        //readFile helper function to read the contents of a file into a vector of chars, used for reading shader code from files
        static std::vector<char> readFile(const std::string& filePath);

        //createGraphicsPipeline creates a graphics pipeline using the provided vertex and fragment shader file paths and pipeline configuration info
        void createGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo);

        //createShaderModule helper function that creates a shader module from the provided shader code, used for creating vertex and fragment shader modules
        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

        AveDevice& aveDevice; // reference to the device, so we can use it to create pipeline
        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };
} //namespace ave