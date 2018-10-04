#pragma once

#include "allocation.hpp"

namespace vks {

// Encaspulates an image, the memory for that image, a view of the image,
// as well as a sampler and the image format.
//
// The sampler is not populated by the allocation code, but is provided
// for convenience and easy cleanup if it is populated.

struct Image : public Allocation {
private:
    using Parent = Allocation;

public:
    vk::Image image;
    vk::Extent3D extent;
    vk::ImageView view;
    vk::Sampler sampler;
    vk::Format format{ vk::Format::eUndefined };

    operator bool() const { return image.operator bool(); }

    void destroy() override {
        if (sampler) {
            device.destroy(sampler);
            sampler = nullptr;
        }
        if (view) {
            device.destroy(view);
            view = nullptr;
        }
        if (image) {
            device.destroy(image);
            image = nullptr;
        }
        Parent::destroy();
    }
};

}  // namespace vks

