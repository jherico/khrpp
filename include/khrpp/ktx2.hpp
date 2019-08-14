//
//  Created by Bradley Austin Davis on 2019/08/12
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once
#ifndef khrpp_ktx2_hpp
#define khrpp_ktx2_hpp

#include "constants.hpp"
#include "helpers.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace khrpp { namespace ktx2 {

enum class SupercompressionScheme : uint32_t
{
    NONE = 0,
    BASIS = 1,
    LZMA = 2,
    ZLIB = 3,
    ZSTD = 4,
};

struct Descriptor {
    static const size_t IDENTIFIER_LENGTH{ 12 };
    using Identifier = std::array<uint8_t, IDENTIFIER_LENGTH>;

    static const Identifier& IDENTIFIER() {
        static const Identifier IDENTIFIER_VALUE{ { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A } };
        return IDENTIFIER_VALUE;
    };

    struct Header {
        Byte identifier[IDENTIFIER_LENGTH];
        vk::Format format{ vk::Format::UNDEFINED };
        uint32_t typeSize;
        uint32_t pixelWidth{ 1 };
        uint32_t pixelHeight{ 1 };
        uint32_t pixelDepth{ 0 };
        uint32_t arrayElementCount{ 0 };
        uint32_t faceCount{ 1 };
        uint32_t levelCount{ 1 };
        SupercompressionScheme supercompressionScheme{ 0 };

        // Data format descriptor
        uint32_t dfdByteOffset{ 0 };
        uint32_t dfdByteLength{ 0 };
        // Key Value data
        uint32_t kvdByteOffset{ 0 };
        uint32_t kvdByteLength{ 0 };
        // Supercompression global data
        uint64_t sgdByteOffset{ 0 };
        uint64_t sgdByteLength{ 0 };
    } header;

    struct LevelDescriptor {
        uint64_t byteOffset{ 0 };
        uint64_t byteLength{ 0 };
        uint64_t uncompressedByteLength{ 0 };
    };
    std::vector<LevelDescriptor> levels;

    // DFD 1.3 (1.2 for now)
    // FIXME create a dfd wrapper and build a vector of those
    Bytes dfd;

    // Key/value pairs
    std::unordered_map<String, Bytes> kvd;

    struct BasisDescriptor {
        struct BasisHeader {
            uint32_t globalFlags;
            uint16_t endpointCount;
            uint16_t selectorCount;
            uint32_t endpointsByteLength;
            uint32_t selectorsByteLength;
            uint32_t tablesByteLength;
            uint32_t extendedByteLength;
        } header;

        struct BasisImageDescriptor {
            uint32_t sliceFlags;
            uint32_t sliceByteOffset;
            uint32_t sliceByteLength;
            uint32_t alphaSliceByteOffset;
            uint32_t alphaSliceByteLength;
        };

        std::vector<BasisImageDescriptor> images;
        Bytes endpointsData;
        Bytes selectorsData;
        Bytes tableData;

        void parse(const Header& ktxHeader, const uint8_t* const data, size_t size);
    };

    // Supercompression global data
    std::optional<BasisDescriptor> basisData;

    // Mip descriptors?
    void parse(const uint8_t* const data, size_t size);

    static bool validate(const uint8_t* const data, size_t size) noexcept {
        try {
            Descriptor().parse(data, size);
        } catch (const std::runtime_error& err) {
            return false;
        }
        return true;
    }
};

}}  // namespace khronos::ktx2

namespace khrpp { namespace ktx2 {

inline const std::unordered_set<vk::Format>& VALID_VK_FORMATS() {
    static const std::unordered_set<vk::Format> VALID_VK_FORMATS{
        vk::Format::UNDEFINED,
        vk::Format::R4G4_UNORM_PACK8,
        vk::Format::R4G4B4A4_UNORM_PACK16,
        vk::Format::B4G4R4A4_UNORM_PACK16,
        vk::Format::R5G6B5_UNORM_PACK16,
        vk::Format::B5G6R5_UNORM_PACK16,
        vk::Format::R5G5B5A1_UNORM_PACK16,
        vk::Format::B5G5R5A1_UNORM_PACK16,
        vk::Format::A1R5G5B5_UNORM_PACK16,
        vk::Format::R8_UNORM,
        vk::Format::R8_SNORM,
        vk::Format::R8_UINT,
        vk::Format::R8_SINT,
        vk::Format::R8_SRGB,
        vk::Format::R8G8_UNORM,
        vk::Format::R8G8_SNORM,
        vk::Format::R8G8_UINT,
        vk::Format::R8G8_SINT,
        vk::Format::R8G8_SRGB,
        vk::Format::R8G8B8_UNORM,
        vk::Format::R8G8B8_SNORM,
        vk::Format::R8G8B8_UINT,
        vk::Format::R8G8B8_SINT,
        vk::Format::R8G8B8_SRGB,
        vk::Format::B8G8R8_UNORM,
        vk::Format::B8G8R8_SNORM,
        vk::Format::B8G8R8_UINT,
        vk::Format::B8G8R8_SINT,
        vk::Format::B8G8R8_SRGB,
        vk::Format::R8G8B8A8_UNORM,
        vk::Format::R8G8B8A8_SNORM,
        vk::Format::R8G8B8A8_UINT,
        vk::Format::R8G8B8A8_SINT,
        vk::Format::R8G8B8A8_SRGB,
        vk::Format::B8G8R8A8_UNORM,
        vk::Format::B8G8R8A8_SNORM,
        vk::Format::B8G8R8A8_UINT,
        vk::Format::B8G8R8A8_SINT,
        vk::Format::B8G8R8A8_SRGB,
        vk::Format::A2R10G10B10_UNORM_PACK32,
        vk::Format::A2R10G10B10_SNORM_PACK32,
        vk::Format::A2R10G10B10_UINT_PACK32,
        vk::Format::A2R10G10B10_SINT_PACK32,
        vk::Format::A2B10G10R10_UNORM_PACK32,
        vk::Format::A2B10G10R10_SNORM_PACK32,
        vk::Format::A2B10G10R10_UINT_PACK32,
        vk::Format::A2B10G10R10_SINT_PACK32,
        vk::Format::R16_UNORM,
        vk::Format::R16_SNORM,
        vk::Format::R16_UINT,
        vk::Format::R16_SINT,
        vk::Format::R16_SFLOAT,
        vk::Format::R16G16_UNORM,
        vk::Format::R16G16_SNORM,
        vk::Format::R16G16_UINT,
        vk::Format::R16G16_SINT,
        vk::Format::R16G16_SFLOAT,
        vk::Format::R16G16B16_UNORM,
        vk::Format::R16G16B16_SNORM,
        vk::Format::R16G16B16_UINT,
        vk::Format::R16G16B16_SINT,
        vk::Format::R16G16B16_SFLOAT,
        vk::Format::R16G16B16A16_UNORM,
        vk::Format::R16G16B16A16_SNORM,
        vk::Format::R16G16B16A16_UINT,
        vk::Format::R16G16B16A16_SINT,
        vk::Format::R16G16B16A16_SFLOAT,
        vk::Format::R32_UINT,
        vk::Format::R32_SINT,
        vk::Format::R32_SFLOAT,
        vk::Format::R32G32_UINT,
        vk::Format::R32G32_SINT,
        vk::Format::R32G32_SFLOAT,
        vk::Format::R32G32B32_UINT,
        vk::Format::R32G32B32_SINT,
        vk::Format::R32G32B32_SFLOAT,
        vk::Format::R32G32B32A32_UINT,
        vk::Format::R32G32B32A32_SINT,
        vk::Format::R32G32B32A32_SFLOAT,
        vk::Format::R64_UINT,
        vk::Format::R64_SINT,
        vk::Format::R64_SFLOAT,
        vk::Format::R64G64_UINT,
        vk::Format::R64G64_SINT,
        vk::Format::R64G64_SFLOAT,
        vk::Format::R64G64B64_UINT,
        vk::Format::R64G64B64_SINT,
        vk::Format::R64G64B64_SFLOAT,
        vk::Format::R64G64B64A64_UINT,
        vk::Format::R64G64B64A64_SINT,
        vk::Format::R64G64B64A64_SFLOAT,
        vk::Format::B10G11R11_UFLOAT_PACK32,
        vk::Format::E5B9G9R9_UFLOAT_PACK32,
        vk::Format::D16_UNORM,
        vk::Format::X8_D24_UNORM_PACK32,
        vk::Format::D32_SFLOAT,
        vk::Format::S8_UINT,
        vk::Format::D16_UNORM_S8_UINT,
        vk::Format::D24_UNORM_S8_UINT,
        vk::Format::D32_SFLOAT_S8_UINT,
        vk::Format::BC1_RGB_UNORM_BLOCK,
        vk::Format::BC1_RGB_SRGB_BLOCK,
        vk::Format::BC1_RGBA_UNORM_BLOCK,
        vk::Format::BC1_RGBA_SRGB_BLOCK,
        vk::Format::BC2_UNORM_BLOCK,
        vk::Format::BC2_SRGB_BLOCK,
        vk::Format::BC3_UNORM_BLOCK,
        vk::Format::BC3_SRGB_BLOCK,
        vk::Format::BC4_UNORM_BLOCK,
        vk::Format::BC4_SNORM_BLOCK,
        vk::Format::BC5_UNORM_BLOCK,
        vk::Format::BC5_SNORM_BLOCK,
        vk::Format::BC6H_UFLOAT_BLOCK,
        vk::Format::BC6H_SFLOAT_BLOCK,
        vk::Format::BC7_UNORM_BLOCK,
        vk::Format::BC7_SRGB_BLOCK,
        vk::Format::ETC2_R8G8B8_UNORM_BLOCK,
        vk::Format::ETC2_R8G8B8_SRGB_BLOCK,
        vk::Format::ETC2_R8G8B8A1_UNORM_BLOCK,
        vk::Format::ETC2_R8G8B8A1_SRGB_BLOCK,
        vk::Format::ETC2_R8G8B8A8_UNORM_BLOCK,
        vk::Format::ETC2_R8G8B8A8_SRGB_BLOCK,
        vk::Format::EAC_R11_UNORM_BLOCK,
        vk::Format::EAC_R11_SNORM_BLOCK,
        vk::Format::EAC_R11G11_UNORM_BLOCK,
        vk::Format::EAC_R11G11_SNORM_BLOCK,
        vk::Format::ASTC_4x4_UNORM_BLOCK,
        vk::Format::ASTC_4x4_SRGB_BLOCK,
        vk::Format::ASTC_5x4_UNORM_BLOCK,
        vk::Format::ASTC_5x4_SRGB_BLOCK,
        vk::Format::ASTC_5x5_UNORM_BLOCK,
        vk::Format::ASTC_5x5_SRGB_BLOCK,
        vk::Format::ASTC_6x5_UNORM_BLOCK,
        vk::Format::ASTC_6x5_SRGB_BLOCK,
        vk::Format::ASTC_6x6_UNORM_BLOCK,
        vk::Format::ASTC_6x6_SRGB_BLOCK,
        vk::Format::ASTC_8x5_UNORM_BLOCK,
        vk::Format::ASTC_8x5_SRGB_BLOCK,
        vk::Format::ASTC_8x6_UNORM_BLOCK,
        vk::Format::ASTC_8x6_SRGB_BLOCK,
        vk::Format::ASTC_8x8_UNORM_BLOCK,
        vk::Format::ASTC_8x8_SRGB_BLOCK,
        vk::Format::ASTC_10x5_UNORM_BLOCK,
        vk::Format::ASTC_10x5_SRGB_BLOCK,
        vk::Format::ASTC_10x6_UNORM_BLOCK,
        vk::Format::ASTC_10x6_SRGB_BLOCK,
        vk::Format::ASTC_10x8_UNORM_BLOCK,
        vk::Format::ASTC_10x8_SRGB_BLOCK,
        vk::Format::ASTC_10x10_UNORM_BLOCK,
        vk::Format::ASTC_10x10_SRGB_BLOCK,
        vk::Format::ASTC_12x10_UNORM_BLOCK,
        vk::Format::ASTC_12x10_SRGB_BLOCK,
        vk::Format::ASTC_12x12_UNORM_BLOCK,
        vk::Format::ASTC_12x12_SRGB_BLOCK,
        vk::Format::G8B8G8R8_422_UNORM,
        vk::Format::B8G8R8G8_422_UNORM,
        vk::Format::G8_B8_R8_3PLANE_420_UNORM,
        vk::Format::G8_B8R8_2PLANE_420_UNORM,
        vk::Format::G8_B8_R8_3PLANE_422_UNORM,
        vk::Format::G8_B8R8_2PLANE_422_UNORM,
        vk::Format::G8_B8_R8_3PLANE_444_UNORM,
        vk::Format::R10X6_UNORM_PACK16,
        vk::Format::R10X6G10X6_UNORM_2PACK16,
        vk::Format::R10X6G10X6B10X6A10X6_UNORM_4PACK16,
        vk::Format::G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
        vk::Format::B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
        vk::Format::G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
        vk::Format::G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
        vk::Format::G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
        vk::Format::G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
        vk::Format::G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
        vk::Format::R12X4_UNORM_PACK16,
        vk::Format::R12X4G12X4_UNORM_2PACK16,
        vk::Format::R12X4G12X4B12X4A12X4_UNORM_4PACK16,
        vk::Format::G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
        vk::Format::B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
        vk::Format::G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
        vk::Format::G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
        vk::Format::G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
        vk::Format::G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
        vk::Format::G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
        vk::Format::G16B16G16R16_422_UNORM,
        vk::Format::B16G16R16G16_422_UNORM,
        vk::Format::G16_B16_R16_3PLANE_420_UNORM,
        vk::Format::G16_B16R16_2PLANE_420_UNORM,
        vk::Format::G16_B16_R16_3PLANE_422_UNORM,
        vk::Format::G16_B16R16_2PLANE_422_UNORM,
        vk::Format::G16_B16_R16_3PLANE_444_UNORM,
        vk::Format::PVRTC1_2BPP_UNORM_BLOCK_IMG,
        vk::Format::PVRTC1_4BPP_UNORM_BLOCK_IMG,
        vk::Format::PVRTC2_2BPP_UNORM_BLOCK_IMG,
        vk::Format::PVRTC2_4BPP_UNORM_BLOCK_IMG,
        vk::Format::PVRTC1_2BPP_SRGB_BLOCK_IMG,
        vk::Format::PVRTC1_4BPP_SRGB_BLOCK_IMG,
        vk::Format::PVRTC2_2BPP_SRGB_BLOCK_IMG,
        vk::Format::PVRTC2_4BPP_SRGB_BLOCK_IMG,
        vk::Format::ASTC_4x4_SFLOAT_BLOCK_EXT,
        vk::Format::ASTC_5x4_SFLOAT_BLOCK_EXT,
        vk::Format::ASTC_5x5_SFLOAT_BLOCK_EXT,
        vk::Format::ASTC_6x5_SFLOAT_BLOCK_EXT,
        vk::Format::ASTC_6x6_SFLOAT_BLOCK_EXT,
        vk::Format::ASTC_8x5_SFLOAT_BLOCK_EXT,
        vk::Format::ASTC_8x6_SFLOAT_BLOCK_EXT,
        vk::Format::ASTC_8x8_SFLOAT_BLOCK_EXT,
        vk::Format::ASTC_10x5_SFLOAT_BLOCK_EXT,
        vk::Format::ASTC_10x6_SFLOAT_BLOCK_EXT,
        vk::Format::ASTC_10x8_SFLOAT_BLOCK_EXT,
        vk::Format::ASTC_10x10_SFLOAT_BLOCK_EXT,
        vk::Format::ASTC_12x10_SFLOAT_BLOCK_EXT,
        vk::Format::ASTC_12x12_SFLOAT_BLOCK_EXT,
    };
    return VALID_VK_FORMATS;
}

inline void Descriptor::BasisDescriptor::parse(const Header& ktxHeader, const uint8_t* const data, size_t size) {
    AlignedStreamBuffer buffer{ size, data };
    if (!buffer.read(header)) {
        throw std::runtime_error("Unable to parse KTX2 basis supercompression header");
    }

    auto levelCount = std::max<uint32_t>(1, ktxHeader.levelCount);
    auto layerCount = std::max<uint32_t>(1, ktxHeader.arrayElementCount);
    auto faceCount = std::max<uint32_t>(1, ktxHeader.faceCount);
    auto pixelDepth = std::max<uint32_t>(1, ktxHeader.pixelDepth);
    uint32_t imageCount = levelCount * layerCount * faceCount * pixelDepth;
    for (uint32_t imageIndex = 0; imageIndex < imageCount; ++imageIndex) {
        BasisImageDescriptor imageDescriptor;
        if (!buffer.read(imageDescriptor)) {
            throw std::runtime_error(FORMAT("Unable to parse KTX2 basis image descriptor header {}", imageIndex));
        }
        images.push_back(imageDescriptor);
    }

    if (!buffer.readVector(header.endpointsByteLength, endpointsData)) {
        throw std::runtime_error("Unable to parse KTX2 basis endpointsData");
    }
    if (!buffer.readVector(header.selectorsByteLength, selectorsData)) {
        throw std::runtime_error("Unable to parse KTX2 basis selectorsData");
    }
    if (!buffer.readVector(header.tablesByteLength, tableData)) {
        throw std::runtime_error("Unable to parse KTX2 basis tableData");
    }
    if (header.extendedByteLength != 0) {
        Bytes extendedData;
        if (!buffer.readVector(header.extendedByteLength, extendedData)) {
            throw std::runtime_error("Unable to parse KTX2 basis extendedData");
        }
    }

    if (buffer.size() != 0) {
        throw std::runtime_error("Unexpected leftover KTX2 basis data");
    }
}

inline void Descriptor::parse(const uint8_t* const data, size_t size) {
    AlignedStreamBuffer buffer{ size, data };

    if (!buffer.read(header)) {
        throw std::runtime_error("Unable to parse KTX2 header");
    }

	if (0 == VALID_VK_FORMATS().count(header.format)) {
		throw std::runtime_error(FORMAT("Invalid vulkan format {}", static_cast<uint32_t>(header.format)));
	}

    if (0 != memcmp(IDENTIFIER().data(), header.identifier, IDENTIFIER_LENGTH)) {
        throw std::runtime_error("Invalid KTX identifier bytes");
    }

    // Mip level index
    {
        uint32_t mipLevelCount = std::max<uint32_t>(1, header.levelCount);
        for (uint32_t mipLevel = 0; mipLevel < mipLevelCount; ++mipLevel) {
            LevelDescriptor level;
            if (!buffer.read(level)) {
                throw std::runtime_error(FORMAT("Unable to read KTX2 mip level descriptor {}", mipLevel));
            }
            // FIXME Validate the values?  ensure the offset is always dropping?
            levels.push_back(level);
        }
    }

    // DFD block
    if (header.dfdByteLength) {
        if (buffer.offset() != header.dfdByteOffset) {
            throw std::runtime_error("Invalid DFD byte offset");
        }
        uint32_t dfdSize;
        if (!buffer.read(dfdSize)) {
            throw std::runtime_error("Unable to read KTX2 dfd descriptor size");
        }

        if (dfdSize != header.dfdByteLength) {
            throw std::runtime_error("DFD descriptor size mismatch.  Header size must match DFD size");
        }

        buffer.readVector(header.dfdByteLength - sizeof(dfdSize), dfd);
    }

    // Key/value data
    if (header.kvdByteLength) {
        auto offset = buffer.offset();
        if (buffer.offset() != header.kvdByteOffset) {
            throw std::runtime_error("Invalid key/value data byte offset");
        }
        auto kvBuffer = buffer.front(header.kvdByteLength);
        if (!buffer.skip(header.kvdByteLength)) {
            throw std::runtime_error("Unable to read key/value data");
        }
		parseKtxKeyValueData(kvBuffer, kvd);
    }

    if (!buffer.align(8)) {
        throw std::runtime_error("Unable to align to kvd/sgd, or alignment padding is non-Zero");
    }

    if (header.sgdByteLength) {
        if (buffer.offset() != header.sgdByteOffset) {
            throw std::runtime_error("Invalid supercompression data byte offset");
        }

        if (!buffer.skip(header.sgdByteLength)) {
            throw std::runtime_error("Unable to read supercompression data");
        }

        if (header.supercompressionScheme == SupercompressionScheme::BASIS) {
            basisData.emplace().parse(header, data + header.sgdByteOffset, header.sgdByteLength);
        }
    }

    if (!buffer.align(8)) {
        throw std::runtime_error("Unable to align on sgd/image data interval, or alignment padding is non-Zero");
    }

    // validate per-mip levels
    {
        for (auto itr = levels.rbegin(); itr != levels.rend(); ++itr) {
            size_t mipLevel = header.levelCount - (itr - levels.rbegin());
            const auto& level = *itr;
            auto offset = buffer.offset();
            if (offset != level.byteOffset) {
                throw std::runtime_error(
                    FORMAT("Invalid image level byte offset {} or btye length {} for mip {}", level.byteOffset, level.byteLength, mipLevel));
            }
            if (!buffer.skip(level.byteLength)) {
                throw std::runtime_error(FORMAT("Unable to read image btye length {} for mip {}", level.byteLength, mipLevel));
            }
            if (!buffer.empty() && !buffer.align(8)) {
                throw std::runtime_error("Unable to align on mip data interval, or alignment padding is non-Zero");
            }
        }
    }
    if (!buffer.empty()) {
        throw std::runtime_error("Unable to align on mip data interval, or alignment padding is non-Zero");
    }
}

}}  // namespace khronos::ktx2

#endif
