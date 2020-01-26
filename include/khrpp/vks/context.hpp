#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <set>
#include <string>
#include <vector>
#include <queue>

#if 0
#include <glm/glm.hpp>
#include <gli/gli.hpp>
#endif

#include <vulkan/vulkan.hpp>

#include "debug.hpp"
#include "image.hpp"
#include "buffer.hpp"
#include "helpers.hpp"

namespace vks {

using StringList = std::list<std::string>;
using CStringVector = std::vector<const char*>;

using DevicePickerFunction = std::function<vk::PhysicalDevice(const std::vector<vk::PhysicalDevice>&)>;
using DeviceFeaturesPickerFunction = std::function<void(const vk::PhysicalDevice&, vk::PhysicalDeviceFeatures2&)>;
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
            deviceQueue.queueCount = static_cast<uint32_t>(deviceQueuePriorities.size());
            deviceQueue.pQueuePriorities = deviceQueuePriorities.data();
        }

        this->queueCreateInfoCount = static_cast<uint32_t>(deviceQueues.size());
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

struct Context {
private:
    static CStringVector toCStrings(const StringList& values) {
        CStringVector result;
        result.reserve(values.size());
        for (const auto& string : values) {
            result.push_back(string.c_str());
        }
        return result;
    }

    static CStringVector toCStrings(const vk::ArrayProxy<const std::string>& values) {
        CStringVector result;
        result.reserve(values.size());
        for (const auto& string : values) {
            result.push_back(string.c_str());
        }
        return result;
    }

    static CStringVector filterLayers(const StringList& desiredLayers) {
        static std::set<std::string> validLayerNames = getAvailableLayers();
        CStringVector result;
        for (const auto& string : desiredLayers) {
            if (validLayerNames.count(string) != 0) {
                result.push_back(string.c_str());
            }
        }
        return result;
    }

public:
    // Create application wide Vulkan instance
    static std::set<std::string> getAvailableLayers() {
        std::set<std::string> result;
        auto layers = vk::enumerateInstanceLayerProperties();
        for (auto layer : layers) {
            result.insert(layer.layerName);
        }
        return result;
    }

    static std::vector<vk::ExtensionProperties> getExtensions() { return vk::enumerateInstanceExtensionProperties(); }

    static std::set<std::string> getExtensionNames() {
        std::set<std::string> extensionNames;
        for (auto& ext : getExtensions()) {
            extensionNames.insert(ext.extensionName);
        }
        return extensionNames;
    }

    static bool isExtensionPresent(const std::string& extensionName) { return getExtensionNames().count(extensionName) != 0; }

    static std::vector<vk::ExtensionProperties> getDeviceExtensions(const vk::PhysicalDevice& physicalDevice) {
        return physicalDevice.enumerateDeviceExtensionProperties();
    }

    static std::set<std::string> getDeviceExtensionNames(const vk::PhysicalDevice& physicalDevice) {
        std::set<std::string> extensionNames;
        for (auto& ext : getDeviceExtensions(physicalDevice)) {
            extensionNames.insert(ext.extensionName);
        }
        return extensionNames;
    }

    static bool isDeviceExtensionPresent(const vk::PhysicalDevice& physicalDevice, const std::string& extension) {
        return getDeviceExtensionNames(physicalDevice).count(extension) != 0;
    }

    void requireExtensions(const vk::ArrayProxy<const std::string>& requestedExtensions) {
        requiredExtensions.insert(requestedExtensions.begin(), requestedExtensions.end());
    }

    void requireDeviceExtensions(const vk::ArrayProxy<const std::string>& requestedExtensions) {
        requiredDeviceExtensions.insert(requestedExtensions.begin(), requestedExtensions.end());
    }

    void addInstanceExtensionPicker(const InstanceExtensionsPickerFunction& function) { instanceExtensionsPickers.push_back(function); }

    void setDevicePicker(const DevicePickerFunction& picker) { devicePicker = picker; }

    void setDeviceFeaturesPicker(const DeviceFeaturesPickerFunction& picker) { deviceFeaturesPicker = picker; }

    void setDeviceExtensionsPicker(const DeviceExtensionsPickerFunction& picker) { deviceExtensionsPicker = picker; }

    void setValidationEnabled(bool enable) {
        if (instance != vk::Instance()) {
            throw std::runtime_error("Cannot change validations state after instance creation");
        }
        enableValidation = enable;
    }

    void createInstance(uint32_t version = VK_MAKE_VERSION(1, 1, 0)) {
        if (enableValidation) {
            requireExtensions({ (const char*)VK_EXT_DEBUG_REPORT_EXTENSION_NAME });
        }

        // Vulkan instance
        vk::ApplicationInfo appInfo;
        appInfo.pApplicationName = "VulkanExamples";
        appInfo.pEngineName = "VulkanExamples";
        appInfo.apiVersion = version;

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
        if (!enabledExtensions.empty()) {
            instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
            instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
        }

        CStringVector layers;
        if (enableValidation) {
            layers = filterLayers(debug::Messenger::getDefaultLayerNames(getAvailableLayers()));
            instanceCreateInfo.enabledLayerCount = (uint32_t)layers.size();
            instanceCreateInfo.ppEnabledLayerNames = layers.data();
        }

        instance = vk::createInstance(instanceCreateInfo);

        if (enableValidation) {
            debug::Messenger::setup(instance, vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning);
        }

        dynamicDispatch.init(instance, &vkGetInstanceProcAddr);
    }

    void createDevice(const vk::SurfaceKHR& surface = nullptr) {
        pickDevice(surface);
        buildDevice();
        dynamicDispatch.init(instance, &vkGetInstanceProcAddr, device, &vkGetDeviceProcAddr);

        if (enableDebugMarkers) {
            debug::Marker::setup(instance, device);
        }

#ifdef USE_VMA
        {
            VmaAllocatorCreateInfo allocatorInfo = {};
            allocatorInfo.physicalDevice = physicalDevice;
            allocatorInfo.device = device;
            vmaCreateAllocator(&allocatorInfo, &allocator);
        }
#endif

        pipelineCache = device.createPipelineCache(vk::PipelineCacheCreateInfo());
        // Find a queue that supports graphics operations

        // Get the graphics queue
        queue = device.getQueue(queueFamilyIndices.graphics, 0);
    }

    void emptyTrash() const {
        if (queue) {
            queue.waitIdle();
        }
        device.waitIdle();
        for (const auto& trash : dumpster) {
            trash();
        }
        dumpster.clear();
    }

    void destroy() {
        while (!recycler.empty()) {
            recycle();
        }

        destroyCommandPool();

        device.destroyPipelineCache(pipelineCache);

#ifdef USE_VMA
        vmaDestroyAllocator(allocator);
#endif

        device.destroy();
        if (enableValidation) {
            debug::Messenger::shutdown(instance);
        }
        instance.destroy();
    }

    uint32_t findQueue(const vk::QueueFlags& desiredFlags, const vk::SurfaceKHR& presentSurface = nullptr) const {
        uint32_t bestMatch{ VK_QUEUE_FAMILY_IGNORED };
        VkQueueFlags bestMatchExtraFlags{ VK_QUEUE_FLAG_BITS_MAX_ENUM };
        size_t queueCount = queueFamilyProperties.size();
        for (uint32_t i = 0; i < queueCount; ++i) {
            auto currentFlags = queueFamilyProperties[i].queueFlags;
            // Doesn't contain the required flags, skip it
            if (!(currentFlags & desiredFlags)) {
                continue;
            }

            if (presentSurface && VK_FALSE == physicalDevice.getSurfaceSupportKHR(i, presentSurface)) {
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

    template <typename T>
    void trash(
        T value,
        const std::function<void(T t)>& destructor = [](T t) { t.destroy(); }) const {
        dumpster.push_back([=] { destructor(value); });
    }

    template <typename T>
    void trashItem(T value) const {
        throw std::runtime_error("unimplemented");
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
    void trashCommandBuffers(const vk::CommandPool& commandPool, std::vector<vk::CommandBuffer>& cmdBuffers) const {
        std::function<void(const std::vector<vk::CommandBuffer>& t)> destructor = [=](const std::vector<vk::CommandBuffer>& cmdBuffers) {
            device.freeCommandBuffers(commandPool, cmdBuffers);
        };
        trashAll(cmdBuffers, destructor);
    }

    // Should be called from time to time by the application to migrate zombie resources
    // to the recycler along with a fence that will be signalled when the objects are
    // safe to delete.
    void emptyDumpster(vk::Fence fence) {
        VoidLambdaList newDumpster;
        newDumpster.swap(dumpster);
        recycler.push(FencedLambda{ fence, [newDumpster] {
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
                device.destroy(fence);
            }
        }
    }

    // Create an image memory barrier for changing the layout of
    // an image and put it into an active command buffer
    // See chapter 11.4 "vk::Image Layout" for details

    void setImageLayout(vk::CommandBuffer cmdbuffer,
                        vk::Image image,
                        vk::ImageLayout oldImageLayout,
                        vk::ImageLayout newImageLayout,
                        vk::ImageSubresourceRange subresourceRange) const {
        // Create an image barrier object
        vk::ImageMemoryBarrier imageMemoryBarrier;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;
        imageMemoryBarrier.srcAccessMask = vks::util::accessFlagsForLayout(oldImageLayout);
        imageMemoryBarrier.dstAccessMask = vks::util::accessFlagsForLayout(newImageLayout);
        vk::PipelineStageFlags srcStageMask = vks::util::pipelineStageForLayout(oldImageLayout);
        vk::PipelineStageFlags destStageMask = vks::util::pipelineStageForLayout(newImageLayout);
        // Put barrier on top
        // Put barrier inside setup command buffer
        cmdbuffer.pipelineBarrier(srcStageMask, destStageMask, vk::DependencyFlags(), nullptr, nullptr, imageMemoryBarrier);
    }


    void addComputeToGraphicsBarrier(const vk::CommandBuffer& commandBuffer, const vk::ArrayProxy<const vk::Buffer>& buffers, vk::AccessFlags dstAccessMask = vk::AccessFlagBits::eVertexAttributeRead, vk::PipelineStageFlags dstStage = vk::PipelineStageFlagBits::eVertexInput) {
        vk::BufferMemoryBarrier bufferBarrier;
        bufferBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
        bufferBarrier.dstAccessMask = dstAccessMask;
        bufferBarrier.srcQueueFamilyIndex = queueFamilyIndices.compute;
        bufferBarrier.dstQueueFamilyIndex = queueFamilyIndices.graphics;
        bufferBarrier.size = VK_WHOLE_SIZE;
        std::vector<vk::BufferMemoryBarrier> bufferBarriers;
        for (const auto& buffer : buffers) {
            bufferBarrier.buffer = buffer;
            bufferBarriers.push_back(bufferBarrier);
        }
        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, dstStage, {}, nullptr, bufferBarriers, nullptr);
    }

    void addGraphicsToComputeBarrier(const vk::CommandBuffer& commandBuffer, const vk::ArrayProxy<const vk::Buffer>& buffers, vk::AccessFlags srcAccessMask = vk::AccessFlagBits::eVertexAttributeRead, vk::PipelineStageFlags srcStage = vk::PipelineStageFlagBits::eVertexInput) {
        vk::BufferMemoryBarrier bufferBarrier;
        bufferBarrier.srcAccessMask = srcAccessMask;
        bufferBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        bufferBarrier.srcQueueFamilyIndex = queueFamilyIndices.graphics;
        bufferBarrier.dstQueueFamilyIndex = queueFamilyIndices.compute;
        bufferBarrier.size = VK_WHOLE_SIZE;
        std::vector<vk::BufferMemoryBarrier> bufferBarriers;
        for (const auto& buffer : buffers) {
            bufferBarrier.buffer = buffer;
            bufferBarriers.push_back(bufferBarrier);
        }
        commandBuffer.pipelineBarrier(srcStage, vk::PipelineStageFlagBits::eComputeShader, {}, nullptr, bufferBarriers, nullptr);
    }


    // Fixed sub resource on first mip level and layer
    void setImageLayout(vk::CommandBuffer cmdbuffer, vk::Image image, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout) const {
        setImageLayout(cmdbuffer, image, vk::ImageAspectFlagBits::eColor, oldImageLayout, newImageLayout);
    }

    // Fixed sub resource on first mip level and layer
    void setImageLayout(vk::CommandBuffer cmdbuffer,
                        vk::Image image,
                        vk::ImageAspectFlags aspectMask,
                        vk::ImageLayout oldImageLayout,
                        vk::ImageLayout newImageLayout) const {
        vk::ImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = aspectMask;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 1;
        setImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange);
    }

    void setImageLayout(vk::Image image, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout, vk::ImageSubresourceRange subresourceRange) const {
        withPrimaryCommandBuffer([&](const auto& commandBuffer) { setImageLayout(commandBuffer, image, oldImageLayout, newImageLayout, subresourceRange); });
    }

    // Fixed sub resource on first mip level and layer
    void setImageLayout(vk::Image image, vk::ImageAspectFlags aspectMask, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout) const {
        withPrimaryCommandBuffer([&](const auto& commandBuffer) { setImageLayout(commandBuffer, image, aspectMask, oldImageLayout, newImageLayout); });
    }

protected:
    void pickDevice(const vk::SurfaceKHR& surface) {
        // Physical device
        physicalDevices = instance.enumeratePhysicalDevices();

        // Note :
        // This example will always use the first physical device reported,
        // change the vector index if you have multiple Vulkan devices installed
        // and want to use another one
        physicalDevice = devicePicker(physicalDevices);
        struct Version {
            uint32_t patch : 12;
            uint32_t minor : 10;
            uint32_t major : 10;
        } _version;
        // Store properties (including limits) and features of the phyiscal device
        // So examples can check against them and see if a feature is actually supported
        queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
        deviceProperties = physicalDevice.getProperties();
        memcpy(&_version, &deviceProperties.apiVersion, sizeof(uint32_t));
        deviceFeatures = physicalDevice.getFeatures();
        // Gather physical device memory properties
        deviceMemoryProperties = physicalDevice.getMemoryProperties();
        queueFamilyIndices.graphics = findQueue(vk::QueueFlagBits::eGraphics, surface);
        queueFamilyIndices.compute = findQueue(vk::QueueFlagBits::eCompute);
        queueFamilyIndices.transfer = findQueue(vk::QueueFlagBits::eTransfer);
    }

    void buildDevice() {
        // Vulkan device
        vks::queues::DeviceCreateInfo deviceCreateInfo;

        deviceFeaturesPicker(physicalDevice, enabledFeatures2);
        if (enabledFeatures2.pNext) {
            deviceCreateInfo.pNext = &enabledFeatures2;
        } else {
            deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
        }

        deviceCreateInfo.addQueueFamily(queueFamilyIndices.graphics, queueFamilyProperties[queueFamilyIndices.graphics].queueCount);
        if (queueFamilyIndices.compute != VK_QUEUE_FAMILY_IGNORED && queueFamilyIndices.compute != queueFamilyIndices.graphics) {
            deviceCreateInfo.addQueueFamily(queueFamilyIndices.compute, queueFamilyProperties[queueFamilyIndices.compute].queueCount);
        }
        if (queueFamilyIndices.transfer != VK_QUEUE_FAMILY_IGNORED && queueFamilyIndices.transfer != queueFamilyIndices.graphics &&
            queueFamilyIndices.transfer != queueFamilyIndices.compute) {
            deviceCreateInfo.addQueueFamily(queueFamilyIndices.transfer, queueFamilyProperties[queueFamilyIndices.transfer].queueCount);
        }
        deviceCreateInfo.update();

        std::set<std::string> allDeviceExtensions = deviceExtensionsPicker(physicalDevice);
        allDeviceExtensions.insert(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

        std::vector<const char*> enabledExtensions;
        for (const auto& extension : allDeviceExtensions) {
            enabledExtensions.push_back(extension.c_str());
        }

        // enable the debug marker extension if it is present (likely meaning a debugging tool is present)
        if (isDeviceExtensionPresent(physicalDevice, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
            enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
            enableDebugMarkers = true;
        }

        if (!enabledExtensions.empty()) {
            deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
            deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
        }
        device = physicalDevice.createDevice(deviceCreateInfo);
    }

public:
    // Vulkan instance, stores all per-application states
    vk::Instance instance;
    std::vector<vk::PhysicalDevice> physicalDevices;
    // Physical device (GPU) that Vulkan will ise
    vk::PhysicalDevice physicalDevice;

    // Queue family properties
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
    // Stores physical device properties (for e.g. checking device limits)
    vk::PhysicalDeviceProperties deviceProperties;
    // Stores phyiscal device features (for e.g. checking if a feature is available)
    vk::PhysicalDeviceFeatures deviceFeatures;

    vk::PhysicalDeviceFeatures2 enabledFeatures2;
    vk::PhysicalDeviceFeatures& enabledFeatures = enabledFeatures2.features;
    // Stores all available memory (type) properties for the physical device
    vk::PhysicalDeviceMemoryProperties deviceMemoryProperties;
    // Logical device, application's view of the physical device (GPU)
    vk::Device device;
    // vk::Pipeline cache object
    vk::PipelineCache pipelineCache;
    // Helper for accessing functionality not available in the statically linked Vulkan library
    vk::DispatchLoaderDynamic dynamicDispatch;

#ifdef USE_VMA
    VmaAllocator allocator{ nullptr };
#endif

    struct QueueIndices {
        uint32_t graphics{ VK_QUEUE_FAMILY_IGNORED };
        uint32_t transfer{ VK_QUEUE_FAMILY_IGNORED };
        uint32_t compute{ VK_QUEUE_FAMILY_IGNORED };
    } queueFamilyIndices;

    vk::Queue queue;

    vk::CommandPool getCommandPool() const {
#ifdef WIN32
        static __declspec(thread) vk::CommandPool s_cmdPool;
#else
        static thread_local vk::CommandPool s_cmdPool;
#endif
        if (!s_cmdPool) {
            vk::CommandPoolCreateInfo cmdPoolInfo;
            cmdPoolInfo.queueFamilyIndex = queueFamilyIndices.graphics;
            cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
            s_cmdPool = device.createCommandPool(cmdPoolInfo);
        }
        return s_cmdPool;
    }

    void destroyCommandPool() const {
        if (getCommandPool()) {
            device.destroyCommandPool(getCommandPool());
        }
    }

    std::vector<vk::CommandBuffer> allocateCommandBuffers(uint32_t count, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const {
        std::vector<vk::CommandBuffer> result;
        vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
        commandBufferAllocateInfo.commandPool = getCommandPool();
        commandBufferAllocateInfo.commandBufferCount = count;
        commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
        result = device.allocateCommandBuffers(commandBufferAllocateInfo);
        return result;
    }

    vk::CommandBuffer allocateCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const {
        return allocateCommandBuffers(1, level)[0];
    }

    vk::CommandBuffer createCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const {
        vk::CommandBuffer cmdBuffer;
        vk::CommandBufferAllocateInfo cmdBufAllocateInfo;
        cmdBufAllocateInfo.commandPool = getCommandPool();
        cmdBufAllocateInfo.level = level;
        cmdBufAllocateInfo.commandBufferCount = 1;
        cmdBuffer = device.allocateCommandBuffers(cmdBufAllocateInfo)[0];
        return cmdBuffer;
    }

    void flushCommandBuffer(const vk::CommandBuffer& commandBuffer, const vk::ArrayProxy<const vk::Semaphore>& signalSemaphores = nullptr) const {
        if (!commandBuffer) {
            return;
        }
        auto fence = device.createFence({});
        queue.submit(vk::SubmitInfo{ 0, nullptr, nullptr, 1, &commandBuffer, signalSemaphores.size(), signalSemaphores.data() }, fence);
        device.waitForFences(fence, VK_TRUE, UINT64_MAX);
        device.destroy(fence);
    }

    // Create a short lived command buffer which is immediately executed and released
    // This function is intended for initialization only.  It incurs a queue and device
    // flush and may impact performance if used in non-setup code
    void withPrimaryCommandBuffer(const std::function<void(const vk::CommandBuffer& commandBuffer)>& f, const vk::ArrayProxy<const vk::Semaphore>& signalSemaphores = nullptr) const {
        vk::CommandBuffer commandBuffer = allocateCommandBuffer();
        commandBuffer.begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        f(commandBuffer);
        commandBuffer.end();
        flushCommandBuffer(commandBuffer, signalSemaphores);
        device.freeCommandBuffers(getCommandPool(), commandBuffer);
    }

    Image createImageWithView(const vk::ImageCreateInfo& imageCreateInfo,
                              vk::ImageViewCreateInfo viewCreateInfo,
                              const vk::MemoryPropertyFlags& memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal) const {
        Image result = createImage(imageCreateInfo, memoryPropertyFlags);
        viewCreateInfo.image = result.image;
        result.view = device.createImageView(viewCreateInfo);
        return result;
    }

    Image createImage(const vk::ImageCreateInfo& imageCreateInfo,
                      const vk::MemoryPropertyFlags& memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal) const {
        Image result;
        result.device = device;
#if USE_VMA
        const VkImageCreateInfo& rawImageCreateInfo = imageCreateInfo;
        VmaAllocationCreateInfo allocCreateInfo = {};
        VkImage rawImage;
        VkResult createResult = vmaCreateImage(allocator, &rawImageCreateInfo, &allocCreateInfo, &rawImage, &result.allocation, nullptr);
        if (createResult != VK_SUCCESS) {
            throw std::runtime_error("Bad allocation");
        }
        result.allocator = allocator;
        result.image = rawImage;
#else
        result.device = device;
        result.image = device.createImage(imageCreateInfo);
        vk::MemoryRequirements memReqs = device.getImageMemoryRequirements(result.image);
        vk::MemoryAllocateInfo memAllocInfo;
        memAllocInfo.allocationSize = result.allocSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
        result.memory = device.allocateMemory(memAllocInfo);
        device.bindImageMemory(result.image, result.memory, 0);
#endif
        result.format = imageCreateInfo.format;
        result.extent = imageCreateInfo.extent;
        return result;
    }

    Image stageToDeviceImage(vk::ImageCreateInfo imageCreateInfo,
                             vk::DeviceSize size,
                             const void* data,
                             const std::vector<MipData>& mipData = {},
                             const vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal) const {
        Buffer staging = createStagingBuffer(size, data);
        imageCreateInfo.usage = imageCreateInfo.usage | vk::ImageUsageFlagBits::eTransferDst;
        Image result = createImage(imageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);

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
                } else {
                    bufferCopyRegion.imageExtent = imageCreateInfo.extent;
                    bufferCopyRegions.push_back(bufferCopyRegion);
                }
            }
            copyCmd.copyBufferToImage(staging.buffer, result.image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions);
            // Prepare for shader read
            setImageLayout(copyCmd, result.image, vk::ImageLayout::eTransferDstOptimal, layout, range);
        });
        staging.destroy();
        return result;
    }

    template <typename T>
    Image stageToDeviceImage(const vk::ImageCreateInfo& imageCreateInfo, const std::vector<T>& data) const {
        return stageToDeviceImage(imageCreateInfo, data.size() * sizeof(T), (void*)data.data());
    }

    Buffer createBuffer(const vk::BufferUsageFlags& usageFlags, const vk::MemoryPropertyFlags& memoryPropertyFlags, vk::DeviceSize size) const {
        Buffer result;
        result.device = device;
        vk::BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.usage = usageFlags;
        bufferCreateInfo.size = size;
#ifdef USE_VMA
        const VkBufferCreateInfo& rawBufferCreateInfo = bufferCreateInfo;
        VmaAllocationCreateInfo allocCreateInfo = {};
        if (memoryPropertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) {
            allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        } else if (memoryPropertyFlags & (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)) {
            allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        } else if (memoryPropertyFlags & vk::MemoryPropertyFlagBits::eHostVisible) {
            allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        } else {
            allocCreateInfo.usage = VMA_MEMORY_USAGE_UNKNOWN;
        }

        allocCreateInfo.requiredFlags = memoryPropertyFlags.operator VkMemoryMapFlags();

        VkBuffer rawBuffer;
        VkResult createResult = vmaCreateBuffer(allocator, &rawBufferCreateInfo, &allocCreateInfo, &rawBuffer, &result.allocation, nullptr);
        if (createResult != VK_SUCCESS) {
            throw std::runtime_error("Bad allocation");
        }
        result.allocator = allocator;
        result.buffer = rawBuffer;
#else
        result.device = device;
        result.size = size;
        result.buffer = device.createBuffer(bufferCreateInfo);
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

    Buffer createStagingBuffer(vk::DeviceSize size, const void* data = nullptr) const {
        constexpr auto stagingUsage = vk::BufferUsageFlagBits::eTransferSrc;
        constexpr auto stagingMemoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        auto result = createBuffer(stagingUsage, stagingMemoryProperties, size);
        if (data != nullptr) {
            result.map();
            result.copy(size, data);
            result.unmap();
        }
        return result;
    }

    template <typename T>
    Buffer createStagingBuffer(const std::vector<T>& data) const {
        return createStagingBuffer(data.size() * sizeof(T), (void*)data.data());
    }

    template <typename T>
    Buffer createStagingBuffer(const T& data) const {
        return createStagingBuffer(sizeof(T), &data);
    }

    Buffer createDeviceBuffer(const vk::BufferUsageFlags& usageFlags, vk::DeviceSize size) const {
        return createBuffer(usageFlags, vk::MemoryPropertyFlagBits::eDeviceLocal, size);
    }

    Buffer createSizedUniformBuffer(vk::DeviceSize size) const {
        constexpr auto uniformUsage = vk::BufferUsageFlagBits::eUniformBuffer;
        constexpr auto uniformMemoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        auto alignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
        auto extra = size % alignment;
        auto count = 1;
        auto alignedSize = size + (alignment - extra);
        auto allocatedSize = count * alignedSize;
        auto result = createBuffer(uniformUsage, uniformMemoryProperties, allocatedSize);
        result.descriptor.range = result.alignment = alignedSize;
        result.map();
        return result;
    }

    template <typename T>
    Buffer createUniformBuffer(const T& data) const {
        auto result = createSizedUniformBuffer(sizeof(T));
        result.copy(data);
        return result;
    }

    Buffer stageToDeviceBuffer(const vk::BufferUsageFlags& usage, size_t size, const void* data) const {
        Buffer staging = createStagingBuffer(size, data);
        Buffer result = createDeviceBuffer(usage | vk::BufferUsageFlagBits::eTransferDst, size);
        withPrimaryCommandBuffer([&](vk::CommandBuffer copyCmd) { copyCmd.copyBuffer(staging.buffer, result.buffer, vk::BufferCopy(0, 0, size)); });
        staging.destroy();
        return result;
    }

    template <typename T>
    Buffer stageToDeviceBuffer(const vk::BufferUsageFlags& usage, const std::vector<T>& data) const {
        return stageToDeviceBuffer(usage, sizeof(T) * data.size(), data.data());
    }

    template <typename T>
    Buffer stageToDeviceBuffer(const vk::BufferUsageFlags& usage, const T& data) const {
        return stageToDeviceBuffer(usage, sizeof(T), (void*)&data);
    }

    vk::Bool32 getMemoryType(uint32_t typeBits, const vk::MemoryPropertyFlags& properties, uint32_t* typeIndex) const {
        for (uint32_t i = 0; i < 32; i++) {
            if ((typeBits & 1) == 1) {
                if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    *typeIndex = i;
                    return VK_TRUE;
                }
            }
            typeBits >>= 1;
        }
        return VK_FALSE;
    }

    uint32_t getMemoryType(uint32_t typeBits, const vk::MemoryPropertyFlags& properties) const {
        uint32_t result = 0;
        if (VK_FALSE == getMemoryType(typeBits, properties, &result)) {
            throw std::runtime_error("Unable to find memory type " + vk::to_string(properties));
            // todo : throw error
        }
        return result;
    }

    // migrate to queue manager class?
    void submit(const vk::ArrayProxy<const vk::CommandBuffer>& commandBuffers,
                const vk::ArrayProxy<const vk::Semaphore>& wait = {},
                const vk::ArrayProxy<const vk::PipelineStageFlags>& waitStages = {},
                const vk::ArrayProxy<const vk::Semaphore>& signals = {},
                const vk::Fence& fence = vk::Fence()) const {
        vk::SubmitInfo info;
        info.commandBufferCount = commandBuffers.size();
        info.pCommandBuffers = commandBuffers.data();

        if (!signals.empty()) {
            info.signalSemaphoreCount = signals.size();
            info.pSignalSemaphores = signals.data();
        }

        assert(waitStages.size() == wait.size());

        if (!wait.empty()) {
            info.waitSemaphoreCount = wait.size();
            info.pWaitSemaphores = wait.data();
            info.pWaitDstStageMask = waitStages.data();
        }
        info.pWaitDstStageMask = waitStages.data();

        info.signalSemaphoreCount = signals.size();
        queue.submit(info, fence);
    }

    using SemaphoreStagePair = std::pair<const vk::Semaphore, const vk::PipelineStageFlags>;

    void submit(const vk::ArrayProxy<const vk::CommandBuffer>& commandBuffers,
                const vk::ArrayProxy<const SemaphoreStagePair>& wait = {},
                const vk::ArrayProxy<const vk::Semaphore>& signals = {},
                const vk::Fence& fence = vk::Fence()) const {
        std::vector<vk::Semaphore> waitSemaphores;
        std::vector<vk::PipelineStageFlags> waitStages;
        for (size_t i = 0; i < wait.size(); ++i) {
            const auto& pair = wait.data()[i];
            waitSemaphores.push_back(pair.first);
            waitStages.push_back(pair.second);
        }
        submit(commandBuffers, waitSemaphores, waitStages, signals, fence);
    }

    // Helper submit function when there is only one wait semaphore, to remove ambiguity
    void submit(const vk::ArrayProxy<const vk::CommandBuffer>& commandBuffers,
                const SemaphoreStagePair& wait,
                const vk::ArrayProxy<const vk::Semaphore>& signals = {},
                const vk::Fence& fence = vk::Fence()) const {
        submit(commandBuffers, wait.first, wait.second, signals, fence);
    }

    vk::Format getSupportedDepthFormat() const {
        // Since all depth formats may be optional, we need to find a suitable depth format to use
        // Start with the highest precision packed format
        std::vector<vk::Format> depthFormats{ {
            vk::Format::eD32SfloatS8Uint,
            vk::Format::eD32Sfloat,
            vk::Format::eD24UnormS8Uint,
            vk::Format::eD16UnormS8Uint,
            vk::Format::eD16Unorm,
        } };

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

    // Migrate to dedicated "recycler" class

    // A collection of items queued for destruction.  Once a fence has been created
    // for a queued submit, these items can be moved to the recycler for actual destruction
    // by calling the rec
    mutable VoidLambdaList dumpster;
    FencedLambdaQueue recycler;
    InstanceExtensionsPickerFunctions instanceExtensionsPickers;
    // Set to true when example is created with enabled validation layers
#ifndef NDEBUG
    bool enableValidation = true;
#else
    bool enableValidation = false;
#endif
    // Set to true when the debug marker extension is detected
    bool enableDebugMarkers = false;

private:
    std::set<std::string> requiredExtensions;
    std::set<std::string> requiredDeviceExtensions;

    DevicePickerFunction devicePicker = [](const std::vector<vk::PhysicalDevice>& devices) -> vk::PhysicalDevice { return devices[0]; };
    DeviceFeaturesPickerFunction deviceFeaturesPicker = [](const vk::PhysicalDevice& device, vk::PhysicalDeviceFeatures2& features) {};
    DeviceExtensionsPickerFunction deviceExtensionsPicker = [](const vk::PhysicalDevice& device) -> std::set<std::string> { return {}; };
};

template <>
inline void Context::trashItem<vk::CommandBuffer>(vk::CommandBuffer value) const {
    std::function<void(vk::CommandBuffer t)> destructor = [this](vk::CommandBuffer commandBuffer) {
        device.freeCommandBuffers(getCommandPool(), commandBuffer);
    };
    trash<>(value, destructor);
}

#if 0 
// Template specialization for texture objects
template <>
inline Buffer Context::createStagingBuffer(const gli::texture_cube& data) const {
    return createStagingBuffer(static_cast<vk::DeviceSize>(data.size()), data.data());
}

template <>
inline Buffer Context::createStagingBuffer(const gli::texture2d_array& data) const {
    return createStagingBuffer(static_cast<vk::DeviceSize>(data.size()), data.data());
}

template <>
inline Buffer Context::createStagingBuffer(const gli::texture2d& data) const {
    return createStagingBuffer(static_cast<vk::DeviceSize>(data.size()), data.data());
}

template <>
inline Buffer Context::createStagingBuffer(const gli::texture& data) const {
    return createStagingBuffer(static_cast<vk::DeviceSize>(data.size()), data.data());
}

template <>
inline void Context::copyToMemory(const vk::DeviceMemory& memory, const gli::texture& data, size_t offset) const {
    copyToMemory(memory, data.data(), static_cast<vk::DeviceSize>(data.size()), offset);
}
#endif

}  // namespace vks

#if 0
#if defined(__ANDROID__)
requireExtension(VK_KHR_SURFACE_EXTENSION_NAME);
requireExtension(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#else
requireExtensions(glfw::getRequiredInstanceExtensions());
#endif

requireDeviceExtensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME });



// Load a SPIR-V shader
inline vk::PipelineShaderStageCreateInfo loadShader(const std::string& fileName, vk::ShaderStageFlagBits stage) const {
    vk::PipelineShaderStageCreateInfo shaderStage;
    shaderStage.stage = stage;
    shaderStage.module = vkx::loadShader(fileName, device, stage);
    shaderStage.pName = "main"; // todo : make param
    assert(shaderStage.module);
    shaderModules.push_back(shaderStage.module);
    return shaderStage;
}

#endif
