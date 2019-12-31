#pragma once

#include <vulkan/vulkan.hpp>

#ifdef USE_VMA
#include <vk_mem_alloc.h>
#endif

namespace vks {

// A wrapper class for an allocation, either an Image or Buffer.  Not intended to be used used directly
// but only as a base class providing common functionality for the classes below.
//
// Provides easy to use mechanisms for mapping, unmapping and copying host data to the device memory
struct Allocation {
#if USE_VMA
    VmaAllocator allocator;
    VmaAllocation allocation;
#else
    vk::DeviceMemory memory;
    vk::DeviceSize size{ 0 };
    vk::DeviceSize allocSize{ 0 };
    /** @brief Memory propertys flags to be filled by external source at buffer creation (to query at some later point) */
    vk::MemoryPropertyFlags memoryPropertyFlags;
#endif
    vk::Device device;
    vk::DeviceSize alignment{ 0 };
    void* mapped{ nullptr };

    template <typename T = void>
    inline T* map(size_t offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) {
#if USE_VMA
        vmaMapMemory(allocator, allocation, &mapped);
#else
        mapped = device.mapMemory(memory, offset, size, vk::MemoryMapFlags());
#endif
        return (T*)mapped;
    }

    inline void unmap() {
#if USE_VMA
        vmaUnmapMemory(allocator, allocation);
#else
        device.unmapMemory(memory);
#endif
        mapped = nullptr;
    }

    inline void copy(size_t size, const void* data, VkDeviceSize offset = 0) const {
        assert(mapped);
        memcpy(static_cast<uint8_t*>(mapped) + offset, data, size);
    }

    template <typename T>
    inline void copy(const T& data, VkDeviceSize offset = 0) const {
        copy(sizeof(T), &data, offset);
    }

    template <typename T>
    inline void copy(const std::vector<T>& data, VkDeviceSize offset = 0) const {
        copy(sizeof(T) * data.size(), data.data(), offset);
    }

    /**
        * Flush a memory range of the buffer to make it visible to the device
        *
        * @note Only required for non-coherent memory
        *
        * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the complete buffer range.
        * @param offset (Optional) Byte offset from beginning
        *
        * @return VkResult of the flush call
        */
    void flush(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0) {
#if USE_VMA
        vmaFlushAllocation(allocator, allocation, offset, size);
#else
        device.flushMappedMemoryRanges(vk::MappedMemoryRange{ memory, offset, size });
#endif
    }

    /**
        * Invalidate a memory range of the buffer to make it visible to the host
        *
        * @note Only required for non-coherent memory
        *
        * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate the complete buffer range.
        * @param offset (Optional) Byte offset from beginning
        *
        * @return VkResult of the invalidate call
        */
    void invalidate(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0) {
#if USE_VMA
        vmaInvalidateAllocation(allocator, allocation, offset, size);
#else
        device.invalidateMappedMemoryRanges(vk::MappedMemoryRange{ memory, offset, size });
#endif
    }

    virtual void destroy() {
        if (nullptr != mapped) {
            unmap();
        }
#if USE_VMA
        // All work is done in the derived classes
#else
        if (memory) {
            device.freeMemory(memory);
            memory = vk::DeviceMemory();
        }
#endif
    }
};
}  // namespace vks
