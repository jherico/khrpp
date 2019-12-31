#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <string>
#include <iostream>
#include <sstream>
#include <mutex>

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

namespace vk { namespace ext {
using DebugReportObjectType = ::vk::DebugReportObjectTypeEXT;
}}  // namespace vk::ext

namespace vks {

struct Dispatchable {
protected:
    static const vk::DispatchLoaderDynamic& getDispatcher(const vk::Instance& instance = {}, const vk::Device& device = {}) {
        static vk::DispatchLoaderDynamic dispatcher;
        static vk::Instance storedInstance;
        static vk::Device storedDevice;
        static std::mutex mutex;
        if (!storedInstance || (device && !storedDevice)) {
            std::unique_lock<std::mutex> lock(mutex);
            if (!storedInstance || (device && !storedDevice)) {
                dispatcher.init(instance, &::vkGetInstanceProcAddr, device, &::vkGetDeviceProcAddr);
            }
        }
        return dispatcher;
    }
};

namespace debug {

using Flags = vk::DebugReportFlagsEXT;
using FlagBits = vk::DebugReportFlagBitsEXT;
using ObjectType = vk::DebugReportObjectTypeEXT;
using CallbackCreateInfo = vk::DebugReportCallbackCreateInfoEXT;
using Callback = vk::DebugReportCallbackEXT;

struct Message {
    Flags flags;
    ObjectType objType;
    uint64_t srcObject;
    size_t location;
    int32_t msgCode;
    const char* pLayerPrefix;
    const char* pMsg;
    void* pUserData;
};

using MessageHandler = std::function<VkBool32(const Message& message)>;

struct Messenger : public Dispatchable {
    Messenger() = delete;

    static constexpr auto DEFAULT_HANDLER = [](const Message& m) { return VK_TRUE; };

    static void setup(const vk::Instance& instance, const Flags& flags, const MessageHandler& handler = DEFAULT_HANDLER);

    static void shutdown(const vk::Instance& instance);

    static const std::list<std::string>& getDefaultLayerNames() {
#if defined(__ANDROID__)
        static std::list<std::string> LAYER_NAMES{
            "VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_LUNARG_object_tracker", "VK_LAYER_LUNARG_core_validation",
            "VK_LAYER_LUNARG_swapchain", "VK_LAYER_GOOGLE_unique_objects",
        };
#else
        static std::list<std::string> LAYER_NAMES{
            "VK_LAYER_LUNARG_assistant_layer",
            // This is a meta layer that enables all of the standard
            // validation layers in the correct order :
            // threading, parameter_validation, device_limits, object_tracker, image, core_validation, swapchain, and unique_objects
            "VK_LAYER_LUNARG_standard_validation",
        };
#endif
        return LAYER_NAMES;
    }

private:
    static Callback& getMsgCallback() {
        static Callback msgCallback;
        return msgCallback;
    }

    static VkBool32 callback(VkDebugReportFlagsEXT flags,
                             VkDebugReportObjectTypeEXT objType,
                             uint64_t srcObject,
                             size_t location,
                             int32_t msgCode,
                             const char* pLayerPrefix,
                             const char* pMsg,
                             void* pUserData);
};

struct Marker : public Dispatchable {
    static bool& active() {
        static bool activeFlag{ false };
        return activeFlag;
    }

    static void setup(const vk::Instance& instance, const vk::Device& device);

    static void insert(const vk::CommandBuffer& cmdbuffer, const std::string& markerName, const glm::vec4& color);
    static void beginRegion(const vk::CommandBuffer& cmdbuffer, const std::string& pMarkerName, const glm::vec4& color);
    static void endRegion(const vk::CommandBuffer& cmdBuffer);
    static void setObjectName(const vk::Device& device, uint64_t object, vk::DebugReportObjectTypeEXT objectType, const char* name);
    static void setObjectTag(const vk::Device& device,
                             uint64_t object,
                             vk::DebugReportObjectTypeEXT objectType,
                             uint64_t name,
                             size_t tagSize,
                             const void* tag);
    static void setCommandBufferName(const vk::Device& device, const VkCommandBuffer& cmdBuffer, const char* name);
    static void setQueueName(const vk::Device& device, const VkQueue& queue, const char* name);
    static void setImageName(const vk::Device& device, const VkImage& image, const char* name);
    static void setSamplerName(const vk::Device& device, const VkSampler& sampler, const char* name);
    static void setBufferName(const vk::Device& device, const VkBuffer& buffer, const char* name);
    static void setDeviceMemoryName(const vk::Device& device, const VkDeviceMemory& memory, const char* name);
    static void setShaderModuleName(const vk::Device& device, const VkShaderModule& shaderModule, const char* name);
    static void setPipelineName(const vk::Device& device, const VkPipeline& pipeline, const char* name);
    static void setPipelineLayoutName(const vk::Device& device, const VkPipelineLayout& pipelineLayout, const char* name);
    static void setRenderPassName(const vk::Device& device, const VkRenderPass& renderPass, const char* name);
    static void setFramebufferName(const vk::Device& device, const VkFramebuffer& framebuffer, const char* name);
    static void setDescriptorSetLayoutName(const vk::Device& device, const VkDescriptorSetLayout& descriptorSetLayout, const char* name);
    static void setDescriptorSetName(const vk::Device& device, const VkDescriptorSet& descriptorSet, const char* name);
    static void setSemaphoreName(const vk::Device& device, const VkSemaphore& semaphore, const char* name);
    static void setFenceName(const vk::Device& device, const VkFence& fence, const char* name);
    static void setEventName(const vk::Device& device, const VkEvent& _event, const char* name);
};

//class Marker {
//public:
//    Marker(const vk::CommandBuffer& cmdBuffer, const std::string& name, const glm::vec4& color = glm::vec4(0.8f))
//        : cmdBuffer(cmdBuffer) {
//        if (active) {
//            beginRegion(cmdBuffer, name, color);
//        }
//    }
//    ~Marker() {
//        if (active) {
//            endRegion(cmdBuffer);
//        }
//    }
//
//private:
//    const vk::CommandBuffer& cmdBuffer;
//};

inline VkBool32 Messenger::callback(VkDebugReportFlagsEXT flags,
                                    VkDebugReportObjectTypeEXT objType,
                                    uint64_t srcObject,
                                    size_t location,
                                    int32_t msgCode,
                                    const char* pLayerPrefix,
                                    const char* pMsg,
                                    void* pUserData) {
    std::string message;
    {
        std::stringstream buf;
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
            buf << "ERROR: ";
        } else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
            buf << "WARNING: ";
        } else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
            buf << "PERF: ";
        } else {
            return false;
        }
        buf << "[" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;
        message = buf.str();
    }

    std::cout << message << std::endl;

#ifdef __ANDROID__
    __android_log_write(ANDROID_LOG_DEBUG, LOG_TAG, message.c_str());
#endif
#ifdef _MSC_VER
    OutputDebugStringA(message.c_str());
    OutputDebugStringA("\n");
#endif
    return false;
}

inline void Messenger::setup(const vk::Instance& instance, const Flags& flags, const MessageHandler& handler) {
    getMsgCallback() = instance.createDebugReportCallbackEXT({ flags, (PFN_vkDebugReportCallbackEXT)Messenger::callback }, nullptr, getDispatcher(instance));
}

inline void Messenger::shutdown(const vk::Instance& instance) {
    instance.destroyDebugReportCallbackEXT(getMsgCallback(), nullptr, getDispatcher(instance));
    getMsgCallback() = nullptr;
}

inline void Marker::setup(const vk::Instance& instance, const vk::Device& device) {
    const auto& dispatcher = getDispatcher(instance, device);
    // Set flag if at least one function pointer is present
    active() = (dispatcher.vkDebugMarkerSetObjectNameEXT != VK_NULL_HANDLE);
}

inline void Marker::setObjectName(const vk::Device& device, uint64_t object, ObjectType objectType, const char* name) {
    const auto& markerDispatcher = getDispatcher();
    // Check for valid function pointer (may not be present if not running in a debugging application)
    if (markerDispatcher.vkDebugMarkerSetObjectNameEXT) {
        device.debugMarkerSetObjectNameEXT({ objectType, object, name }, markerDispatcher);
    }
}

inline void Marker::setObjectTag(const vk::Device& device, uint64_t object, ObjectType objectType, uint64_t name, size_t tagSize, const void* tag) {
    const auto& markerDispatcher = getDispatcher();
    // Check for valid function pointer (may not be present if not running in a debugging application)
    if (markerDispatcher.vkDebugMarkerSetObjectTagEXT) {
        device.debugMarkerSetObjectTagEXT({ objectType, object, name, tagSize, tag }, markerDispatcher);
    }
}

static inline std::array<float, 4> toFloatArray(const glm::vec4& color) {
    return { color.r, color.g, color.b, color.a };
}

inline void Marker::beginRegion(const vk::CommandBuffer& cmdbuffer, const std::string& markerName, const glm::vec4& color) {
    const auto& markerDispatcher = getDispatcher();
    // Check for valid function pointer (may not be present if not running in a debugging application)
    if (markerDispatcher.vkCmdDebugMarkerBeginEXT) {
        cmdbuffer.debugMarkerBeginEXT({ markerName.c_str(), toFloatArray(color) }, markerDispatcher);
    }
}

inline void Marker::insert(const vk::CommandBuffer& cmdbuffer, const std::string& markerName, const glm::vec4& color) {
    const auto& markerDispatcher = getDispatcher();
    // Check for valid function pointer (may not be present if not running in a debugging application)
    if (markerDispatcher.vkCmdDebugMarkerInsertEXT) {
        cmdbuffer.debugMarkerInsertEXT({ markerName.c_str(), toFloatArray(color) }, markerDispatcher);
    }
}

inline void Marker::endRegion(const vk::CommandBuffer& cmdbuffer) {
    const auto& markerDispatcher = getDispatcher();
    // Check for valid function (may not be present if not runnin in a debugging application)
    if (markerDispatcher.vkCmdDebugMarkerEndEXT) {
        cmdbuffer.debugMarkerEndEXT(markerDispatcher);
    }
}

inline void Marker::setCommandBufferName(const vk::Device& device, const VkCommandBuffer& cmdBuffer, const char* name) {
    setObjectName(device, (uint64_t)cmdBuffer, vk::DebugReportObjectTypeEXT::eCommandBuffer, name);
}

inline void Marker::setQueueName(const vk::Device& device, const VkQueue& queue, const char* name) {
    setObjectName(device, (uint64_t)queue, vk::DebugReportObjectTypeEXT::eQueue, name);
}

inline void Marker::setImageName(const vk::Device& device, const VkImage& image, const char* name) {
    setObjectName(device, (uint64_t)image, vk::DebugReportObjectTypeEXT::eImage, name);
}

inline void Marker::setSamplerName(const vk::Device& device, const VkSampler& sampler, const char* name) {
    setObjectName(device, (uint64_t)sampler, vk::DebugReportObjectTypeEXT::eSampler, name);
}

inline void Marker::setBufferName(const vk::Device& device, const VkBuffer& buffer, const char* name) {
    setObjectName(device, (uint64_t)buffer, vk::DebugReportObjectTypeEXT::eBuffer, name);
}

inline void Marker::setDeviceMemoryName(const vk::Device& device, const VkDeviceMemory& memory, const char* name) {
    setObjectName(device, (uint64_t)memory, vk::DebugReportObjectTypeEXT::eDeviceMemory, name);
}

inline void Marker::setShaderModuleName(const vk::Device& device, const VkShaderModule& shaderModule, const char* name) {
    setObjectName(device, (uint64_t)shaderModule, vk::DebugReportObjectTypeEXT::eShaderModule, name);
}

inline void Marker::setPipelineName(const vk::Device& device, const VkPipeline& pipeline, const char* name) {
    setObjectName(device, (uint64_t)pipeline, vk::DebugReportObjectTypeEXT::ePipeline, name);
}

inline void Marker::setPipelineLayoutName(const vk::Device& device, const VkPipelineLayout& pipelineLayout, const char* name) {
    setObjectName(device, (uint64_t)pipelineLayout, vk::DebugReportObjectTypeEXT::ePipelineLayout, name);
}

inline void Marker::setRenderPassName(const vk::Device& device, const VkRenderPass& renderPass, const char* name) {
    setObjectName(device, (uint64_t)renderPass, vk::DebugReportObjectTypeEXT::eRenderPass, name);
}

inline void Marker::setFramebufferName(const vk::Device& device, const VkFramebuffer& framebuffer, const char* name) {
    setObjectName(device, (uint64_t)framebuffer, vk::DebugReportObjectTypeEXT::eFramebuffer, name);
}

inline void Marker::setDescriptorSetLayoutName(const vk::Device& device, const VkDescriptorSetLayout& descriptorSetLayout, const char* name) {
    setObjectName(device, (uint64_t)descriptorSetLayout, vk::DebugReportObjectTypeEXT::eDescriptorSetLayout, name);
}

inline void Marker::setDescriptorSetName(const vk::Device& device, const VkDescriptorSet& descriptorSet, const char* name) {
    setObjectName(device, (uint64_t)descriptorSet, vk::DebugReportObjectTypeEXT::eDescriptorSet, name);
}

inline void Marker::setSemaphoreName(const vk::Device& device, const VkSemaphore& semaphore, const char* name) {
    setObjectName(device, (uint64_t)semaphore, vk::DebugReportObjectTypeEXT::eSemaphore, name);
}

inline void Marker::setFenceName(const vk::Device& device, const VkFence& fence, const char* name) {
    setObjectName(device, (uint64_t)fence, vk::DebugReportObjectTypeEXT::eFence, name);
}

inline void Marker::setEventName(const vk::Device& device, const VkEvent& _event, const char* name) {
    setObjectName(device, (uint64_t)_event, vk::DebugReportObjectTypeEXT::eEvent, name);
}

}  // namespace debug
}  // namespace vks