#pragma once

#include "lv_device.hpp"

namespace lv
{
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