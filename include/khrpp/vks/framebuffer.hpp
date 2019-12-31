//
//  Created by Bradley Austin Davis on 2016/03/19
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#include <vector>
#include <algorithm>
#include <vulkan/vulkan.hpp>

#include <vks/context.hpp>

namespace vks {

struct Framebuffer {
    vk::Device device;
    vk::Framebuffer framebuffer;
    vks::Image depth;
    std::vector<vks::Image> colors;

    void destroy() {
        for (auto& color : colors) {
            color.destroy();
        }
        if (depth.format != vk::Format::eUndefined) {
            depth.destroy();
        }
        if (device && framebuffer) {
            device.destroy(framebuffer);
            framebuffer = nullptr;
        }
    }
};

namespace framebuffer {

struct Builder {
protected:
    const vks::Context& context;
    vk::ImageCreateInfo imageCreateInfo{ {}, vk::ImageType::e2D };
    vk::FramebufferCreateInfo createInfo;
    vk::Format depthFormat{ vk::Format::eUndefined };
    std::vector<vk::Format> colorFormats{ vk::Format::eR8G8B8A8Unorm };
    vk::ImageUsageFlags colorUsage{ vk::ImageUsageFlagBits::eSampled };
    vk::ImageUsageFlags depthUsage{ vk::ImageUsageFlagBits::eSampled };

public:
    Builder(const vks::Context& context, vk::RenderPass renderPass, const vk::Extent2D& size)
        : context(context) {
        createInfo.renderPass = renderPass;
        createInfo.width = imageCreateInfo.extent.width = size.width;
        createInfo.height = imageCreateInfo.extent.height = size.height;
        createInfo.layers = imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
    }

    Builder& withDepthFormat(vk::Format depthFormat) {
        this->depthFormat = depthFormat;
        return *this;
    }

    Builder& withColorFormats(const vk::ArrayProxy<const vk::Format>& colorFormats) {
        auto size = colorFormats.size();
        this->colorFormats.resize(size);
        memcpy(this->colorFormats.data(), colorFormats.data(), sizeof(vk::Format) * size);
        return *this;
    }

    Builder& withLayers(uint32_t layers) {
        createInfo.layers = imageCreateInfo.arrayLayers = layers;
        return *this;
    }

    Builder& withSampleCount(vk::SampleCountFlagBits sampleCount) {
        imageCreateInfo.samples = sampleCount;
        return *this;
    }

    Builder& withDepthUsage(const vk::ImageUsageFlags& depthUsage) {
        this->depthUsage = depthUsage;
        return *this;
    }

    Builder& withColorUsage(const vk::ImageUsageFlags& colorUsage) {
        this->colorUsage = colorUsage;
        return *this;
    }

    vks::Framebuffer create() {
        vk::ImageViewCreateInfo imageView;
        imageView.viewType = vk::ImageViewType::e2D;
        imageView.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        imageView.subresourceRange.levelCount = 1;
        imageView.subresourceRange.layerCount = imageCreateInfo.arrayLayers;

        std::vector<vk::ImageView> views;
        views.reserve(colorFormats.size() + 1);


        vks::Framebuffer result;
        result.device = context.device;

        auto& colorAttachments = result.colors;
        colorAttachments.reserve(colorFormats.size());
        for (size_t i = 0; i < colorFormats.size(); ++i) {
            imageCreateInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | colorUsage;
            imageCreateInfo.format = imageView.format = colorFormats[i];
            colorAttachments.push_back(context.createImageWithView(imageCreateInfo, imageView));
            views.push_back(colorAttachments.back().view);
        }

        bool useDepth = depthFormat != vk::Format::eUndefined;
        auto& depthAttachment = result.depth;
        // Depth stencil attachment
        if (useDepth) {
            imageCreateInfo.format = depthFormat;
            imageCreateInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | depthUsage;
            imageView.format = depthFormat;
            imageView.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
            result.depth = context.createImageWithView(imageCreateInfo, imageView);
            views.push_back(depthAttachment.view);
        }

        createInfo.attachmentCount = (uint32_t)views.size();
        createInfo.pAttachments = views.data();

        result.framebuffer = context.device.createFramebuffer(createInfo);
        return result;
    }
};

}  // namespace framebuffer

}  // namespace vks
