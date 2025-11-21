//code borrowed from https://pastebin.com/yU7dMAxt

#pragma once
 
#include "ave_device.hpp"
 
// std
#include <memory>
#include <unordered_map>
#include <vector>
 
namespace ave {
 
    class AveDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(AveDevice &aveDevice) : aveDevice{aveDevice} {}
        
            Builder &addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1); //appends to map of bindings  
            std::unique_ptr<AveDescriptorSetLayout> build() const; //create an instance of the descriptor set layout wrapper class
        
        private:
            AveDevice &aveDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };
        
        AveDescriptorSetLayout(
            AveDevice &aveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~AveDescriptorSetLayout();
        AveDescriptorSetLayout(const AveDescriptorSetLayout &) = delete;
        AveDescriptorSetLayout &operator=(const AveDescriptorSetLayout &) = delete;
        
        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
        
    private:
        AveDevice &aveDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
        
        friend class AveDescriptorWriter;
    };

    class AveDescriptorPool {
    public:
        class Builder {
        public:
            Builder(AveDevice &aveDevice) : aveDevice{aveDevice} {}
        
            Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder &setMaxSets(uint32_t count);
            std::unique_ptr<AveDescriptorPool> build() const;
        
        private:
            AveDevice &aveDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };
        AveDescriptorPool(
            AveDevice &aveDevice,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize> &poolSizes);
        ~AveDescriptorPool();
        AveDescriptorPool(const AveDescriptorPool &) = delete;
        AveDescriptorPool &operator=(const AveDescriptorPool &) = delete;
        
        bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;
        
        void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;
        
        void resetPool();
        
        private:
        AveDevice &aveDevice;
        VkDescriptorPool descriptorPool;
        
        friend class AveDescriptorWriter;
    };

    class AveDescriptorWriter { //make building the descriptor sets easier
        public:
        AveDescriptorWriter(AveDescriptorSetLayout &setLayout, AveDescriptorPool &pool);
        
        AveDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
        AveDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);
        
        bool build(VkDescriptorSet &set);
        void overwrite(VkDescriptorSet &set);
        
        private:
        AveDescriptorSetLayout &setLayout;
        AveDescriptorPool &pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}  // namespace vke