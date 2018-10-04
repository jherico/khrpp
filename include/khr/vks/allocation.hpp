#pragma once

#include <vulkan/vulkan.hpp>
#include <mutex>

#if defined(KHRPP_VULKAN_USE_VMA)
#include <vk_mem_alloc.h>
#endif

namespace vks {

// A wrapper class for an allocation, either an Image or Buffer.  Not intended to be used used directly
// but only as a base class providing common functionality for the classes below.
//
// Provides easy to use mechanisms for mapping, unmapping and copying host data to the device memory
struct Allocation {
    template <typename T = void>
    inline T* map(size_t offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) {
        return (T*)rawmap(offset, size);
    }

    inline void copy(size_t size, const void* data, VkDeviceSize offset = 0) const {
        memcpy((uint8_t*)mapped + offset, data, size);
    }

    template <typename T>
    inline void copy(const T& data, VkDeviceSize offset = 0) const {
        copy(sizeof(T), &data, offset);
    }

    template <typename T>
    inline void copy(const std::vector<T>& data, VkDeviceSize offset = 0) const {
        copy(sizeof(T) * data.size(), data.data(), offset);
    }

    vk::Device device;
    vk::DeviceSize size{ 0 };
    vk::DeviceSize alignment{ 0 };
    vk::DeviceSize allocSize{ 0 };

#if defined(KHRPP_VULKAN_USE_VMA)
    static VmaAllocator& getAllocator() {
        static VmaAllocator allocator;
        return allocator;
    }

    static void initAllocator(const vk::PhysicalDevice& physicalDevice, const vk::Device& device) {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = device;

        VmaAllocator& allocator = getAllocator();
        vmaCreateAllocator(&allocatorInfo, &allocator);
    }
    static void shutdownAllocator() {
        VmaAllocator& allocator = getAllocator();
        vmaDestroyAllocator(allocator);
        allocator = nullptr;
    }

    void flush(vk::DeviceSize size, vk::DeviceSize offset = 0) { vmaFlushAllocation(getAllocator(), allocation, offset, size); }

    void invalidate(vk::DeviceSize size, vk::DeviceSize offset) {
        vmaInvalidateAllocation(getAllocator(), allocation, offset, size);
    }

    void* rawmap(size_t offset, vk::DeviceSize size) {
        if (offset != 0 || size != VK_WHOLE_SIZE) {
            throw std::runtime_error("Unsupported");
        }

        if (!mapped) {
            vmaMapMemory(getAllocator(), allocation, &mapped);
        }

        return mapped;
    }

    void unmap() {
        if (mapped) {
            vmaUnmapMemory(getAllocator(), allocation);
            mapped = nullptr;
        }
    }

    // Note that we don't do anything here because the VMA allocates a buffer or image along with the memory...
    // so the deallocation code must also be in the buffer/image derived classes
    virtual void destroy() {}

    VmaAllocation allocation;
    /** @brief Memory propertys flags to be filled by external source at buffer creation (to query at some later point) */
    vk::MemoryPropertyFlags memoryPropertyFlags;

#else
    static void initAllocator(const vk::PhysicalDevice&, const vk::Device&) {}
    static void shutdownAllocator() {}

    void flush(vk::DeviceSize size, vk::DeviceSize offset) {
        return device.flushMappedMemoryRanges(vk::MappedMemoryRange{ memory, offset, size });
    }

    void invalidate(vk::DeviceSize size, vk::DeviceSize offset) {
        return device.invalidateMappedMemoryRanges(vk::MappedMemoryRange{ memory, offset, size });
    }

    void* rawmap(size_t offset, VkDeviceSize size) {
        mapped = device.mapMemory(memory, offset, size, vk::MemoryMapFlags());
        return mapped;
    }

    void unmap() { device.unmapMemory(memory); }

    void destroy() {
        if (mapped) {
            unmap();
        }

        if (memory) {
            device.freeMemory(memory);
            memory = nullptr;
        }
    }

    vk::DeviceMemory memory;

#endif
    void* mapped{ nullptr };
};
}  // namespace vks
