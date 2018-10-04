#pragma once

#include <vulkan/vulkan.hpp>

#include <array>
#include <vector>

namespace vks { namespace util {

inline vk::ColorComponentFlags fullColorWriteMask() {
    return vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
           vk::ColorComponentFlagBits::eA;
}

inline vk::Viewport viewport(float width, float height, float minDepth = 0, float maxDepth = 1) {
    vk::Viewport viewport;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    return viewport;
}

#if 0
            inline vk::Viewport viewport(const glm::uvec2& size, float minDepth = 0, float maxDepth = 1) {
                return viewport((float)size.x, (float)size.y, minDepth, maxDepth);
            }

            inline vk::Viewport viewport(const vk::Extent2D& size, float minDepth = 0, float maxDepth = 1) {
                return viewport((float)size.width, (float)size.height, minDepth, maxDepth);
            }
            inline vk::Rect2D rect2D(const glm::uvec2& size, const glm::ivec2& offset = glm::ivec2(0)) {
                return rect2D(size.x, size.y, offset.x, offset.y);
            }
            inline vk::ClearColorValue clearColor(const glm::vec4& v = glm::vec4(0)) {
                vk::ClearColorValue result;
                memcpy(&result.float32, &v, sizeof(result.float32));
                return result;
            }

#endif

inline vk::Rect2D rect2D(uint32_t width, uint32_t height, int32_t offsetX = 0, int32_t offsetY = 0) {
    vk::Rect2D rect2D;
    rect2D.extent.width = width;
    rect2D.extent.height = height;
    rect2D.offset.x = offsetX;
    rect2D.offset.y = offsetY;
    return rect2D;
}

inline vk::Rect2D rect2D(const vk::Extent2D& size, const vk::Offset2D& offset = vk::Offset2D()) {
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

bool loadPipelineCacheData(std::vector<uint8_t>& outCache);
void savePipelineCacheData(const std::vector<uint8_t>& cache);

}}  // namespace vks::util

#include <mutex>

#if 0
const QString& getPipelineCacheFile() {
    static const QString PIPELINE_CACHE_FOLDER{ "" };
    static const QString PIPELINE_CACHE_FILE_NAME{ "pipeline_cache.bin" };
    static const QString PIPELINE_CACHE_FILE = FileUtils::standardPath(PIPELINE_CACHE_FOLDER) + PIPELINE_CACHE_FILE_NAME;
    return PIPELINE_CACHE_FOLDER;
}

bool vks::util::loadPipelineCacheData(std::vector<uint8_t>& outCache) {
    outCache.clear();
    const QString& cacheFile = getPipelineCacheFile();
    if (QFileInfo(cacheFile).exists()) {
        QFile file(cacheFile);
        if (file.open(QFile::ReadOnly)) {
            QByteArray data = file.readAll();
            outCache.resize(data.size());
            memcpy(outCache.data(), data.data(), data.size());
            file.close();
            return true;
        }
    }
    return false;
}

void vks::util::savePipelineCacheData(const std::vector<uint8_t>& cache) {
    QString cacheFile = getPipelineCacheFile();
    QFile saveFile(cacheFile);
    saveFile.open(QFile::WriteOnly | QFile::Truncate);
    saveFile.write((const char*)cache.data(), cache.size());
    saveFile.close();
}

static std::set<std::string> getGLExtensions() {
    static std::set<std::string> result;
    static std::once_flag once;
    std::call_once(once, [&] {
        GLint count = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &count);
        for (GLint i = 0; i < count; ++i) {
            auto name = glGetStringi(GL_EXTENSIONS, i);
            result.insert((const char*)name);
        }
    });
    return result;
}

static bool hasExtension(const std::string& name) {
    const auto& extensions = getGLExtensions();
    return 0 != extensions.count(name);
}

vks::util::gl::UuidSet vks::util::gl::getUuids() {
    static vks::util::gl::UuidSet result;
    static std::once_flag once;
    QUuid driverUuid;
    using GLUUID = std::array<GLint, 16>;

    std::call_once(once, [&] {
        GLUUID value;
        glGetIntegerv(GL_DRIVER_UUID_EXT, value.data());

        GLint deviceIdCount = 0;
        glGetIntegerv(GL_NUM_DEVICE_UUIDS_EXT, &deviceIdCount);
        for (GLint i = 0; i < deviceIdCount; ++i) {
            result.insert(QUuid(QByteArray((const char*)value.data(), (int)value.size())));
        }
    });
    return result;
}

bool vks::util::gl::contextSupported(QOpenGLContext*) {
    return hasExtension("GL_EXT_memory_object") && hasExtension("GL_EXT_semaphore");
}
#endif