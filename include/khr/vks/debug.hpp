#pragma once

#include <vulkan/vulkan.hpp>

#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <sstream>

namespace vks {

class Debug {
public:
    using StringList = std::list<std::string>;
    using SevBits = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using TypeBits = vk::DebugUtilsMessageTypeFlagBitsEXT;
    using SevFlags = vk::DebugUtilsMessageSeverityFlagsEXT;
    using TypeFlags = vk::DebugUtilsMessageTypeFlagsEXT;
    using CallbackData = vk::DebugUtilsMessengerCallbackDataEXT;
    using Output = std::function<void(const SevFlags&, const std::string&)>;
    using MessageFormatter = std::function<std::string(const SevFlags&, const TypeFlags&, const CallbackData*, void*)>;

    static const StringList& getDefaultValidationLayers() {
        static const StringList validationLayerNames {
#if defined(__ANDROID__)
            "VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_LUNARG_object_tracker",
                "VK_LAYER_LUNARG_core_validation", "VK_LAYER_LUNARG_swapchain", "VK_LAYER_GOOGLE_unique_objects",
#else
            "VK_LAYER_LUNARG_standard_validation",
#endif
        };
        return validationLayerNames;
    }

    static Output setOutputHandler(const Output& function) {
        Output result = function;
        std::swap(result, currentOutput());
        return result;
    }

    static void setMessageFormatter(const MessageFormatter& function) { currentMessageFormatter() = function; }

    static void startup(const vk::Instance& instance,
                        const SevFlags& severityFlags = SevBits::eError | SevBits::eWarning,
                        const TypeFlags& typeFlags = TypeBits::eGeneral | TypeBits::eValidation | TypeBits::ePerformance,
                        void* userData = nullptr) {
        vk::DebugUtilsMessengerCreateInfoEXT createInfo{ {}, severityFlags, typeFlags, debugCallback, userData };
        currentMessenger() = instance.createDebugUtilsMessengerEXT(createInfo, nullptr, getInstanceDispatcher(instance));
    }

    // Clear debug callback
    static void shutdown(const vk::Instance& instance) {
        instance.destroyDebugUtilsMessengerEXT(currentMessenger(), nullptr, getInstanceDispatcher(instance));
    }

private:
    static VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                  void* pUserData) {
        SevFlags sevFlags;
        reinterpret_cast<VkDebugUtilsMessageSeverityFlagBitsEXT&>(sevFlags) = messageSeverity;
        TypeFlags typeFlags{ messageType };
        auto callbackData = reinterpret_cast<const CallbackData*>(pCallbackData);
        auto message = currentMessageFormatter()(sevFlags, typeFlags, callbackData, pUserData);
        currentOutput()(sevFlags, message);
        return VK_TRUE;
    }
    static Output& currentOutput() {
        static Output output = [](const SevFlags& sevFlags, const std::string& message) {
#ifdef _MSC_VER
        //OutputDebugStringA(message.c_str());
        //OutputDebugStringA("\n");
#endif
            std::stringstream buf;
            if (sevFlags & SevBits::eError) {
                std::cout << "ERROR: ";
            } else if (sevFlags & SevBits::eWarning) {
                std::cout << "WARNING: ";
            } else if (sevFlags & SevBits::eInfo) {
                std::cout << "INFO: ";
            } else if (sevFlags & SevBits::eVerbose) {
                std::cout << "VERBOSE: ";
            } else {
                std::cout << "Unknown sev: ";
            }

            std::cout << message << std::endl;
        };
        return output;
    }

    static MessageFormatter& currentMessageFormatter() {
        static MessageFormatter formatter = [](const SevFlags& sevFlags, const TypeFlags& typeFlags,
                                               const CallbackData* callbackData, void*) -> std::string {
            // FIXME improve on this
            return std::string(callbackData->pMessage);
        };
        return formatter;
    }

    static vk::DebugUtilsMessengerEXT& currentMessenger() {
        static vk::DebugUtilsMessengerEXT messenger{};
        return messenger;
    }

    static const vk::DispatchLoaderDynamic& getInstanceDispatcher(const vk::Instance& instance = nullptr) {
        static vk::DispatchLoaderDynamic dispatcher;
        static std::once_flag once;
        if (instance) {
            std::call_once(once, [&] { dispatcher.init(instance); });
        }
        return dispatcher;
    }

    friend class DebugMarker;
};

// Note that the extension will only be present if run from an offline debugging application
// The actual check for extension presence and enabling it on the device is done in the example base class
// See ExampleBase::createInstance and ExampleBase::createDevice (base/vkx::ExampleBase.cpp)
class DebugMarker {
public:
    // Get function pointers for the debug report extensions from the device
    static void setup(const vk::Instance& instance) {
        const auto& dispatcher = Debug::getInstanceDispatcher(instance);
        active() = (nullptr != dispatcher.vkSetDebugUtilsObjectTagEXT);
    }

    // Start a new debug marker region
    static void beginRegion(const vk::CommandBuffer& cmdbuffer, const std::string& name, const std::array<float, 4>& color) {
        // Check for valid function pointer (may not be present if not running in a debugging application)
        if (isActive()) {
            cmdbuffer.beginDebugUtilsLabelEXT(vk::DebugUtilsLabelEXT{ name.c_str(), color }, Debug::getInstanceDispatcher());
        }
    }

    // Insert a new debug marker into the command buffer
    static void insert(const vk::CommandBuffer& cmdbuffer, const std::string& name, const std::array<float, 4>& color) {
        // Check for valid function pointer (may not be present if not running in a debugging application)
        if (isActive()) {
            cmdbuffer.insertDebugUtilsLabelEXT(vk::DebugUtilsLabelEXT{ name.c_str(), color }, Debug::getInstanceDispatcher());
        }
    }

    // End the current debug marker region
    static void endRegion(const vk::CommandBuffer& cmdbuffer) {
        // Check for valid function (may not be present if not running in a debugging application)
        if (isActive()) {
            cmdbuffer.endDebugUtilsLabelEXT(Debug::getInstanceDispatcher());
        }
    }

    // Sets the debug name of an object
    // All Objects in Vulkan are represented by their 64-bit handles which are passed into this function
    // along with the object type
    static void setObjectName(const vk::Device& device, uint64_t object, vk::ObjectType objectType, const std::string& name) {
        if (isActive()) {
            const auto& dispatcher = Debug::getInstanceDispatcher();
            device.setDebugUtilsObjectNameEXT({ objectType, object, name.c_str() }, dispatcher);
        }
    }

    // Object specific naming functions
    static void setName(const vk::Device& device, const vk::CommandBuffer& obj, const std::string& name) {
        setObjectName(device, (uint64_t)obj.operator VkCommandBuffer(), vk::ObjectType::eCommandBuffer, name);
    }

    static void setName(const vk::Device& device, const vk::Queue& obj, const std::string& name) {
        setObjectName(device, (uint64_t)obj.operator VkQueue(), vk::ObjectType::eQueue, name);
    }

    static void setName(const vk::Device& device, const vk::Image& obj, const std::string& name) {
        setObjectName(device, (uint64_t)obj.operator VkImage(), vk::ObjectType::eImage, name);
    }

    static void setName(const vk::Device& device, const vk::Buffer& obj, const std::string& name) {
        setObjectName(device, (uint64_t)obj.operator VkBuffer(), vk::ObjectType::eBuffer, name);
    }

    static void setName(const vk::Device& device, const vk::Framebuffer& obj, const std::string& name) {
        setObjectName(device, (uint64_t)obj.operator VkFramebuffer(), vk::ObjectType::eFramebuffer, name);
    }

    static void setName(const vk::Device& device, const vk::Pipeline& obj, const std::string& name) {
        setObjectName(device, (uint64_t)obj.operator VkPipeline(), vk::ObjectType::ePipeline, name);
    }
    static bool isActive() { return active(); }

private:
    static bool& active() {
        static bool active{ false };
        return active;
    }
};

}  // namespace vks
