#include "lv_buffer.hpp"

#include <cassert>

namespace lv
{
	LvBuffer::LvBuffer(
		LvDevice& device,
		VkDeviceSize instanceSize,
		uint32_t instanceCount,
		VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags memoryPropertyFlags,
		VkDeviceSize minOffsetAlignment)
		: device{ device },
		instanceSize{ instanceSize },
		instanceCount{ instanceCount },
		usageFlags{ usageFlags },
		memoryPropertyFlags{ memoryPropertyFlags }
	{
		alignmentSize = getAlignment(
			instanceSize, minOffsetAlignment);
		bufferSize = alignmentSize * instanceCount;
		device.createBuffer(
			bufferSize,
			usageFlags,
			memoryPropertyFlags,
			buffer,
			bufferMemory);
	}

	LvBuffer::~LvBuffer()
	{
		auto lDevice = device.getLogicalDevice();
		unmap();
		vkDestroyBuffer(lDevice, buffer, nullptr);
		vkFreeMemory(lDevice, bufferMemory, nullptr);
	}

	VkDeviceSize LvBuffer::getAlignment(
		VkDeviceSize instanceSize,
		VkDeviceSize minOffsetAlignment)
	{
		// formula from here: 
		// https://en.wikipedia.org/wiki/Data_structure_alignment
		if (minOffsetAlignment > 0) {
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
		}
		return instanceSize;
	}

	VkResult LvBuffer::map(VkDeviceSize size, VkDeviceSize offset)
	{
		assert(buffer && bufferMemory && "map not allowed before buffer creation");

		if (size == VK_WHOLE_SIZE)
		{
			return vkMapMemory(
				device.getLogicalDevice(),
				bufferMemory,
				0,
				size,
				0,
				&mapped
			);
		}
		return vkMapMemory(
			device.getLogicalDevice(),
			bufferMemory,
			offset,
			bufferSize,
			0,
			&mapped
		);

	}

	void LvBuffer::unmap()
	{
		if (mapped)
		{
			vkUnmapMemory(
				device.getLogicalDevice(), bufferMemory);
			mapped = nullptr;
		}
	}

	void LvBuffer::writeToBuffer(
		void* data, 
		VkDeviceSize size, 
		VkDeviceSize offset)
	{
		assert(mapped && "can't write to unmapped buffer");

		if (size == VK_WHOLE_SIZE) 
			memcpy(mapped, data, bufferSize);
		else
			memcpy((char*)mapped + offset, data, size);
	}

	VkResult LvBuffer::flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = bufferMemory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(
			device.getLogicalDevice(), 1, &mappedRange);
	}

	VkDescriptorBufferInfo LvBuffer::descriptorInfo(
		VkDeviceSize size, 
		VkDeviceSize offset)
	{
		return VkDescriptorBufferInfo{
			buffer,
			offset,
			size,
		};
	}

	VkResult LvBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = bufferMemory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(
			device.getLogicalDevice(), 1, &mappedRange);
	}

	void LvBuffer::writeToIndex(void* data, int index)
	{
		writeToBuffer(data, alignmentSize, index * alignmentSize);
	}

	VkResult LvBuffer::flushIndex(int index)
	{
		return flush(alignmentSize, index * alignmentSize);
	}

	VkDescriptorBufferInfo LvBuffer::descriptorInfoForIndex(int index)
	{
		return descriptorInfo(alignmentSize, index * alignmentSize);
	}

	VkResult LvBuffer::invalidateIndex(int index)
	{
		return invalidate(alignmentSize, index * alignmentSize);
	}
}