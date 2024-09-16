#pragma once

#include "lv_device.hpp"

#include <memory>
#include <vector>
#include <unordered_map>

namespace lv
{
    class LvDescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            Builder(LvDevice& lvDevice) : device{ lvDevice } {};
            Builder& addBinding(
                uint32_t bindingIndex,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<LvDescriptorSetLayout> build() const;

        private:
            LvDevice& device;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>
                bindings{};
        };
        
        LvDescriptorSetLayout(
            LvDevice& lvDevice,
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>bindings);
        ~LvDescriptorSetLayout();
        LvDescriptorSetLayout(const LvDescriptorSetLayout&) = delete;
        LvDescriptorSetLayout& operator=(const LvDescriptorSetLayout&) 
            = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const 
        { return descriptorSetLayout; }
    
    private:
        LvDevice& device;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>
            bindings;

        friend class LvDescriptorWriter;
    };

    class LvDescriptorPool {
    public:
        class Builder {
        public:
            Builder(LvDevice& lvDevice) : device{ lvDevice } {}

            Builder& addPoolSize(
                VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<LvDescriptorPool> build() const;

        private:
            LvDevice& device;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        LvDescriptorPool(
            LvDevice& lvDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~LvDescriptorPool();
        LvDescriptorPool(const LvDescriptorPool&) = delete;
        LvDescriptorPool& operator=(const LvDescriptorPool&) = delete;

        bool allocateDescriptorSet(
            const VkDescriptorSetLayout descriptorSetLayout,
            VkDescriptorSet& descriptor) const;

        void freeDescriptorSets(
            std::vector<VkDescriptorSet>& descriptorSets) const;

        void resetPool();

    private:
        LvDevice& device;
        VkDescriptorPool descriptorPool;

        friend class LvDescriptorWriter;
    };

    class LvDescriptorWriter {
    public:
        LvDescriptorWriter(
            LvDescriptorSetLayout& setLayout,
            LvDescriptorPool& pool)
            : setLayout{ setLayout }, pool{ pool } {};

        LvDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        LvDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        LvDescriptorSetLayout& setLayout;
        LvDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };
}