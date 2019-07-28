//
//  Created by Bradley Austin Davis on 2017/05/13
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef khronos_khr_hpp
#define khronos_khr_hpp

#include <unordered_map>
#include <cstdint>

namespace khronos {
namespace gl {

enum class Type : uint32_t
{
    COMPRESSED = 0,
    // GL 4.4 Table 8.2
    UNSIGNED_BYTE = 0x1401,
    BYTE = 0x1400,
    UNSIGNED_SHORT = 0x1403,
    SHORT = 0x1402,
    UNSIGNED_INT = 0x1405,
    INT = 0x1404,
    HALF_FLOAT = 0x140B,
    FLOAT = 0x1406,
    UNSIGNED_BYTE_3_3_2 = 0x8032,
    UNSIGNED_BYTE_2_3_3_REV = 0x8362,
    UNSIGNED_SHORT_5_6_5 = 0x8363,
    UNSIGNED_SHORT_5_6_5_REV = 0x8364,
    UNSIGNED_SHORT_4_4_4_4 = 0x8033,
    UNSIGNED_SHORT_4_4_4_4_REV = 0x8365,
    UNSIGNED_SHORT_5_5_5_1 = 0x8034,
    UNSIGNED_SHORT_1_5_5_5_REV = 0x8366,
    UNSIGNED_INT_8_8_8_8 = 0x8035,
    UNSIGNED_INT_8_8_8_8_REV = 0x8367,
    UNSIGNED_INT_10_10_10_2 = 0x8036,
    UNSIGNED_INT_2_10_10_10_REV = 0x8368,
    UNSIGNED_INT_24_8 = 0x84FA,
    UNSIGNED_INT_10F_11F_11F_REV = 0x8C3B,
    UNSIGNED_INT_5_9_9_9_REV = 0x8C3E,
    FLOAT_32_UNSIGNED_INT_24_8_REV = 0x8DAD,
};

namespace texture {

enum class Format : uint32_t
{
    COMPRESSED = 0,

    // GL 4.4 Table 8.3
    STENCIL_INDEX = 0x1901,
    DEPTH_COMPONENT = 0x1902,
    DEPTH_STENCIL = 0x84F9,

    LUMINANCE = 0x1909,
    RED = 0x1903,
    GREEN = 0x1904,
    BLUE = 0x1905,
    RG = 0x8227,
    RGB = 0x1907,
    RGBA = 0x1908,
    BGR = 0x80E0,
    BGRA = 0x80E1,

    RG_INTEGER = 0x8228,
    RED_INTEGER = 0x8D94,
    GREEN_INTEGER = 0x8D95,
    BLUE_INTEGER = 0x8D96,
    RGB_INTEGER = 0x8D98,
    RGBA_INTEGER = 0x8D99,
    BGR_INTEGER = 0x8D9A,
    BGRA_INTEGER = 0x8D9B,
};

enum class InternalFormat : uint32_t
{
    // GLES 2.0 
    LUMINANCE8 = 0x8040,

    // GL 4.4 Table 8.12
    R8 = 0x8229,
    R8_SNORM = 0x8F94,

    R16 = 0x822A,
    R16_SNORM = 0x8F98,

    RG8 = 0x822B,
    RG8_SNORM = 0x8F95,

    RG16 = 0x822C,
    RG16_SNORM = 0x8F99,

    R3_G3_B2 = 0x2A10,
    RGB4 = 0x804F,
    RGB5 = 0x8050,
    RGB565 = 0x8D62,

    RGB8 = 0x8051,
    RGB8_SNORM = 0x8F96,
    RGB10 = 0x8052,
    RGB12 = 0x8053,

    RGB16 = 0x8054,
    RGB16_SNORM = 0x8F9A,

    RGBA2 = 0x8055,
    RGBA4 = 0x8056,
    RGB5_A1 = 0x8057,
    RGBA8 = 0x8058,
    RGBA8_SNORM = 0x8F97,

    RGB10_A2 = 0x8059,
    RGB10_A2UI = 0x906F,

    RGBA12 = 0x805A,
    RGBA16 = 0x805B,
    RGBA16_SNORM = 0x8F9B,

    SRGB8 = 0x8C41,
    SRGB8_ALPHA8 = 0x8C43,

    R16F = 0x822D,
    RG16F = 0x822F,
    RGB16F = 0x881B,
    RGBA16F = 0x881A,

    R32F = 0x822E,
    RG32F = 0x8230,
    RGB32F = 0x8815,
    RGBA32F = 0x8814,

    R11F_G11F_B10F = 0x8C3A,
    RGB9_E5 = 0x8C3D,

    R8I = 0x8231,
    R8UI = 0x8232,
    R16I = 0x8233,
    R16UI = 0x8234,
    R32I = 0x8235,
    R32UI = 0x8236,
    RG8I = 0x8237,
    RG8UI = 0x8238,
    RG16I = 0x8239,
    RG16UI = 0x823A,
    RG32I = 0x823B,
    RG32UI = 0x823C,

    RGB8I = 0x8D8F,
    RGB8UI = 0x8D7D,
    RGB16I = 0x8D89,
    RGB16UI = 0x8D77,

    RGB32I = 0x8D83,
    RGB32UI = 0x8D71,
    RGBA8I = 0x8D8E,
    RGBA8UI = 0x8D7C,
    RGBA16I = 0x8D88,
    RGBA16UI = 0x8D76,
    RGBA32I = 0x8D82,

    RGBA32UI = 0x8D70,

    // GL 4.4 Table 8.13
    DEPTH_COMPONENT16 = 0x81A5,
    DEPTH_COMPONENT24 = 0x81A6,
    DEPTH_COMPONENT32 = 0x81A7,

    DEPTH_COMPONENT32F = 0x8CAC,
    DEPTH24_STENCIL8 = 0x88F0,
    DEPTH32F_STENCIL8 = 0x8CAD,

    STENCIL_INDEX1 = 0x8D46,
    STENCIL_INDEX4 = 0x8D47,
    STENCIL_INDEX8 = 0x8D48,
    STENCIL_INDEX16 = 0x8D49,

    // GL 4.4 Table 8.14
    COMPRESSED_RED = 0x8225,
    COMPRESSED_RG = 0x8226,
    COMPRESSED_RGB = 0x84ED,
    COMPRESSED_RGBA = 0x84EE,

    COMPRESSED_SRGB = 0x8C48,
    COMPRESSED_SRGB_ALPHA = 0x8C49,

    COMPRESSED_ETC1_RGB8_OES = 0x8D64,

    COMPRESSED_RGB_S3TC_DXT1_EXT = 0x83F0,
    COMPRESSED_RGBA_S3TC_DXT1_EXT = 0x83F1,
    COMPRESSED_RGBA_S3TC_DXT3_EXT = 0x83F2,
    COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3,
    COMPRESSED_SRGB_S3TC_DXT1_EXT = 0x8C4C,
    COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT = 0x8C4D,
    COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT = 0x8C4E,
    COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT = 0x8C4F,

    COMPRESSED_RED_RGTC1 = 0x8DBB,
    COMPRESSED_SIGNED_RED_RGTC1 = 0x8DBC,
    COMPRESSED_RG_RGTC2 = 0x8DBD,
    COMPRESSED_SIGNED_RG_RGTC2 = 0x8DBE,

    COMPRESSED_RGBA_BPTC_UNORM = 0x8E8C,
    COMPRESSED_SRGB_ALPHA_BPTC_UNORM = 0x8E8D,
    COMPRESSED_RGB_BPTC_SIGNED_FLOAT = 0x8E8E,
    COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT = 0x8E8F,

    COMPRESSED_RGB8_ETC2 = 0x9274,
    COMPRESSED_SRGB8_ETC2 = 0x9275,
    COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9276,
    COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9277,
    COMPRESSED_RGBA8_ETC2_EAC = 0x9278,
    COMPRESSED_SRGB8_ALPHA8_ETC2_EAC = 0x9279,

    COMPRESSED_R11_EAC = 0x9270,
    COMPRESSED_SIGNED_R11_EAC = 0x9271,
    COMPRESSED_RG11_EAC = 0x9272,
    COMPRESSED_SIGNED_RG11_EAC = 0x9273,

    COMPRESSED_RGBA_ASTC_4x4 = 0x93b0,
    COMPRESSED_RGBA_ASTC_5x4 = 0x93b1,
    COMPRESSED_RGBA_ASTC_5x5 = 0x93b2,
    COMPRESSED_RGBA_ASTC_6x5 = 0x93b3,
    COMPRESSED_RGBA_ASTC_6x6 = 0x93b4,
    COMPRESSED_RGBA_ASTC_8x5 = 0x93b5,
    COMPRESSED_RGBA_ASTC_8x6 = 0x93b6,
    COMPRESSED_RGBA_ASTC_8x8 = 0x93b7,
    COMPRESSED_RGBA_ASTC_10x5 = 0x93b8,
    COMPRESSED_RGBA_ASTC_10x6 = 0x93b9,
    COMPRESSED_RGBA_ASTC_10x8 = 0x93ba,
    COMPRESSED_RGBA_ASTC_10x10 = 0x93bb,
    COMPRESSED_RGBA_ASTC_12x10 = 0x93bc,
    COMPRESSED_RGBA_ASTC_12x12 = 0x93bd,
    COMPRESSED_RGBA_ASTC_3x3x3_OES = 0x93c0,
    COMPRESSED_RGBA_ASTC_4x3x3_OES = 0x93c1,
    COMPRESSED_RGBA_ASTC_4x4x3_OES = 0x93c2,
    COMPRESSED_RGBA_ASTC_4x4x4_OES = 0x93c3,
    COMPRESSED_RGBA_ASTC_5x4x4_OES = 0x93c4,
    COMPRESSED_RGBA_ASTC_5x5x4_OES = 0x93c5,
    COMPRESSED_RGBA_ASTC_5x5x5_OES = 0x93c6,
    COMPRESSED_RGBA_ASTC_6x5x5_OES = 0x93c7,
    COMPRESSED_RGBA_ASTC_6x6x5_OES = 0x93c8,
    COMPRESSED_RGBA_ASTC_6x6x6_OES = 0x93c9,
    COMPRESSED_SRGB8_ALPHA8_ASTC_4x4 = 0x93d0,
    COMPRESSED_SRGB8_ALPHA8_ASTC_5x4 = 0x93d1,
    COMPRESSED_SRGB8_ALPHA8_ASTC_5x5 = 0x93d2,
    COMPRESSED_SRGB8_ALPHA8_ASTC_6x5 = 0x93d3,
    COMPRESSED_SRGB8_ALPHA8_ASTC_6x6 = 0x93d4,
    COMPRESSED_SRGB8_ALPHA8_ASTC_8x5 = 0x93d5,
    COMPRESSED_SRGB8_ALPHA8_ASTC_8x6 = 0x93d6,
    COMPRESSED_SRGB8_ALPHA8_ASTC_8x8 = 0x93d7,
    COMPRESSED_SRGB8_ALPHA8_ASTC_10x5 = 0x93d8,
    COMPRESSED_SRGB8_ALPHA8_ASTC_10x6 = 0x93d9,
    COMPRESSED_SRGB8_ALPHA8_ASTC_10x8 = 0x93da,
    COMPRESSED_SRGB8_ALPHA8_ASTC_10x10 = 0x93db,
    COMPRESSED_SRGB8_ALPHA8_ASTC_12x10 = 0x93dc,
    COMPRESSED_SRGB8_ALPHA8_ASTC_12x12 = 0x93dd,
    COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES = 0x93e0,
    COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES = 0x93e1,
    COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES = 0x93e2,
    COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES = 0x93e3,
    COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES = 0x93e4,
    COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES = 0x93e5,
    COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES = 0x93e6,
    COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES = 0x93e7,
    COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES = 0x93e8,
    COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES = 0x93e9,
};

inline uint8_t evalUncompressedBlockBitSize(InternalFormat format) {
    switch (format) {
        case InternalFormat::STENCIL_INDEX1:
            return 1;

        case InternalFormat::STENCIL_INDEX4:
            return 4;

        case InternalFormat::R8:
        case InternalFormat::R8_SNORM:
        case InternalFormat::R8I:
        case InternalFormat::R8UI:
        case InternalFormat::R3_G3_B2:
        case InternalFormat::RGBA2:
        case InternalFormat::STENCIL_INDEX8:
            return 8;

        case InternalFormat::RGB4:
            return 12;

        case InternalFormat::STENCIL_INDEX16:
        case InternalFormat::R16:
        case InternalFormat::R16_SNORM:
        case InternalFormat::RG8:
        case InternalFormat::RG8_SNORM:
        case InternalFormat::RG16:
        case InternalFormat::RG16_SNORM:
        case InternalFormat::RGB5:
        case InternalFormat::RGB565:
        case InternalFormat::RGBA4:
        case InternalFormat::RGB5_A1:
        case InternalFormat::R16F:
        case InternalFormat::R16I:
        case InternalFormat::R16UI:
        case InternalFormat::RG8I:
        case InternalFormat::RG8UI:
        case InternalFormat::DEPTH_COMPONENT16:
            return 16;

        case InternalFormat::DEPTH_COMPONENT24:
        case InternalFormat::SRGB8:
        case InternalFormat::RGB8:
        case InternalFormat::RGB8_SNORM:
        case InternalFormat::RGB8I:
        case InternalFormat::RGB8UI:
            return 24;

        case InternalFormat::R32F:
        case InternalFormat::RG16F:
        case InternalFormat::SRGB8_ALPHA8:
        case InternalFormat::RGBA8:
        case InternalFormat::RGBA8_SNORM:
        case InternalFormat::RGB10_A2:
        case InternalFormat::RGB10_A2UI:
        case InternalFormat::RGB10:  // TODO: check if this is really the case
        case InternalFormat::R11F_G11F_B10F:
        case InternalFormat::RGB9_E5:
        case InternalFormat::R32I:
        case InternalFormat::R32UI:
        case InternalFormat::RG16I:
        case InternalFormat::RG16UI:
        case InternalFormat::RGBA8I:
        case InternalFormat::RGBA8UI:
        case InternalFormat::DEPTH_COMPONENT32:
        case InternalFormat::DEPTH_COMPONENT32F:
        case InternalFormat::DEPTH24_STENCIL8:
            return 32;

        case InternalFormat::RGB12:  // TODO: check if this is really the case
        case InternalFormat::RGB16F:
        case InternalFormat::RGB16:
        case InternalFormat::RGB16_SNORM:
        case InternalFormat::RGBA12:
        case InternalFormat::RGB16I:
        case InternalFormat::RGB16UI:
            return 48;

        case InternalFormat::RGBA16:
        case InternalFormat::RGBA16_SNORM:
        case InternalFormat::RGBA16F:
        case InternalFormat::RG32F:
        case InternalFormat::RG32I:
        case InternalFormat::RG32UI:
        case InternalFormat::RGBA16I:
        case InternalFormat::RGBA16UI:
            return 64;

        case InternalFormat::RGB32F:
        case InternalFormat::RGB32I:
        case InternalFormat::RGB32UI:
            return 96;

        case InternalFormat::RGBA32F:
        case InternalFormat::RGBA32I:
        case InternalFormat::RGBA32UI:
            return 128;

        case InternalFormat::DEPTH32F_STENCIL8: // TODO : check if this true
            return 40;

        default:
            throw std::runtime_error("Bad format");
    }
}

template <uint32_t ALIGNMENT>
inline uint32_t evalAlignedCompressedBlockCount(uint32_t value) {
    enum
    {
        val = ALIGNMENT && !(ALIGNMENT & (ALIGNMENT - 1))
    };
    static_assert(val, "template parameter ALIGNMENT must be a power of 2");
    // FIXME add static assert that ALIGNMENT is a power of 2
    static uint32_t ALIGNMENT_REMAINDER = ALIGNMENT - 1;
    return (value + ALIGNMENT_REMAINDER) / ALIGNMENT;
}

inline uint32_t evalCompressedBlockCount(InternalFormat format, uint32_t value) {
    switch (format) {
        case InternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT:        // BC1
        case InternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:  // BC1A
        case InternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:  // BC3
        case InternalFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case InternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case InternalFormat::COMPRESSED_RGB_S3TC_DXT1_EXT:
        case InternalFormat::COMPRESSED_RED_RGTC1:                // BC4
        case InternalFormat::COMPRESSED_RG_RGTC2:                 // BC5
        case InternalFormat::COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:  // BC6
        case InternalFormat::COMPRESSED_SRGB_ALPHA_BPTC_UNORM:    // BC7
        // ETC2 / EAC
        case InternalFormat::COMPRESSED_RGB8_ETC2:
        case InternalFormat::COMPRESSED_SRGB8_ETC2:
        case InternalFormat::COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case InternalFormat::COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case InternalFormat::COMPRESSED_RGBA8_ETC2_EAC:
        case InternalFormat::COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
        case InternalFormat::COMPRESSED_R11_EAC:
        case InternalFormat::COMPRESSED_SIGNED_R11_EAC:
        case InternalFormat::COMPRESSED_RG11_EAC:
        case InternalFormat::COMPRESSED_SIGNED_RG11_EAC:
            return evalAlignedCompressedBlockCount<4>(value);

        default:
            throw std::runtime_error("Unknown format");
    }
}

inline uint8_t evalCompressedBlockSize(InternalFormat format) {
    switch (format) {
        case InternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT:
        case InternalFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case InternalFormat::COMPRESSED_RGB_S3TC_DXT1_EXT:
        case InternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        case InternalFormat::COMPRESSED_RED_RGTC1:
        case InternalFormat::COMPRESSED_RGB8_ETC2:
        case InternalFormat::COMPRESSED_SRGB8_ETC2:
        case InternalFormat::COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case InternalFormat::COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case InternalFormat::COMPRESSED_R11_EAC:
        case InternalFormat::COMPRESSED_SIGNED_R11_EAC:
            return 8;

        case InternalFormat::COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
        case InternalFormat::COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
        case InternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
        case InternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case InternalFormat::COMPRESSED_RG_RGTC2:
        case InternalFormat::COMPRESSED_RGBA8_ETC2_EAC:
        case InternalFormat::COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
        case InternalFormat::COMPRESSED_RG11_EAC:
        case InternalFormat::COMPRESSED_SIGNED_RG11_EAC:
            return 16;

        default:
            return 0;
    }
}

inline uint8_t evalCompressedBlockBitSize(InternalFormat format) {
    return evalCompressedBlockSize(format) * 8;
}

enum class BaseInternalFormat : uint32_t
{
    // GL 4.4 Table 8.11
    DEPTH_COMPONENT = 0x1902,
    DEPTH_STENCIL = 0x84F9,
    LUMINANCE = 0x1909,
    RED = 0x1903,
    RG = 0x8227,
    RGB = 0x1907,
    RGBA = 0x1908,
    SRGB = 0x8C40,
    SRGB_ALPHA = 0x8C42,
    STENCIL_INDEX = 0x1901,
};

inline uint8_t evalComponentCount(BaseInternalFormat format) {
    switch (format) {
        case BaseInternalFormat::DEPTH_COMPONENT:
        case BaseInternalFormat::STENCIL_INDEX:
        case BaseInternalFormat::RED:
        case BaseInternalFormat::LUMINANCE:
            return 1;
        case BaseInternalFormat::DEPTH_STENCIL:
        case BaseInternalFormat::RG:
            return 2;
        case BaseInternalFormat::RGB:
            return 3;

        case BaseInternalFormat::RGBA:
            return 4;

    }

    return 0;
}

namespace cubemap {

enum Constants
{
    NUM_CUBEMAPFACES = 6,
};

enum class Face
{
    POSITIVE_X = 0x8515,
    NEGATIVE_X = 0x8516,
    POSITIVE_Y = 0x8517,
    NEGATIVE_Y = 0x8518,
    POSITIVE_Z = 0x8519,
    NEGATIVE_Z = 0x851A,
};

}  // namespace cubemap

}  // namespace texture

}}  // namespace khronos::gl

#endif  // khronos_khr_hpp
