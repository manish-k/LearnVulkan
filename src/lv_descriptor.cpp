#include "lv_descriptor.hpp"

#include <cassert>

namespace lv
{
	// Descriptor Layout Builder

	LvDescriptorSetLayout::Builder& 
		LvDescriptorSetLayout::Builder::addBinding(
		uint32_t bindingIndex,
		VkDescriptorType descriptorType,
		VkShaderStageFlags stageFlags,
		uint32_t count)
	{
		assert(bindings.count(bindingIndex) == 0 && 
			"binding already exists at the given index");

		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = bindingIndex;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stageFlags;
		
		bindings[bindingIndex] = layoutBinding;
		return *this;
	}

	std::unique_ptr<LvDescriptorSetLayout>
		LvDescriptorSetLayout::Builder::build() const
	{
		return std::make_unique<LvDescriptorSetLayout>(
			device,
			bindings);
	}

	// Descriptor set layout

	LvDescriptorSetLayout::LvDescriptorSetLayout(
		LvDevice& lvDevice,
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>bindings) 
		: device{lvDevice}, bindings{bindings}
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
		for (auto &kv : bindings) {
			setLayoutBindings.push_back(kv.second);
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
		descriptorSetLayoutInfo.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutInfo.bindingCount = 
			static_cast<uint32_t>(setLayoutBindings.size());
		descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

		if (vkCreateDescriptorSetLayout(
			device.getLogicalDevice(),
			&descriptorSetLayoutInfo,
			nullptr,
			&descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error(
				"failed to create descriptor set layout!");
		}
	}

	LvDescriptorSetLayout::~LvDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(
			device.getLogicalDevice(),
			descriptorSetLayout, nullptr);
	}

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

	// Descriptor Writer

	LvDescriptorWriter& LvDescriptorWriter::writeBuffer(
		uint32_t binding, 
		VkDescriptorBufferInfo* bufferInfo) {
		assert(setLayout.bindings.count(binding) == 1 && 
			"Layout does not contain specified binding");

		auto& bindingDescription = setLayout.bindings[binding];

		assert(
			bindingDescription.descriptorCount == 1 &&
			"Binding single descriptor info, but binding expects multiple");

		// TODO: do we need buffer type check?

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = bufferInfo;
		write.descriptorCount = 1;

		writes.push_back(write);
		return *this;
	}

	LvDescriptorWriter& LvDescriptorWriter::writeImage(
		uint32_t binding, VkDescriptorImageInfo* imageInfo) {
		assert(setLayout.bindings.count(binding) == 1 &&
			"Layout does not contain specified binding");

		auto& bindingDescription = setLayout.bindings[binding];

		assert(
			bindingDescription.descriptorCount == 1 &&
			"Binding single descriptor info, but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = imageInfo;
		write.descriptorCount = 1;

		writes.push_back(write);
		return *this;
	}

	bool LvDescriptorWriter::build(VkDescriptorSet& set) {
		bool success = pool.allocateDescriptorSet(
			setLayout.getDescriptorSetLayout(), 
			set);
		if (!success) {
			return false;
		}
		overwrite(set);
		return true;
	}

	void LvDescriptorWriter::overwrite(VkDescriptorSet& set) {
		for (auto& write : writes) {
			write.dstSet = set;
		}
		vkUpdateDescriptorSets(
			pool.device.getLogicalDevice(), 
			static_cast<uint32_t>(writes.size()), 
			writes.data(), 
			0, 
			nullptr);
	}
}