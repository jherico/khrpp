//
//  Created by Bradley Austin Davis on 2018/10/03
//  Copyright 2013-2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <khr/vks/context.hpp>

#include <gtest/gtest.h>

#include <mutex>
#include <string>
#include <iostream>

std::string toHumanSize(size_t size) {
    static const std::vector<std::string> SUFFIXES{ { "B", "KB", "MB", "GB", "TB", "PB" } };
    size_t suffixIndex = 0;
    while (suffixIndex < SUFFIXES.size() - 1 && size > 1024) {
        size >>= 10;
        ++suffixIndex;
    }

    std::stringstream buffer;
    buffer << size << " " << SUFFIXES[suffixIndex];
    return buffer.str();
}

void reportContext(const vks::Context& context) {
    std::cout << "Vulkan Context Created" << std::endl;
    std::cout << "API Version:    " << context.version.to_string() << std::endl;
    std::cout << "Driver Version: " << context.driverVersion.to_string() << std::endl;
    std::cout << "Device Name:    " << context.deviceProperties.deviceName << std::endl;
    std::cout << "Device Type:    " << vk::to_string(context.deviceProperties.deviceType) << std::endl;
    std::cout << "Memory Heaps:  " << context.deviceMemoryProperties.memoryHeapCount << std::endl;
    for (size_t i = 0; i < context.deviceMemoryProperties.memoryHeapCount; ++i) {
        const auto& heap = context.deviceMemoryProperties.memoryHeaps[i];
        std::cout << "\tHeap " << i;
        std::cout << " flags " << vk::to_string(heap.flags);
        std::cout << " size " << toHumanSize(heap.size);
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << "Memory Types:  " << context.deviceMemoryProperties.memoryTypeCount << std::endl;
    for (size_t i = 0; i < context.deviceMemoryProperties.memoryTypeCount; ++i) {
        const auto type = context.deviceMemoryProperties.memoryTypes[i];
        std::cout << "\tType " << i;
        std::cout << " flags " << vk::to_string(type.propertyFlags);
        std::cout << " heap " << type.heapIndex;
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << "Queues:" << std::endl;
    std::vector<vk::QueueFamilyProperties> queueProps = context.physicalDevice.getQueueFamilyProperties();

    for (size_t i = 0; i < queueProps.size(); ++i) {
        const auto& queueFamilyProperties = queueProps[i];
        std::cout << std::endl;
        std::cout << "Queue Family: " << i;
        std::cout << " count " << queueFamilyProperties.queueCount;;
        std::cout << " flags " << vk::to_string(queueFamilyProperties.queueFlags);
        std::cout << std::endl;
    }
}

class VkTest : public ::testing::Test {
protected:
    using Debug =  vks::Debug;
    using Output = Debug::Output;

    vks::Context& context{ vks::Context::get() };
    Debug::Output originalOutput;
    size_t errorCount{ 0 };
    size_t lastErrorCount{ 0 };

    void SetUp() override {
        // Set an output handler that will record the number of errors
        originalOutput = Debug::setOutputHandler([this](const Debug::SevFlags& sevFlags, const std::string& message) {
            if (sevFlags & Debug::SevBits::eError) {
                ++errorCount;
            }
            originalOutput(sevFlags, message);
        });
        context.setValidationEnabled(true);
    }

    void TearDown() override {
        Debug::setOutputHandler(originalOutput);
    }

    void checkErrorCountIncreased() {
        ASSERT_GT(errorCount, lastErrorCount);
        lastErrorCount = errorCount;
    }

};

TEST_F(VkTest, testContextCreation) {
    context.createInstance();
    context.createDevice();
    // reportContext(context);

    // Create and destroy a uniform buffer
    {
        uint32_t testUniformData;
        auto uniformBuffer = context.createUniformBuffer(testUniformData);
        uniformBuffer.destroy();
    }

    // FIXME do other stuff, like create an offscreen framebuffer, render an image and compare it to a reference image

    context.destroy();
    ASSERT_EQ(errorCount, 0);
}

#if defined(NDEBUG) || !defined(KHRPP_VULKAN_USE_VMA)
#define TEST_UNRELEASED_MEMORY
#endif

TEST_F(VkTest, testForcedError) {
    context.createInstance();
    context.createDevice();

    auto lastErrorCount = errorCount;
    {
        auto commandBuffer = context.allocateCommandBuffer();
        // execute a command with invalid parameters 
        // (shouldn't crash because we're not going to submit the command, just add it to the command buffer)
        // (but still... it might crash)
        commandBuffer.beginQuery(nullptr, 0, {});
        checkErrorCountIncreased();
        context.device.freeCommandBuffers(context.getCommandPool(), commandBuffer);
    }


    // This test will always fail in debug mode if the VMA allocator is in use, as the
    // allocator will assert due to unreleased memory during shutdown.
#if defined(TEST_UNRELEASED_MEMORY)
    // Create and DONT destroy a uniform buffer
    {
        uint32_t testUniformData;
        auto uniformBuffer = context.createUniformBuffer(testUniformData);
    }
#endif

    context.destroy();
    
#if defined(TEST_UNRELEASED_MEMORY)
    // Unreleased resources trigger errors on destruction of the context
    checkErrorCountIncreased();
#endif

    ASSERT_GT(errorCount, 0);
}
