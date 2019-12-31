#pragma once

#include "context.hpp"

namespace vks { namespace renderpass {

template <typename T>
static void updatePointerFromVector(const std::vector<T>& vector, uint32_t& count, const T*& data) {
    if (vector.empty()) {
        count = 0;
        data = nullptr;
    } else {
        count = static_cast<uint32_t>(vector.size());
        data = vector.data();
    }
}

template <typename T>
static void updatePointerFromVector(const std::vector<T>& vector, const T*& data) {
    if (vector.empty()) {
        data = nullptr;
    } else {
        data = vector.data();
    }
}

struct SubpassDescription : public vk::SubpassDescription {
    vk::AttachmentReference depthReference;
    std::vector<vk::AttachmentReference> inputAttachments;
    std::vector<vk::AttachmentReference> colorAttachments;
    std::vector<vk::AttachmentReference> resolveAttachments;
    std::vector<uint32_t> preserveAttachments;

    void update() {
        if (depthReference.layout != vk::ImageLayout::eUndefined) {
            pDepthStencilAttachment = &depthReference;
        }
        if (!resolveAttachments.empty() && resolveAttachments.size() != colorAttachments.size()) {
            throw std::runtime_error("Resolve attachments vector must be empty or equal in size to the color attachments vector");
        }
        updatePointerFromVector(inputAttachments, inputAttachmentCount, pInputAttachments);
        // Set the resolve attachments first, since the color attachments will overwrite the count
        updatePointerFromVector(resolveAttachments, pResolveAttachments);
        updatePointerFromVector(colorAttachments, colorAttachmentCount, pColorAttachments);
        updatePointerFromVector(preserveAttachments, preserveAttachmentCount, pPreserveAttachments);
    }

private:
};

struct Builder {
    static constexpr uint32_t DEFAULT_COLOR_INDEX{ 0 };
    static constexpr uint32_t DEFAULT_DEPTH_INDEX{ 1 };
    vk::RenderPassCreateInfo createInfo;
    std::vector<SubpassDescription> subpasses;
    std::vector<vk::AttachmentDescription> attachments;
    std::vector<vk::SubpassDependency> dependencies;

    Builder() {}

    Builder& singlePassDepencency() {
        subpasses.resize(1);
        dependencies.resize(2);
        dependencies[0] = vk::SubpassDependency()
                              .setSrcSubpass(0)
                              .setDstSubpass(VK_SUBPASS_EXTERNAL)
                              .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                              .setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
                              .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
                              .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
                              .setDependencyFlags(vk::DependencyFlagBits::eByRegion);
        dependencies[1] = vk::SubpassDependency()
                              .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                              .setDstSubpass(0)
                              .setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
                              .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                              .setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
                              .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
                              .setDependencyFlags(vk::DependencyFlagBits::eByRegion);
        return *this;
    }

    Builder& multiColor(vk::ArrayProxy<const vk::Format> colorFormats, vk::Format depthFormat = vk::Format::eUndefined) {
        singlePassDepencency();

        auto& subpassDescription = subpasses.back();
        subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        uint32_t formatCount = colorFormats.size();
        for (uint32_t i = 0; i < formatCount; ++i) {
            const auto& colorFormat = *(colorFormats.data() + i);
            // Color attachment
            auto attachment = vk::AttachmentDescription{}
                             .setFormat(colorFormat)
                             .setLoadOp(vk::AttachmentLoadOp::eClear)
                             .setStoreOp(vk::AttachmentStoreOp::eStore)
                             .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
            auto colorAttachmentIndex = addAttachment(attachment);
            subpassDescription.colorAttachments.push_back({ colorAttachmentIndex, vk::ImageLayout::eColorAttachmentOptimal });
        }

        // Depth attachment
        if (depthFormat != vk::Format::eUndefined) {
            auto attachment = vk::AttachmentDescription{}
                                  .setFormat(depthFormat)
                                  .setLoadOp(vk::AttachmentLoadOp::eClear)
                                  .setStoreOp(vk::AttachmentStoreOp::eStore)
                                  .setStencilLoadOp(vk::AttachmentLoadOp::eClear)
                                  .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                                  .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
            auto depthStencilAttachmentIndex = addAttachment(attachment);
            subpassDescription.depthReference = { depthStencilAttachmentIndex, vk::ImageLayout::eDepthStencilAttachmentOptimal };
        }

        return *this;
    }

    Builder& simple(vk::Format colorFormat = vk::Format::eR8G8B8A8Srgb, vk::Format depthFormat = vk::Format::eUndefined) {
        singlePassDepencency();
        // Color attachment
        auto attachment = vk::AttachmentDescription{}
                         .setFormat(colorFormat)
                         .setLoadOp(vk::AttachmentLoadOp::eClear)
                         .setStoreOp(vk::AttachmentStoreOp::eStore)
                         .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
        auto colorAttachmentIndex = addAttachment(attachment);
        assert(colorAttachmentIndex == DEFAULT_COLOR_INDEX);

        // Depth attachment
        attachment = vk::AttachmentDescription{}
                              .setFormat(depthFormat)
                              .setLoadOp(vk::AttachmentLoadOp::eClear)
                              .setStoreOp(vk::AttachmentStoreOp::eDontCare)
                              .setStencilLoadOp(vk::AttachmentLoadOp::eClear)
                              .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                              .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
        auto depthStencilAttachmentIndex = addAttachment(attachment);
        assert(depthStencilAttachmentIndex == DEFAULT_DEPTH_INDEX);

        auto& subpassDescription = subpasses.back();
        subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpassDescription.depthReference = { depthStencilAttachmentIndex, vk::ImageLayout::eDepthStencilAttachmentOptimal };
        subpassDescription.colorAttachments = {
            vk::AttachmentReference{ colorAttachmentIndex, vk::ImageLayout::eColorAttachmentOptimal },
        };
        return *this;
    }

    uint32_t addAttachment(const vk::AttachmentDescription& attachment) {
        auto result = attachments.size();
        attachments.push_back(attachment);
        return static_cast<uint32_t>(result);
    }

    vk::RenderPass create(const vk::Device& device) {
        std::vector<vk::SubpassDescription> rawSubpasses;
        rawSubpasses.reserve(subpasses.size());
        for (auto& subpass : subpasses) {
            subpass.update();
            rawSubpasses.push_back(subpass);
        }
        updatePointerFromVector(rawSubpasses, createInfo.subpassCount, createInfo.pSubpasses);
        updatePointerFromVector(attachments, createInfo.attachmentCount, createInfo.pAttachments);
        updatePointerFromVector(dependencies, createInfo.dependencyCount, createInfo.pDependencies);
        return device.createRenderPass(createInfo);
    }
};

}}  // namespace vks::renderpass

#if 0 
attachments[0] = vk::AttachmentDescription{}
                     .setFormat(colorformat)
                     .setLoadOp(vk::AttachmentLoadOp::eClear)
                     .setStoreOp(vk::AttachmentStoreOp::eStore)
                     .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
// Depth attachment
attachments[1] =

// Only one depth attachment, so put it first in the references
vk::AttachmentReference depthReference;
depthReference.attachment = 1;
depthReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

std::vector<vk::AttachmentReference> colorAttachmentReferences;
{
    vk::AttachmentReference colorReference;
    colorReference.attachment = 0;
    colorReference.layout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachmentReferences.push_back(colorReference);
}

using vPSFB = vk::PipelineStageFlagBits;
using vAFB = vk::AccessFlagBits;
std::vector<vk::SubpassDependency> subpassDependencies{
    { 0, VK_SUBPASS_EXTERNAL, vPSFB::eColorAttachmentOutput, vPSFB::eBottomOfPipe, vAFB::eColorAttachmentRead | vAFB::eColorAttachmentWrite, vAFB::eMemoryRead,
      vk::DependencyFlagBits::eByRegion },
    { VK_SUBPASS_EXTERNAL, 0, vPSFB::eBottomOfPipe, vPSFB::eColorAttachmentOutput, vAFB::eMemoryRead, vAFB::eColorAttachmentRead | vAFB::eColorAttachmentWrite,
      vk::DependencyFlagBits::eByRegion },
};
std::vector<vk::SubpassDescription> subpasses{
    { {},
      vk::PipelineBindPoint::eGraphics,
      // Input attachment references
      0,
      nullptr,
      // Color / resolve attachment references
      (uint32_t)colorAttachmentReferences.size(),
      colorAttachmentReferences.data(),
      nullptr,
      // Depth stecil attachment reference,
      &depthReference,
      // Preserve attachments
      0,
      nullptr },
};
#endif