#include "ave_model.hpp"

#include "ave_utils.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
/*
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>*/

#include <cassert>
#include <cstring>
#include <iostream> //can remove print statements later, use 4 now to check
#include <unordered_map>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace std{
    template<>
    struct hash<ave::AveModel::Vertex> {
        size_t operator()(const ave::AveModel::Vertex& vertex) const {
            size_t seed = 0;
            ave::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv, vertex.texCoord);
            return seed;
        }
    };
}

namespace ave
{
    AveModel::AveModel(AveDevice &device, const AveModel::Builder& builder) : aveDevice{device} {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
        //std::cout << "aveDevice address:" << &textureImage.getDevice() << "\n";
    }
    AveModel::~AveModel() {
        textureImage.~AveImage();
    }

    std::unique_ptr<AveModel> AveModel::createModelFromFile(AveDevice &device, const std::string& filepath){
        Builder builder{};
        builder.loadModel(ENGINE_DIR + filepath);
        std::cout << "Vertex count " << builder.vertices.size() << "\n";
        return std::make_unique<AveModel>(device, builder);
    }

    void AveModel::attachTextureFromFile(const std::string& filepath){
        textureImage.createTextureImage(filepath);

    }

    void AveModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount > 0 && "Vertex buffer must have at least 3 vertex");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        AveBuffer stagingBuffer{
            aveDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)vertices.data());    

        vertexBuffer = std::make_unique<AveBuffer>(
            aveDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        //memcpy takes vertices data and copies into host mapped memory region
        //with HOST_CORHERENT_BIT, host memory will automatically be flushed to update GPU memory

        // Create vertex buffer
        //VK_BUFFER_USAGE_VERTEX_BUFFER_BIT tells device that we want to create a buffer used to hold vertex input data
        //bitwise or operator | is used to combine multiple flags, It is used to compare the bit 1 or 0 and returns 1 when one is 1
        // HOST is the cpu

        aveDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }

    void AveModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if(!hasIndexBuffer){
            return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        AveBuffer stagingBuffer{
            aveDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)indices.data());


        //memcpy takes vertices data and copies into host mapped memory region
        //with HOST_CORHERENT_BIT, host memory will automatically be flushed to update GPU memory
        
        indexBuffer = std::make_unique<AveBuffer>(
            aveDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        // Create vertex buffer
        //VK_BUFFER_USAGE_VERTEX_BUFFER_BIT tells device that we want to create a buffer used to hold vertex input data
        //bitwise or operator | is used to combine multiple flags, It is used to compare the bit 1 or 0 and returns 1 when one is 1
        // HOST is the cpu

        aveDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }

    void AveModel::draw(VkCommandBuffer commandBuffer) {
        if(hasIndexBuffer){
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
            return;
        }else{
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }

    void AveModel::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = {vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        //vkCmdBindVertexBuffers tells GPU “For the next draw calls, here’s where you’ll get your vertex data from.”
        //commandBuffer – The command buffer you’re recording into.
        //firstBinding – The index of the first vertex buffer binding (usually 0).
        //bindingCount – How many buffers you’re binding in this call.
        //pBuffers – Pointer to an array of VkBuffer handles (your vertex buffers).
        //pOffsets – Pointer to an array of byte offsets inside each buffer where vertex data starts.
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        if(hasIndexBuffer){
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    std::vector<VkVertexInputBindingDescription> AveModel::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> AveModel::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)}); //texture only have 2 components so just R G
        attributeDescriptions.push_back({4, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord)}); //texture only have 2 components so just R G

        return attributeDescriptions;
    }

    void AveModel::Builder::loadModel(const std::string& filepath) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) { //this loops through each face element in the model getting the index values
                Vertex vertex{};

                if (index.vertex_index >= 0) { //vertex index is the first value of the face element and says what position value to use. index values are optional and negative is no index is provided
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2]
                    };
                }

                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1] // Flip V coordinate
                    };
                }

                if(uniqueVertices.count(vertex) == 0){ //if vertex is new we add it to the unique vertices map its position within the builder vertices vector is given by the vertices vectors current size
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]); //we push back the index of the vertex in the builder vertices vector
            }
        }
    }
}

//by using an index buffer, we save a lot of gpu memory