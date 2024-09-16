#include "lv_descriptor.hpp"

namespace lv
{
	// Descriptor pool builder

	LvDescriptorPool::Builder& LvDescriptorPool::Builder::addPoolSize(
		VkDescriptorType descriptorType, 
		uint32_t count)
	{
		poolSizes.push_back({descriptorType, count});
		return *this;
	}

	LvDescriptorPool::Builder& LvDescriptorPool::Builder::setPoolFlags(
		VkDescriptorPoolCreateFlags flags)
	{
		poolFlags = flags;
		return *this;
	}

	LvDescriptorPool::Builder& LvDescriptorPool::Builder::setMaxSets(
		uint32_t count)
	{
		maxSets = count;
		return *this;
	}

	std::unique_ptr<LvDescriptorPool> 
		LvDescriptorPool::Builder::build() const
	{
		return std::make_unique<LvDescriptorPool>(
			device,
			maxSets,
			poolFlags,
			poolSizes);
	}

	//Descriptor pool

	LvDescriptorPool::LvDescriptorPool(
		LvDevice& lvDevice,
		uint32_t maxSets,
		VkDescriptorPoolCreateFlags poolFlags,
		const std::vector<VkDescriptorPoolSize>& poolSizes)
		: device{ lvDevice }
	{
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;
		descriptorPoolInfo.flags = poolFlags;

		if (vkCreateDescriptorPool(
			device.getLogicalDevice(), 
			&descriptorPoolInfo, 
			nullptr, 
			&descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error(
				"failed to create descriptor pool!");
		}
	}

	LvDescriptorPool::~LvDescriptorPool() {
		vkDestroyDescriptorPool(
			device.getLogicalDevice(), descriptorPool, nullptr);
	}

	bool LvDescriptorPool::allocateDescriptorSet(
		const VkDescriptorSetLayout descriptorSetLayout,
		VkDescriptorSet& descriptorSet) const
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;

		if (vkAllocateDescriptorSets(
			device.getLogicalDevice(), 
			&allocInfo, 
			&descriptorSet) != VK_SUCCESS) {
			return false;
		}
		return true;
	}

	// Free one or more descriptor sets
	void LvDescriptorPool::freeDescriptorSets(
		std::vector<VkDescriptorSet>& descriptorSets) const
	{
		vkFreeDescriptorSets(
			device.getLogicalDevice(),
			descriptorPool,
			static_cast<uint32_t>(descriptorSets.size()),
			descriptorSets.data());
	}

	// return all descriptor sets allocated from a given pool
	// to the pool, rather than freeing individual descriptor sets
	void LvDescriptorPool::resetPool()
	{
		vkResetDescriptorPool(
			device.getLogicalDevice(), descriptorPool, 0);
	}
}