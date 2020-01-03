/*
* Vulkan buffer class
*
* Encapsulates a Vulkan buffer
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include "allocation.hpp"

namespace vks {
/**
    * @brief Encapsulates access to a Vulkan buffer backed up by device memory
    * @note To be filled by an external source like the VulkanDevice
    */
struct Buffer : public Allocation {
private:
    using Parent = Allocation;

public:
    vk::Buffer buffer;
    /** @brief Usage flags to be filled by external source at buffer creation (to query at some later point) */
    vk::BufferUsageFlags usageFlags;
    vk::DescriptorBufferInfo descriptor{ nullptr, 0, VK_WHOLE_SIZE };

    operator bool() const { return buffer.operator bool(); }

#ifndef USE_VMA
    /** 
        * Attach the allocated memory block to the buffer
        * 
        * @param offset (Optional) Byte offset (from the beginning) for the memory region to bind
        * 
        * @return VkResult of the bindBufferMemory call
        */
    void bind(vk::DeviceSize offset = 0) { return device.bindBufferMemory(buffer, memory, offset); }
#endif
    /**
        * Setup the default descriptor for this buffer
        *
        * @param size (Optional) Size of the memory range of the descriptor
        * @param offset (Optional) Byte offset from beginning
        *
        */
    void setupDescriptor(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0) {
        descriptor.offset = offset;
        descriptor.buffer = buffer;
        descriptor.range = size;
    }

    /** 
        * Release all Vulkan resources held by this buffer
        */
    void destroy() override {
        if (mapped) {
            unmap();
        }

        if (buffer) {
#if USE_VMA
            vmaDestroyBuffer(allocator, buffer, allocation);
            allocation = nullptr;
            allocator = nullptr;
#else
            device.destroy(buffer);
#endif
            buffer = nullptr;
        }
        buffer = nullptr;
        Parent::destroy();
    }
};
}  // namespace vks
