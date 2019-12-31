#pragma once

#include <string>
#include <vulkan/vulkan.hpp>
#include <khrpp/storage.hpp>

namespace vks { namespace shaders {

inline vk::ShaderModule loadShaderModule(const vk::Device& device, const std::string& filename) {
    vk::ShaderModule result;
    khrpp::utils::FileStorage::withBinaryFileContents<uint32_t>(filename, [&](const char* filename, size_t size, const uint32_t* data) {
        result = device.createShaderModule({ {}, size, data });
    });
    return result;
}

// Load a SPIR-V shader
inline vk::PipelineShaderStageCreateInfo loadShader(const vk::Device& device,
                                             const std::string& fileName,
                                             vk::ShaderStageFlagBits stage,
                                             const char* entryPoint = "main") {
    vk::PipelineShaderStageCreateInfo shaderStage;
    shaderStage.stage = stage;
    shaderStage.module = loadShaderModule(device, fileName);
    shaderStage.pName = entryPoint;
    return shaderStage;
}

}}  // namespace vks::shaders
