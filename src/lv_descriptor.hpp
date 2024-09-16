#pragma once

#include "lv_device.hpp"

#include <memory>
#include <vector>
#include <unordered_map>

namespace lv
{
    class LvDescriptorLayout
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
            std::unique_ptr<LvDescriptorLayout> build() const;

        private:
            LvDevice& device;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>
                bindings{};
        };
        
        LvDescriptorLayout(
            LvDevice& lvDevice,
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>bindings);
        ~LvDescriptorLayout();
        LvDescriptorLayout(const LvDescriptorLayout&) = delete;
        LvDescriptorLayout& operator=(const LvDescriptorLayout&) =
            delete;
    
    private:
        LvDevice& device;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>
            bindings;
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
}