/*
* Assorted commonly used Vulkan helper functions
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

//#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

namespace vks { namespace util {

inline vk::ColorComponentFlags fullColorWriteMask() {
    return vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
}

inline vk::Viewport viewport(float width, float height, float minDepth = 0, float maxDepth = 1) {
    return vk::Viewport{ 0, 0, width, height, minDepth, maxDepth };
}

inline vk::Viewport viewport(const vk::Extent2D& size, float minDepth = 0, float maxDepth = 1) {
    return viewport(static_cast<float>(size.width), static_cast<float>(size.height), minDepth, maxDepth);
}

inline vk::Rect2D rect2D(uint32_t width, uint32_t height, int32_t offsetX = 0, int32_t offsetY = 0) {
    return vk::Rect2D{ vk::Offset2D{ offsetX, offsetY }, vk::Extent2D{ width, height } };
}

inline vk::Rect2D rect2D(const vk::Extent2D& size, const vk::Offset2D& offset = {}) {
    return rect2D(size.width, size.height, offset.x, offset.y);
}

inline vk::AccessFlags accessFlagsForLayout(vk::ImageLayout layout) {
    switch (layout) {
        case vk::ImageLayout::ePreinitialized:
            return vk::AccessFlagBits::eHostWrite;
        case vk::ImageLayout::eTransferDstOptimal:
            return vk::AccessFlagBits::eTransferWrite;
        case vk::ImageLayout::eTransferSrcOptimal:
            return vk::AccessFlagBits::eTransferRead;
        case vk::ImageLayout::eColorAttachmentOptimal:
            return vk::AccessFlagBits::eColorAttachmentWrite;
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
            return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        case vk::ImageLayout::eShaderReadOnlyOptimal:
            return vk::AccessFlagBits::eShaderRead;
        default:
            return vk::AccessFlags();
    }
}

inline vk::PipelineStageFlags pipelineStageForLayout(vk::ImageLayout layout) {
    switch (layout) {
        case vk::ImageLayout::eTransferDstOptimal:
        case vk::ImageLayout::eTransferSrcOptimal:
            return vk::PipelineStageFlagBits::eTransfer;

        case vk::ImageLayout::eColorAttachmentOptimal:
            return vk::PipelineStageFlagBits::eColorAttachmentOutput;

        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
            return vk::PipelineStageFlagBits::eEarlyFragmentTests;

        case vk::ImageLayout::eShaderReadOnlyOptimal:
            return vk::PipelineStageFlagBits::eFragmentShader;

        case vk::ImageLayout::ePreinitialized:
            return vk::PipelineStageFlagBits::eHost;

        case vk::ImageLayout::eUndefined:
            return vk::PipelineStageFlagBits::eTopOfPipe;

        default:
            return vk::PipelineStageFlagBits::eBottomOfPipe;
    }
}

inline vk::ClearColorValue clearColor(const glm::vec4& v = glm::vec4(0)) {
    vk::ClearColorValue result;
    memcpy(&result.float32, &v, sizeof(result.float32));
    return result;
}

}}  // namespace vks::util
