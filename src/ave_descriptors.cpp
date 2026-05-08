#include "ave_descriptors.hpp"

 
// std
#include <cassert>
#include <stdexcept>
#include <iostream>

namespace ave {

    // *************** Descriptor Set Layout Builder *********************
    AveDescriptorSetLayout::Builder& AveDescriptorSetLayout::Builder::addBinding(
        uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count) {
        assert(bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<AveDescriptorSetLayout> AveDescriptorSetLayout::Builder::build() const {
        return std::make_unique<AveDescriptorSetLayout>(aveDevice, bindings);
    }

    // *************** Descriptor Set Layout *********************
    AveDescriptorSetLayout::AveDescriptorSetLayout(AveDevice& aveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) : aveDevice{ aveDevice }, bindings{ bindings } {
        //create a vector of set layout bindings from the unordered map
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }

        //configure a vulkan create info struct
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        //call createdescriptorSetLayout to create the set layout and store it in descriptorSetLayout member variable
        if (vkCreateDescriptorSetLayout(aveDevice.device(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    AveDescriptorSetLayout::~AveDescriptorSetLayout() { //clean up descriptor set layout
        vkDestroyDescriptorSetLayout(aveDevice.device(), descriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************
    AveDescriptorPool::Builder& AveDescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count) {
        poolSizes.push_back({ descriptorType, count }); //implicit construction of VkDescriptorPoolSize struct
        return *this;
    }

    AveDescriptorPool::Builder& AveDescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags) {
        poolFlags = flags;
        return *this;
    }
    AveDescriptorPool::Builder& AveDescriptorPool::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<AveDescriptorPool> AveDescriptorPool::Builder::build() const {
        return std::make_unique<AveDescriptorPool>(aveDevice, maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool ********************* 
    AveDescriptorPool::AveDescriptorPool(AveDevice& aveDevice, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes) : aveDevice{ aveDevice } {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(aveDevice.device(), &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    AveDescriptorPool::~AveDescriptorPool() {
        vkDestroyDescriptorPool(aveDevice.device(), descriptorPool, nullptr);
    }

    //allocate a descriptor set from the pool
    bool AveDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(aveDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }
        return true;
    }

    void AveDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
        vkFreeDescriptorSets(aveDevice.device(), descriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
    }

    void AveDescriptorPool::resetPool() {
        vkResetDescriptorPool(aveDevice.device(), descriptorPool, 0);
    }

    // *************** Descriptor Writer *********************
    AveDescriptorWriter::AveDescriptorWriter(AveDescriptorSetLayout& setLayout, AveDescriptorPool& pool) : setLayout{ setLayout }, pool{ pool } {}

    AveDescriptorWriter& AveDescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    AveDescriptorWriter& AveDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    bool AveDescriptorWriter::build(VkDescriptorSet& set) {
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
        if (!success) {
            throw std::runtime_error("failed to allocate descriptor set from pool");
        }
        overwrite(set);
        return true;
    }

    void AveDescriptorWriter::overwrite(VkDescriptorSet& set) {
        for (auto& write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.aveDevice.device(), writes.size(), writes.data(), 0, nullptr);
    }
}