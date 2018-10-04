#pragma once

#define VKCPP_ENHANCED_MODE
#include <vulkan/vulkan.hpp>

#include <functional>
#include <list>
#include <vector>
#include <queue>
#include <set>

#include "buffer.hpp"
#include "image.hpp"

//#include "Config.h"
//
//#include "Debug.h"
//#include "Image.h"
//#include "Helpers.h"
//#include "Device.h"

namespace vks {

using StringList = std::list<std::string>;
using CStringVector = std::vector<const char*>;
using DevicePickerFunction = std::function<vk::PhysicalDevice(const std::vector<vk::PhysicalDevice>&)>;
using DeviceExtensionsPickerFunction = std::function<std::set<std::string>(const vk::PhysicalDevice&)>;
using InstanceExtensionsPickerFunction = std::function<std::set<std::string>()>;
using InstanceExtensionsPickerFunctions = std::list<InstanceExtensionsPickerFunction>;
using LayerVector = std::vector<const char*>;
using MipData = ::std::pair<vk::Extent3D, vk::DeviceSize>;

namespace queues {

struct DeviceCreateInfo : public vk::DeviceCreateInfo {
    std::vector<vk::DeviceQueueCreateInfo> deviceQueues;
    std::vector<std::vector<float>> deviceQueuesPriorities;

    void addQueueFamily(uint32_t queueFamilyIndex, vk::ArrayProxy<float> priorities) {
        deviceQueues.push_back({ {}, queueFamilyIndex });
        std::vector<float> prioritiesVector;
        prioritiesVector.resize(priorities.size());
        memcpy(prioritiesVector.data(), priorities.data(), sizeof(float) * priorities.size());
        deviceQueuesPriorities.push_back(prioritiesVector);
    }
    void addQueueFamily(uint32_t queueFamilyIndex, size_t count = 1) {
        std::vector<float> priorities;
        priorities.resize(count);
        std::fill(priorities.begin(), priorities.end(), 0.0f);
        addQueueFamily(queueFamilyIndex, priorities);
    }

    void update() {
        assert(deviceQueuesPriorities.size() == deviceQueues.size());
        auto size = deviceQueues.size();
        for (auto i = 0; i < size; ++i) {
            auto& deviceQueue = deviceQueues[i];
            auto& deviceQueuePriorities = deviceQueuesPriorities[i];
            deviceQueue.queueCount = (uint32_t)deviceQueuePriorities.size();
            deviceQueue.pQueuePriorities = deviceQueuePriorities.data();
        }

        this->queueCreateInfoCount = (uint32_t)deviceQueues.size();
        this->pQueueCreateInfos = deviceQueues.data();
    }
};
}  // namespace queues
///////////////////////////////////////////////////////////////////////
//
// Object destruction support
//
// It's often critical to avoid destroying an object that may be in use by the GPU.  In order to service this need
// the context class contains structures for objects that are pending deletion.
//
// The first container is the dumpster, and it just contains a set of lambda objects that when executed, destroy
// resources (presumably... in theory the lambda can do anything you want, but the purpose is to contain GPU object
// destruction calls).
//
// When the application makes use of a function that uses a fence, it can provide that fence to the context as a marker
// for destroying all the pending objects.  Anything in the dumpster is migrated to the recycler.
//
// Finally, an application can call the recycle function at regular intervals (perhaps once per frame, perhaps less often)
// in order to check the fences and execute the associated destructors for any that are signalled.
using VoidLambda = std::function<void()>;
using VoidLambdaList = std::list<VoidLambda>;
using FencedLambda = std::pair<vk::Fence, VoidLambda>;
using FencedLambdaQueue = std::queue<FencedLambda>;

struct Version {
    uint32_t patch : 12;
    uint32_t minor : 10;
    uint32_t major : 10;

    Version& operator=(uint32_t v) {
        memcpy(this, &v, sizeof(v));
        return *this;
    }

    std::string to_string() const {
        std::string result;
        result = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
        return result;
    }
};

struct Context {
private:
    static CStringVector toCStrings(const StringList& values);

    static CStringVector toCStrings(const vk::ArrayProxy<const std::string>& values);

    static CStringVector filterLayers(const StringList& desiredLayers);

    Context(){};

public:
    static Context& get() {
        static Context context;
        return context;
    }

    // Create application wide Vulkan instance
    static std::set<std::string> getAvailableLayers();
    static std::vector<vk::ExtensionProperties> getExtensions();
    static std::set<std::string> getExtensionNames();
    static bool isExtensionPresent(const std::string& extensionName);
    static std::vector<vk::ExtensionProperties> getDeviceExtensions(const vk::PhysicalDevice& physicalDevice);
    static std::set<std::string> getDeviceExtensionNames(const vk::PhysicalDevice& physicalDevice);
    static bool isDeviceExtensionPresent(const vk::PhysicalDevice& physicalDevice, const std::string& extension);

    void requireExtensions(const vk::ArrayProxy<const std::string>& v) { requiredExtensions.insert(v.begin(), v.end()); }
    void requireDeviceExtensions(const vk::ArrayProxy<const std::string>& v) {
        requiredDeviceExtensions.insert(v.begin(), v.end());
    }
    void addInstanceExtensionPicker(const InstanceExtensionsPickerFunction& function) {
        instanceExtensionsPickers.push_back(function);
    }
    void setDevicePicker(const DevicePickerFunction& picker) { devicePicker = picker; }
    void setDeviceExtensionsPicker(const DeviceExtensionsPickerFunction& picker) { deviceExtensionsPicker = picker; }
    void setValidationEnabled(bool enable);
    void createInstance(const char* engineName = __FILE__, const char* applicationName = __FILE__);
    void createDevice(const vk::SurfaceKHR& presentSurface = nullptr);

    void destroy();

    uint32_t findQueue(const vk::QueueFlags& desiredFlags, const vk::SurfaceKHR& presentSurface = nullptr) const;

#if 0
    template <typename T>
    void trash(T value, std::function<void(T t)> destructor = [](T t) { t.destroy(); }) const {
        if (!value) {
            return;
        }
        dumpster.push_back([=] { destructor(value); });
    }

    template <typename T>
    void trashAll(std::vector<T>& values, std::function<void(const std::vector<T>& t)> destructor) const {
        if (values.empty()) {
            return;
        }
        dumpster.push_back([=] { destructor(values); });
        // Clear the buffer
        values.clear();
    }

    //
    // Convenience functions for trashing specific types.  These functions know what kind of function
    // call to make for destroying a given Vulkan object.
    //

    void trashPipeline(vk::Pipeline& pipeline) const {
        trash<vk::Pipeline>(pipeline, [this](vk::Pipeline pipeline) { device.destroyPipeline(pipeline); });
    }

    void trashCommandBuffers(const vk::CommandPool& commandPool, std::vector<vk::CommandBuffer>& cmdBuffers) const {
        std::function<void(const std::vector<vk::CommandBuffer>& t)> destructor =
            [=](const std::vector<vk::CommandBuffer>& cmdBuffers) { device.freeCommandBuffers(commandPool, cmdBuffers); };
        trashAll(cmdBuffers, destructor);
    }

    // Should be called from time to time by the application to migrate zombie resources
    // to the recycler along with a fence that will be signalled when the objects are
    // safe to delete.
    void emptyDumpster(vk::Fence fence) {
        VoidLambdaList newDumpster;
        newDumpster.swap(dumpster);
        recycler.push(FencedLambda{ fence, [fence, newDumpster, this] {
                                       for (const auto& f : newDumpster) {
                                           f();
                                       }
                                   } });
    }

    // Check the recycler fences for signalled status.  Any that are signalled will have their corresponding
    // lambdas executed, freeing up the associated resources
    void recycle() {
        while (!recycler.empty() && vk::Result::eSuccess == device.getFenceStatus(recycler.front().first)) {
            vk::Fence fence = recycler.front().first;
            VoidLambda lambda = recycler.front().second;
            recycler.pop();

            lambda();

            if (recycler.empty() || fence != recycler.front().first) {
                device.destroyFence(fence);
            }
        }
    }
#endif

    // Create an image memory barrier for changing the layout of
    // an image and put it into an active command buffer
    // See chapter 11.4 "vk::Image Layout" for details
    void setImageLayout(const vk::CommandBuffer& cmdbuffer,
                        const vk::Image& image,
                        const vk::ImageLayout& oldImageLayout,
                        const vk::ImageLayout& newImageLayout,
                        const vk::ImageSubresourceRange& subresourceRange) const;

    // Fixed sub resource on first mip level and layer
    void setImageLayout(const vk::CommandBuffer& cmdbuffer,
                        const vk::Image& image,
                        const vk::ImageLayout& oldImageLayout,
                        const vk::ImageLayout& newImageLayout,
                        const vk::ImageAspectFlags& aspectMask = vk::ImageAspectFlagBits::eColor) const;

    void setImageLayout(const vk::Image& image,
                        const vk::ImageLayout& oldImageLayout,
                        const vk::ImageLayout& newImageLayout,
                        const vk::ImageSubresourceRange& subresourceRange) const {
        withPrimaryCommandBuffer([&](const auto& commandBuffer) {
            setImageLayout(commandBuffer, image, oldImageLayout, newImageLayout, subresourceRange);
        });
    }

    // Fixed sub resource on first mip level and layer
    void setImageLayout(vk::Image image,
                        vk::ImageLayout oldImageLayout,
                        vk::ImageLayout newImageLayout,
                        vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor) const {
        withPrimaryCommandBuffer([&](const auto& commandBuffer) {
            setImageLayout(commandBuffer, image, oldImageLayout, newImageLayout, aspectMask);
        });
    }

private:
    void pickDevice(const vk::SurfaceKHR& device);
    void buildDevice();

public:
    const vk::CommandPool& getCommandPool() const;
    void destroyCommandPool();
    vk::CommandBuffer allocateCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const;
    std::vector<vk::CommandBuffer> allocateCommandBuffers(uint32_t c,
                                                          vk::CommandBufferLevel l = vk::CommandBufferLevel::ePrimary) const;
    void flushCommandBuffer(const vk::CommandBuffer& commandBuffer) const;

    // Create a short lived command buffer which is immediately executed and released
    // This function is intended for initialization only.  It incurs a queue and device
    // flush and may impact performance if used in non-setup code
    void withPrimaryCommandBuffer(const std::function<void(const vk::CommandBuffer& commandBuffer)>& f) const;

    Image createImage(const vk::ImageCreateInfo& imageCreateInfo, const vk::MemoryPropertyFlags& memoryPropertyFlags) const;
#if 0
    Image stageToDeviceImage(vk::ImageCreateInfo imageCreateInfo,
        const vk::MemoryPropertyFlags& memoryPropertyFlags,
        vk::DeviceSize size,
        const void* data,
        const std::vector<MipData>& mipData = {}) const;
    template <typename T>
    Image stageToDeviceImage(const vk::ImageCreateInfo& imageCreateInfo,
                             const vk::MemoryPropertyFlags& memoryPropertyFlags,
                             const std::vector<T>& data) const {
        return stageToDeviceImage(imageCreateInfo, memoryPropertyFlags, data.size() * sizeof(T), (void*)data.data());
    }

    template <typename T>
    Image stageToDeviceImage(const vk::ImageCreateInfo& imageCreateInfo, const std::vector<T>& data) const {
        return stageToDeviceImage(imageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, data.size() * sizeof(T),
                                  (void*)data.data());
    }
#endif

    Buffer createBuffer(const vk::BufferUsageFlags& usageFlags,
                        vk::DeviceSize size,
                        const vk::MemoryPropertyFlags& requiredMemoryFlags = {},
                        const vk::MemoryPropertyFlags& requestedMemoryFlags = {}) const;

    Buffer createDeviceBuffer(const vk::BufferUsageFlags& usageFlags, vk::DeviceSize size) const;

    Buffer createStagingBuffer(vk::DeviceSize size, const void* data = nullptr) const;

    template <typename T>
    Buffer createStagingBuffer(const std::vector<T>& data) const {
        return createBuffer(data.size() * sizeof(T), (void*)data.data());
    }

    template <typename T>
    Buffer createStagingBuffer(const T& data) const {
        return createStagingBuffer(sizeof(T), &data);
    }

    Buffer createSizedUniformBuffer(vk::DeviceSize size) const {
        auto alignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
        auto extra = size % alignment;
        auto count = 1;
        auto alignedSize = size + (alignment - extra);
        auto allocatedSize = count * alignedSize;
        auto result = createBuffer(vk::BufferUsageFlagBits::eUniformBuffer, allocatedSize,
                                   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                                   vk::MemoryPropertyFlagBits::eDeviceLocal);
        result.alignment = alignedSize;
        result.descriptor.range = result.alignment;
        return result;
    }

    template <typename T>
    Buffer createUniformBuffer(const T& data) const {
        auto result = createSizedUniformBuffer(sizeof(T));
        result.map();
        result.copy(data);
        return result;
    }

    Buffer stageToDeviceBuffer(const vk::BufferUsageFlags& usage, size_t size, const void* data) const;

    template <typename T>
    Buffer stageToDeviceBuffer(const vk::BufferUsageFlags& usage, const std::vector<T>& data) const {
        return stageToDeviceBuffer(usage, sizeof(T) * data.size(), data.data());
    }

    template <typename T>
    Buffer stageToDeviceBuffer(const vk::BufferUsageFlags& usage, const T& data) const {
        return stageToDeviceBuffer(usage, sizeof(T), (void*)&data);
    }

    vk::Bool32 getMemoryType(uint32_t typeBits, const vk::MemoryPropertyFlags& properties, uint32_t* typeIndex) const;

    uint32_t getMemoryType(uint32_t typeBits, const vk::MemoryPropertyFlags& properties) const;

    vk::Format getSupportedDepthFormat() const;

public:
    // Vulkan instance, stores all per-application states
    const vk::Instance instance;
    const std::vector<vk::PhysicalDevice> physicalDevices;
    // Physical device (GPU) that Vulkan will ise
    const vk::PhysicalDevice physicalDevice;
    const Version version;
    const Version driverVersion;

    // Queue family properties
    const std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
    // Stores physical device properties (for e.g. checking device limits)
    const vk::PhysicalDeviceProperties deviceProperties;
    // Stores phyiscal device features (for e.g. checking if a feature is available)
    const vk::PhysicalDeviceFeatures deviceFeatures;

    const vk::PhysicalDeviceFeatures enabledFeatures;
    // Stores all available memory (type) properties for the physical device
    const vk::PhysicalDeviceMemoryProperties deviceMemoryProperties;
    // Logical device, application's view of the physical device (GPU)
    const vk::Device device;

    struct QueueIndices {
        uint32_t graphics{ VK_QUEUE_FAMILY_IGNORED };
        uint32_t transfer{ VK_QUEUE_FAMILY_IGNORED };
        uint32_t compute{ VK_QUEUE_FAMILY_IGNORED };
    };

    const QueueIndices queueIndices;
    const vk::Queue queue;

private:
#if 0
    // A collection of items queued for destruction.  Once a fence has been created
    // for a queued submit, these items can be moved to the recycler for actual destruction
    // by calling the rec
    mutable VoidLambdaList dumpster;
    FencedLambdaQueue recycler;
#endif

    InstanceExtensionsPickerFunctions instanceExtensionsPickers;
    // Set to true when example is created with enabled validation layers
    bool enableValidation = false;
    // Set to true when the debug marker extension is detected
    bool enableDebugMarkers = false;

    std::set<std::string> requiredExtensions;
    std::set<std::string> requiredDeviceExtensions;

    DevicePickerFunction devicePicker = [](const std::vector<vk::PhysicalDevice>& devices) -> vk::PhysicalDevice {
        return devices[0];
    };

    DeviceExtensionsPickerFunction deviceExtensionsPicker = [](const vk::PhysicalDevice& device) -> std::set<std::string> {
        return {};
    };

    vk::CommandPool commandPool;
};

using ContextPtr = std::shared_ptr<Context>;
}  // namespace vks

// Implementation, add preprocessor guard

#include "debug.hpp"
#include "helpers.hpp"

namespace vks {

inline CStringVector Context::toCStrings(const StringList& values) {
    CStringVector result;
    result.reserve(values.size());
    for (const auto& string : values) {
        result.push_back(string.c_str());
    }
    return result;
}

inline CStringVector Context::toCStrings(const vk::ArrayProxy<const std::string>& values) {
    CStringVector result;
    result.reserve(values.size());
    for (const auto& string : values) {
        result.push_back(string.c_str());
    }
    return result;
}

inline CStringVector Context::filterLayers(const StringList& desiredLayers) {
    static std::set<std::string> validLayerNames = getAvailableLayers();
    CStringVector result;
    for (const auto& string : desiredLayers) {
        if (validLayerNames.count(string) != 0) {
            result.push_back(string.c_str());
        }
    }
    return result;
}

// Create application wide Vulkan instance
inline std::set<std::string> Context::getAvailableLayers() {
    std::set<std::string> result;
    auto layers = vk::enumerateInstanceLayerProperties();
    for (auto layer : layers) {
        result.insert(layer.layerName);
    }
    return result;
}

inline std::vector<vk::ExtensionProperties> Context::getExtensions() {
    return vk::enumerateInstanceExtensionProperties();
}

inline std::set<std::string> Context::getExtensionNames() {
    std::set<std::string> extensionNames;
    for (auto& ext : getExtensions()) {
        extensionNames.insert(ext.extensionName);
    }
    return extensionNames;
}

inline bool Context::isExtensionPresent(const std::string& extensionName) {
    return getExtensionNames().count(extensionName) != 0;
}

inline std::vector<vk::ExtensionProperties> Context::getDeviceExtensions(const vk::PhysicalDevice& physicalDevice) {
    return physicalDevice.enumerateDeviceExtensionProperties();
}

inline std::set<std::string> Context::getDeviceExtensionNames(const vk::PhysicalDevice& physicalDevice) {
    std::set<std::string> extensionNames;
    for (auto& ext : getDeviceExtensions(physicalDevice)) {
        extensionNames.insert(ext.extensionName);
    }
    return extensionNames;
}

inline bool Context::isDeviceExtensionPresent(const vk::PhysicalDevice& physicalDevice, const std::string& extension) {
    return getDeviceExtensionNames(physicalDevice).count(extension) != 0;
}

inline void Context::setValidationEnabled(bool enable) {
    if (instance) {
        throw std::runtime_error("Cannot change validations state after instance creation");
    }
    enableValidation = enable;
}

inline void Context::createInstance(const char* engineName, const char* applicationName) {
    if (enableValidation) {
        requireExtensions({ (const char*)VK_EXT_DEBUG_UTILS_EXTENSION_NAME });
    }

    // Vulkan instance
    vk::ApplicationInfo appInfo;
    appInfo.pApplicationName = applicationName;
    appInfo.pEngineName = engineName;
    appInfo.apiVersion = VK_API_VERSION_1_1;

    std::set<std::string> instanceExtensions;
    instanceExtensions.insert(requiredExtensions.begin(), requiredExtensions.end());
    for (const auto& picker : instanceExtensionsPickers) {
        auto extensions = picker();
        instanceExtensions.insert(extensions.begin(), extensions.end());
    }

    std::vector<const char*> enabledExtensions;
    for (const auto& extension : instanceExtensions) {
        enabledExtensions.push_back(extension.c_str());
    }

    // Enable surface extensions depending on os
    vk::InstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    if (enabledExtensions.size() > 0) {
        instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    }

    CStringVector layers;
    if (enableValidation) {
        layers = filterLayers(Debug::getDefaultValidationLayers());
        instanceCreateInfo.enabledLayerCount = (uint32_t)layers.size();
        instanceCreateInfo.ppEnabledLayerNames = layers.data();
    }

    const_cast<vk::Instance&>(instance) = vk::createInstance(instanceCreateInfo);
}

inline void Context::createDevice(const vk::SurfaceKHR& surface) {
    pickDevice(surface);
    buildDevice();

    if (enableValidation) {
        Debug::startup(instance);
    }

    vks::Allocation::initAllocator(physicalDevice, device);

    // Get the graphics queue
    const_cast<vk::Queue&>(queue) = device.getQueue(queueIndices.graphics, 0);
}

inline void Context::destroy() {
    queue.waitIdle();
    device.waitIdle();
#if 0
            for (const auto& trash : dumpster) {
                trash();
            }

            while (!recycler.empty()) {
                recycle();
            }

#endif
    destroyCommandPool();

    vks::Allocation::shutdownAllocator();

    device.destroy();

    const_cast<vk::Queue&>(queue) = nullptr;
    const_cast<vk::Device&>(device) = nullptr;
    const_cast<vk::PhysicalDevice&>(physicalDevice) = nullptr;

    if (enableValidation) {
        Debug::shutdown(instance);
    }

    instance.destroy();
    const_cast<vk::Instance&>(instance) = nullptr;
}

inline uint32_t Context::findQueue(const vk::QueueFlags& desiredFlags, const vk::SurfaceKHR& presentSurface) const {
    uint32_t bestMatch{ VK_QUEUE_FAMILY_IGNORED };
    VkQueueFlags bestMatchExtraFlags{ VK_QUEUE_FLAG_BITS_MAX_ENUM };
    size_t queueCount = queueFamilyProperties.size();
    for (uint32_t i = 0; i < queueCount; ++i) {
        auto currentFlags = queueFamilyProperties[i].queueFlags;
        // Doesn't contain the required flags, skip it
        if (!(currentFlags & desiredFlags)) {
            continue;
        }

        VkQueueFlags currentExtraFlags = (currentFlags & ~desiredFlags).operator VkQueueFlags();

        // If we find an exact match, return immediately
        if (0 == currentExtraFlags) {
            return i;
        }

        if (bestMatch == VK_QUEUE_FAMILY_IGNORED || currentExtraFlags < bestMatchExtraFlags) {
            bestMatch = i;
            bestMatchExtraFlags = currentExtraFlags;
        }
    }

    return bestMatch;
}

// Create an image memory barrier for changing the layout of
// an image and put it into an active command buffer
// See chapter 11.4 "vk::Image Layout" for details

inline void Context::setImageLayout(const vk::CommandBuffer& cmdbuffer,
                                    const vk::Image& image,
                                    const vk::ImageLayout& oldImageLayout,
                                    const vk::ImageLayout& newImageLayout,
                                    const vk::ImageSubresourceRange& subresourceRange) const {
    // Create an image barrier object
    vk::ImageMemoryBarrier imageMemoryBarrier;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.srcAccessMask = util::accessFlagsForLayout(oldImageLayout);
    imageMemoryBarrier.dstAccessMask = util::accessFlagsForLayout(newImageLayout);

    // Put barrier on top
    // Put barrier inside setup command buffer
    cmdbuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands,
                              vk::DependencyFlags(), nullptr, nullptr, imageMemoryBarrier);
}

// Fixed sub resource on first mip level and layer
inline void Context::setImageLayout(const vk::CommandBuffer& cmdbuffer,
                                    const vk::Image& image,
                                    const vk::ImageLayout& oldImageLayout,
                                    const vk::ImageLayout& newImageLayout,
                                    const vk::ImageAspectFlags& aspectMask) const {
    vk::ImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = aspectMask;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;
    setImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange);
}

inline void Context::pickDevice(const vk::SurfaceKHR& surface) {
    // Physical device
    const_cast<std::vector<vk::PhysicalDevice>&>(physicalDevices) = instance.enumeratePhysicalDevices();
    const_cast<vk::PhysicalDevice&>(physicalDevice) = devicePicker(physicalDevices);

    // Store properties (including limits) and features of the phyiscal device
    // So examples can check against them and see if a feature is actually supported
    const_cast<std::vector<vk::QueueFamilyProperties>&>(queueFamilyProperties) = physicalDevice.getQueueFamilyProperties();
    const_cast<vk::PhysicalDeviceProperties&>(deviceProperties) = physicalDevice.getProperties();
    const_cast<Version&>(version) = deviceProperties.apiVersion;
    const_cast<Version&>(driverVersion) = deviceProperties.driverVersion;
    const_cast<vk::PhysicalDeviceFeatures&>(deviceFeatures) = physicalDevice.getFeatures();
    // Gather physical device memory properties
    const_cast<vk::PhysicalDeviceMemoryProperties&>(deviceMemoryProperties) = physicalDevice.getMemoryProperties();

    auto& queueIndices = const_cast<QueueIndices&>(this->queueIndices);
    queueIndices.graphics = findQueue(vk::QueueFlagBits::eGraphics, surface);
    queueIndices.compute = findQueue(vk::QueueFlagBits::eCompute);
    queueIndices.transfer = findQueue(vk::QueueFlagBits::eTransfer);
}

inline void Context::buildDevice() {
    // Vulkan device
    vks::queues::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.addQueueFamily(queueIndices.graphics, queueFamilyProperties[queueIndices.graphics].queueCount);
    if (queueIndices.compute != VK_QUEUE_FAMILY_IGNORED && queueIndices.compute != queueIndices.graphics) {
        deviceCreateInfo.addQueueFamily(queueIndices.compute, queueFamilyProperties[queueIndices.compute].queueCount);
    }
    if (queueIndices.transfer != VK_QUEUE_FAMILY_IGNORED && queueIndices.transfer != queueIndices.graphics &&
        queueIndices.transfer != queueIndices.compute) {
        deviceCreateInfo.addQueueFamily(queueIndices.transfer, queueFamilyProperties[queueIndices.transfer].queueCount);
    }
    deviceCreateInfo.update();

    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    std::set<std::string> targetDeviceExtensions = deviceExtensionsPicker(physicalDevice);
    targetDeviceExtensions.insert(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

    std::vector<const char*> enabledExtensions;
    for (const auto& extension : targetDeviceExtensions) {
        enabledExtensions.push_back(extension.c_str());
    }

#if 0
    // enable the debug marker extension if it is present (likely meaning a debugging tool is present)
    if (vks::checkDeviceExtensionPresent(physicalDevice, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
        enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        enableDebugMarkers = true;
    }
#endif

    if (enabledExtensions.size() > 0) {
        deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    }
    const_cast<vk::Device&>(device) = physicalDevice.createDevice(deviceCreateInfo);
}

inline const vk::CommandPool& Context::getCommandPool() const {
    if (!commandPool) {
        vk::CommandPoolCreateInfo cmdPoolInfo;
        cmdPoolInfo.queueFamilyIndex = queueIndices.graphics;
        cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        auto pool = device.createCommandPool(cmdPoolInfo);
        const_cast<vk::CommandPool&>(commandPool) = pool;
    }
    return commandPool;
}

inline void Context::destroyCommandPool() {
    if (commandPool) {
        device.destroy(commandPool);
        commandPool = nullptr;
    }
}

inline std::vector<vk::CommandBuffer> Context::allocateCommandBuffers(uint32_t count, vk::CommandBufferLevel level) const {
    std::vector<vk::CommandBuffer> result;
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.commandPool = getCommandPool();
    commandBufferAllocateInfo.commandBufferCount = count;
    commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    result = device.allocateCommandBuffers(commandBufferAllocateInfo);
    return result;
}

inline vk::CommandBuffer Context::allocateCommandBuffer(vk::CommandBufferLevel level) const {
    return allocateCommandBuffers(1, level)[0];
}

inline void Context::flushCommandBuffer(const vk::CommandBuffer& commandBuffer) const {
    if (!commandBuffer) {
        return;
    }

    auto fence = device.createFence(vk::FenceCreateInfo{});
    queue.submit(vk::SubmitInfo{ 0, nullptr, nullptr, 1, &commandBuffer }, fence);
    device.waitForFences(fence, VK_TRUE, (~0ULL));
}

// Create a short lived command buffer which is immediately executed and released
// This function is intended for initialization only.  It incurs a queue and device
// flush and may impact performance if used in non-setup code
inline void Context::withPrimaryCommandBuffer(const std::function<void(const vk::CommandBuffer& commandBuffer)>& f) const {
    auto commandBuffer = allocateCommandBuffer();
    commandBuffer.begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    f(commandBuffer);
    commandBuffer.end();
    flushCommandBuffer(commandBuffer);
    device.freeCommandBuffers(getCommandPool(), commandBuffer);
}

inline Image Context::createImage(const vk::ImageCreateInfo& imageCreateInfo,
                                  const vk::MemoryPropertyFlags& memoryPropertyFlags) const {
    Image result;
    result.device = device;
    result.format = imageCreateInfo.format;
    result.extent = imageCreateInfo.extent;

#if defined(KHRPP_VULKAN_USE_VMA)
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.requiredFlags = memoryPropertyFlags.operator unsigned int();
    auto pCreateInfo = &(imageCreateInfo.operator const VkImageCreateInfo&());
    auto pImage = &reinterpret_cast<VkImage&>(result.image);
    vmaCreateImage(Allocation::getAllocator(), pCreateInfo, &allocInfo, pImage, &result.allocation, nullptr);
#else
    result.image = device.createImage(imageCreateInfo);
    if ((memoryPropertyFlags & vk::MemoryPropertyFlagBits::eLazilyAllocated) != vk::MemoryPropertyFlagBits::eLazilyAllocated) {
        vk::MemoryRequirements memReqs = device.getImageMemoryRequirements(result.image);
        vk::MemoryAllocateInfo memAllocInfo;
        memAllocInfo.allocationSize = result.allocSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
        result.memory = device.allocateMemory(memAllocInfo);
        device.bindImageMemory(result.image, result.memory, 0);
    }
#endif
    return result;
}
#if 0
        Image stageToDeviceImage(vk::ImageCreateInfo imageCreateInfo,
            const vk::MemoryPropertyFlags& memoryPropertyFlags,
            vk::DeviceSize size,
            const void* data,
            const std::vector<MipData>& mipData = {}) const {
            Buffer staging = createStagingBuffer(size, data);
            imageCreateInfo.usage = imageCreateInfo.usage | vk::ImageUsageFlagBits::eTransferDst;
            Image result = createImage(imageCreateInfo, memoryPropertyFlags);

            withPrimaryCommandBuffer([&](const vk::CommandBuffer& copyCmd) {
                vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, imageCreateInfo.mipLevels, 0, 1);
                // Prepare for transfer
                setImageLayout(copyCmd, result.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, range);

                // Prepare for transfer
                std::vector<vk::BufferImageCopy> bufferCopyRegions;
                {
                    vk::BufferImageCopy bufferCopyRegion;
                    bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                    bufferCopyRegion.imageSubresource.layerCount = 1;
                    if (!mipData.empty()) {
                        for (uint32_t i = 0; i < imageCreateInfo.mipLevels; i++) {
                            bufferCopyRegion.imageSubresource.mipLevel = i;
                            bufferCopyRegion.imageExtent = mipData[i].first;
                            bufferCopyRegions.push_back(bufferCopyRegion);
                            bufferCopyRegion.bufferOffset += mipData[i].second;
                        }
                    }
                    else {
                        bufferCopyRegion.imageExtent = imageCreateInfo.extent;
                        bufferCopyRegions.push_back(bufferCopyRegion);
                    }
                }
                copyCmd.copyBufferToImage(staging.buffer, result.image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions);
                // Prepare for shader read
                setImageLayout(copyCmd, result.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                    range);
            });
            staging.destroy();
            return result;
        }

        template <typename T>
        Image stageToDeviceImage(const vk::ImageCreateInfo& imageCreateInfo,
            const vk::MemoryPropertyFlags& memoryPropertyFlags,
            const std::vector<T>& data) const {
            return stageToDeviceImage(imageCreateInfo, memoryPropertyFlags, data.size() * sizeof(T), (void*)data.data());
        }

        template <typename T>
        Image stageToDeviceImage(const vk::ImageCreateInfo& imageCreateInfo, const std::vector<T>& data) const {
            return stageToDeviceImage(imageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, data.size() * sizeof(T),
                (void*)data.data());
        }
#endif

inline Buffer Context::createBuffer(const vk::BufferUsageFlags& usageFlags,
                                    vk::DeviceSize size,
                                    const vk::MemoryPropertyFlags& requiredMemoryFlags,
                                    const vk::MemoryPropertyFlags& requestedMemoryFlags) const {
    Buffer result;
    result.device = device;
    result.size = size;
    result.descriptor.range = size;
    result.descriptor.offset = 0;

    vk::BufferCreateInfo createInfo{ {}, size, usageFlags };

#if defined(KHRPP_VULKAN_USE_VMA)
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.requiredFlags = requiredMemoryFlags.operator unsigned int();
    allocInfo.preferredFlags = requestedMemoryFlags.operator unsigned int();
    auto pCreateInfo = &createInfo.operator const VkBufferCreateInfo&();
    auto pBuffer = &reinterpret_cast<VkBuffer&>(result.buffer);
    vmaCreateBuffer(Allocation::getAllocator(), pCreateInfo, &allocInfo, pBuffer, &result.allocation, nullptr);
#else
    result.descriptor.buffer = result.buffer = device.createBuffer(bufferCreateInfo);
    vk::MemoryRequirements memReqs = device.getBufferMemoryRequirements(result.buffer);
    vk::MemoryAllocateInfo memAlloc;
    result.allocSize = memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
    result.memory = device.allocateMemory(memAlloc);
    device.bindBufferMemory(result.buffer, result.memory, 0);
#endif
    result.descriptor.buffer = result.buffer;
    return result;
}

inline Buffer Context::createDeviceBuffer(const vk::BufferUsageFlags& usageFlags, vk::DeviceSize size) const {
    static const vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    return createBuffer(usageFlags, size, memoryProperties);
}

inline Buffer Context::createStagingBuffer(vk::DeviceSize size, const void* data) const {
    auto result = createBuffer(vk::BufferUsageFlagBits::eTransferSrc, size,
                               vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    if (data != nullptr) {
        result.map();
        result.copy(size, data);
        result.unmap();
    }
    return result;
}

inline Buffer Context::stageToDeviceBuffer(const vk::BufferUsageFlags& usage, size_t size, const void* data) const {
    Buffer staging = createStagingBuffer(size, data);
    Buffer result = createDeviceBuffer(usage | vk::BufferUsageFlagBits::eTransferDst, size);
    withPrimaryCommandBuffer(
        [&](vk::CommandBuffer copyCmd) { copyCmd.copyBuffer(staging.buffer, result.buffer, vk::BufferCopy(0, 0, size)); });
    staging.destroy();
    return result;
}

inline vk::Bool32 Context::getMemoryType(uint32_t typeBits,
                                         const vk::MemoryPropertyFlags& properties,
                                         uint32_t* typeIndex) const {
    for (uint32_t i = 0; i < 32; i++) {
        if ((typeBits & 1) == 1) {
            if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    return false;
}

inline uint32_t Context::getMemoryType(uint32_t typeBits, const vk::MemoryPropertyFlags& properties) const {
    uint32_t result = 0;
    if (!getMemoryType(typeBits, properties, &result)) {
        // todo : throw error
    }
    return result;
}

inline vk::Format Context::getSupportedDepthFormat() const {
    // Since all depth formats may be optional, we need to find a suitable depth format to use
    // Start with the highest precision packed format
    std::vector<vk::Format> depthFormats = { vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint,
                                             vk::Format::eD16UnormS8Uint, vk::Format::eD16Unorm };

    for (auto& format : depthFormats) {
        vk::FormatProperties formatProps;
        formatProps = physicalDevice.getFormatProperties(format);
        // vk::Format must support depth stencil attachment for optimal tiling
        if (formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
            return format;
        }
    }

    throw std::runtime_error("No supported depth format");
}

}  // namespace vks
