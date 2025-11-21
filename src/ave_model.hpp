#pragma once

#include "ave_image.hpp"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace ave{
    class AveModel{
        public:

        struct Vertex{
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};
            glm::vec2 texCoord{}; //added for texture coordinates
            
            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex& other) const {
                return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
            }
        };

        struct Builder{
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string& filepath);
        };

        AveModel(AveDevice &device, const AveModel::Builder &builder);
        ~AveModel();

        //becuase this is managing Vulkan objects for pipeline layout and command buffers, we should delete copy constructors 
        AveModel(const AveModel&) = delete; // Disable copy constructor
        AveModel& operator=(const AveModel&) = delete;

        static std::unique_ptr<AveModel> createModelFromFile(AveDevice &device, const std::string& filepath);
        void attachTextureFromFile(const std::string& filepath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);   

        AveImage& getTextureImage(){ return textureImage; };
        VkDescriptorSet getTextureDescriptor() const { return textureDescriptor; }
        void setTextureDescriptor(VkDescriptorSet descriptor) {textureDescriptor = descriptor; }

        private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

        AveDevice& aveDevice;
        AveImage& textureImage = *new AveImage(aveDevice); //default texture image if none provided. maybe swap to unique pointer if necessary
        //VkBuffer is a raw block of memory on the GPU (or CPU) that you can use to store any kind of data — vertices, indices, uniform values, staging data, etc.
        std::unique_ptr<AveBuffer> vertexBuffer;
        uint32_t vertexCount;
        VkDescriptorSet textureDescriptor = VK_NULL_HANDLE;

        bool hasIndexBuffer = false;
        std::unique_ptr<AveBuffer> indexBuffer;
        uint32_t indexCount;
    };
}