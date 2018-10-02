//
//  Created by Bradley Austin Davis on 2017/05/13
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef khrpp_constants_hpp
#define khrpp_constants_hpp

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace khrpp {

using Byte = uint8_t;
using Bytes = std::vector<uint8_t>;
using String = std::string;

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

        case InternalFormat::DEPTH32F_STENCIL8:  // TODO : check if this true
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
        case BaseInternalFormat::SRGB:
        case BaseInternalFormat::RGB:
            return 3;
        case BaseInternalFormat::SRGB_ALPHA:
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

}  // namespace gl

namespace vk {

enum class Format : uint32_t
{
    UNDEFINED = 0,
    R4G4_UNORM_PACK8 = 1,
    R4G4B4A4_UNORM_PACK16 = 2,
    B4G4R4A4_UNORM_PACK16 = 3,
    R5G6B5_UNORM_PACK16 = 4,
    B5G6R5_UNORM_PACK16 = 5,
    R5G5B5A1_UNORM_PACK16 = 6,
    B5G5R5A1_UNORM_PACK16 = 7,
    A1R5G5B5_UNORM_PACK16 = 8,
    R8_UNORM = 9,
    R8_SNORM = 10,
    R8_USCALED = 11,
    R8_SSCALED = 12,
    R8_UINT = 13,
    R8_SINT = 14,
    R8_SRGB = 15,
    R8G8_UNORM = 16,
    R8G8_SNORM = 17,
    R8G8_USCALED = 18,
    R8G8_SSCALED = 19,
    R8G8_UINT = 20,
    R8G8_SINT = 21,
    R8G8_SRGB = 22,
    R8G8B8_UNORM = 23,
    R8G8B8_SNORM = 24,
    R8G8B8_USCALED = 25,
    R8G8B8_SSCALED = 26,
    R8G8B8_UINT = 27,
    R8G8B8_SINT = 28,
    R8G8B8_SRGB = 29,
    B8G8R8_UNORM = 30,
    B8G8R8_SNORM = 31,
    B8G8R8_USCALED = 32,
    B8G8R8_SSCALED = 33,
    B8G8R8_UINT = 34,
    B8G8R8_SINT = 35,
    B8G8R8_SRGB = 36,
    R8G8B8A8_UNORM = 37,
    R8G8B8A8_SNORM = 38,
    R8G8B8A8_USCALED = 39,
    R8G8B8A8_SSCALED = 40,
    R8G8B8A8_UINT = 41,
    R8G8B8A8_SINT = 42,
    R8G8B8A8_SRGB = 43,
    B8G8R8A8_UNORM = 44,
    B8G8R8A8_SNORM = 45,
    B8G8R8A8_USCALED = 46,
    B8G8R8A8_SSCALED = 47,
    B8G8R8A8_UINT = 48,
    B8G8R8A8_SINT = 49,
    B8G8R8A8_SRGB = 50,
    A8B8G8R8_UNORM_PACK32 = 51,
    A8B8G8R8_SNORM_PACK32 = 52,
    A8B8G8R8_USCALED_PACK32 = 53,
    A8B8G8R8_SSCALED_PACK32 = 54,
    A8B8G8R8_UINT_PACK32 = 55,
    A8B8G8R8_SINT_PACK32 = 56,
    A8B8G8R8_SRGB_PACK32 = 57,
    A2R10G10B10_UNORM_PACK32 = 58,
    A2R10G10B10_SNORM_PACK32 = 59,
    A2R10G10B10_USCALED_PACK32 = 60,
    A2R10G10B10_SSCALED_PACK32 = 61,
    A2R10G10B10_UINT_PACK32 = 62,
    A2R10G10B10_SINT_PACK32 = 63,
    A2B10G10R10_UNORM_PACK32 = 64,
    A2B10G10R10_SNORM_PACK32 = 65,
    A2B10G10R10_USCALED_PACK32 = 66,
    A2B10G10R10_SSCALED_PACK32 = 67,
    A2B10G10R10_UINT_PACK32 = 68,
    A2B10G10R10_SINT_PACK32 = 69,
    R16_UNORM = 70,
    R16_SNORM = 71,
    R16_USCALED = 72,
    R16_SSCALED = 73,
    R16_UINT = 74,
    R16_SINT = 75,
    R16_SFLOAT = 76,
    R16G16_UNORM = 77,
    R16G16_SNORM = 78,
    R16G16_USCALED = 79,
    R16G16_SSCALED = 80,
    R16G16_UINT = 81,
    R16G16_SINT = 82,
    R16G16_SFLOAT = 83,
    R16G16B16_UNORM = 84,
    R16G16B16_SNORM = 85,
    R16G16B16_USCALED = 86,
    R16G16B16_SSCALED = 87,
    R16G16B16_UINT = 88,
    R16G16B16_SINT = 89,
    R16G16B16_SFLOAT = 90,
    R16G16B16A16_UNORM = 91,
    R16G16B16A16_SNORM = 92,
    R16G16B16A16_USCALED = 93,
    R16G16B16A16_SSCALED = 94,
    R16G16B16A16_UINT = 95,
    R16G16B16A16_SINT = 96,
    R16G16B16A16_SFLOAT = 97,
    R32_UINT = 98,
    R32_SINT = 99,
    R32_SFLOAT = 100,
    R32G32_UINT = 101,
    R32G32_SINT = 102,
    R32G32_SFLOAT = 103,
    R32G32B32_UINT = 104,
    R32G32B32_SINT = 105,
    R32G32B32_SFLOAT = 106,
    R32G32B32A32_UINT = 107,
    R32G32B32A32_SINT = 108,
    R32G32B32A32_SFLOAT = 109,
    R64_UINT = 110,
    R64_SINT = 111,
    R64_SFLOAT = 112,
    R64G64_UINT = 113,
    R64G64_SINT = 114,
    R64G64_SFLOAT = 115,
    R64G64B64_UINT = 116,
    R64G64B64_SINT = 117,
    R64G64B64_SFLOAT = 118,
    R64G64B64A64_UINT = 119,
    R64G64B64A64_SINT = 120,
    R64G64B64A64_SFLOAT = 121,
    B10G11R11_UFLOAT_PACK32 = 122,
    E5B9G9R9_UFLOAT_PACK32 = 123,
    D16_UNORM = 124,
    X8_D24_UNORM_PACK32 = 125,
    D32_SFLOAT = 126,
    S8_UINT = 127,
    D16_UNORM_S8_UINT = 128,
    D24_UNORM_S8_UINT = 129,
    D32_SFLOAT_S8_UINT = 130,
    BC1_RGB_UNORM_BLOCK = 131,
    BC1_RGB_SRGB_BLOCK = 132,
    BC1_RGBA_UNORM_BLOCK = 133,
    BC1_RGBA_SRGB_BLOCK = 134,
    BC2_UNORM_BLOCK = 135,
    BC2_SRGB_BLOCK = 136,
    BC3_UNORM_BLOCK = 137,
    BC3_SRGB_BLOCK = 138,
    BC4_UNORM_BLOCK = 139,
    BC4_SNORM_BLOCK = 140,
    BC5_UNORM_BLOCK = 141,
    BC5_SNORM_BLOCK = 142,
    BC6H_UFLOAT_BLOCK = 143,
    BC6H_SFLOAT_BLOCK = 144,
    BC7_UNORM_BLOCK = 145,
    BC7_SRGB_BLOCK = 146,
    ETC2_R8G8B8_UNORM_BLOCK = 147,
    ETC2_R8G8B8_SRGB_BLOCK = 148,
    ETC2_R8G8B8A1_UNORM_BLOCK = 149,
    ETC2_R8G8B8A1_SRGB_BLOCK = 150,
    ETC2_R8G8B8A8_UNORM_BLOCK = 151,
    ETC2_R8G8B8A8_SRGB_BLOCK = 152,
    EAC_R11_UNORM_BLOCK = 153,
    EAC_R11_SNORM_BLOCK = 154,
    EAC_R11G11_UNORM_BLOCK = 155,
    EAC_R11G11_SNORM_BLOCK = 156,
    ASTC_4x4_UNORM_BLOCK = 157,
    ASTC_4x4_SRGB_BLOCK = 158,
    ASTC_5x4_UNORM_BLOCK = 159,
    ASTC_5x4_SRGB_BLOCK = 160,
    ASTC_5x5_UNORM_BLOCK = 161,
    ASTC_5x5_SRGB_BLOCK = 162,
    ASTC_6x5_UNORM_BLOCK = 163,
    ASTC_6x5_SRGB_BLOCK = 164,
    ASTC_6x6_UNORM_BLOCK = 165,
    ASTC_6x6_SRGB_BLOCK = 166,
    ASTC_8x5_UNORM_BLOCK = 167,
    ASTC_8x5_SRGB_BLOCK = 168,
    ASTC_8x6_UNORM_BLOCK = 169,
    ASTC_8x6_SRGB_BLOCK = 170,
    ASTC_8x8_UNORM_BLOCK = 171,
    ASTC_8x8_SRGB_BLOCK = 172,
    ASTC_10x5_UNORM_BLOCK = 173,
    ASTC_10x5_SRGB_BLOCK = 174,
    ASTC_10x6_UNORM_BLOCK = 175,
    ASTC_10x6_SRGB_BLOCK = 176,
    ASTC_10x8_UNORM_BLOCK = 177,
    ASTC_10x8_SRGB_BLOCK = 178,
    ASTC_10x10_UNORM_BLOCK = 179,
    ASTC_10x10_SRGB_BLOCK = 180,
    ASTC_12x10_UNORM_BLOCK = 181,
    ASTC_12x10_SRGB_BLOCK = 182,
    ASTC_12x12_UNORM_BLOCK = 183,
    ASTC_12x12_SRGB_BLOCK = 184,
    G8B8G8R8_422_UNORM = 1000156000,
    B8G8R8G8_422_UNORM = 1000156001,
    G8_B8_R8_3PLANE_420_UNORM = 1000156002,
    G8_B8R8_2PLANE_420_UNORM = 1000156003,
    G8_B8_R8_3PLANE_422_UNORM = 1000156004,
    G8_B8R8_2PLANE_422_UNORM = 1000156005,
    G8_B8_R8_3PLANE_444_UNORM = 1000156006,
    R10X6_UNORM_PACK16 = 1000156007,
    R10X6G10X6_UNORM_2PACK16 = 1000156008,
    R10X6G10X6B10X6A10X6_UNORM_4PACK16 = 1000156009,
    G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 = 1000156010,
    B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 = 1000156011,
    G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 = 1000156012,
    G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 = 1000156013,
    G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 = 1000156014,
    G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 = 1000156015,
    G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 = 1000156016,
    R12X4_UNORM_PACK16 = 1000156017,
    R12X4G12X4_UNORM_2PACK16 = 1000156018,
    R12X4G12X4B12X4A12X4_UNORM_4PACK16 = 1000156019,
    G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 = 1000156020,
    B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 = 1000156021,
    G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 = 1000156022,
    G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 = 1000156023,
    G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 = 1000156024,
    G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 = 1000156025,
    G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 = 1000156026,
    G16B16G16R16_422_UNORM = 1000156027,
    B16G16R16G16_422_UNORM = 1000156028,
    G16_B16_R16_3PLANE_420_UNORM = 1000156029,
    G16_B16R16_2PLANE_420_UNORM = 1000156030,
    G16_B16_R16_3PLANE_422_UNORM = 1000156031,
    G16_B16R16_2PLANE_422_UNORM = 1000156032,
    G16_B16_R16_3PLANE_444_UNORM = 1000156033,
    PVRTC1_2BPP_UNORM_BLOCK_IMG = 1000054000,
    PVRTC1_4BPP_UNORM_BLOCK_IMG = 1000054001,
    PVRTC2_2BPP_UNORM_BLOCK_IMG = 1000054002,
    PVRTC2_4BPP_UNORM_BLOCK_IMG = 1000054003,
    PVRTC1_2BPP_SRGB_BLOCK_IMG = 1000054004,
    PVRTC1_4BPP_SRGB_BLOCK_IMG = 1000054005,
    PVRTC2_2BPP_SRGB_BLOCK_IMG = 1000054006,
    PVRTC2_4BPP_SRGB_BLOCK_IMG = 1000054007,
    ASTC_4x4_SFLOAT_BLOCK_EXT = 1000066000,
    ASTC_5x4_SFLOAT_BLOCK_EXT = 1000066001,
    ASTC_5x5_SFLOAT_BLOCK_EXT = 1000066002,
    ASTC_6x5_SFLOAT_BLOCK_EXT = 1000066003,
    ASTC_6x6_SFLOAT_BLOCK_EXT = 1000066004,
    ASTC_8x5_SFLOAT_BLOCK_EXT = 1000066005,
    ASTC_8x6_SFLOAT_BLOCK_EXT = 1000066006,
    ASTC_8x8_SFLOAT_BLOCK_EXT = 1000066007,
    ASTC_10x5_SFLOAT_BLOCK_EXT = 1000066008,
    ASTC_10x6_SFLOAT_BLOCK_EXT = 1000066009,
    ASTC_10x8_SFLOAT_BLOCK_EXT = 1000066010,
    ASTC_10x10_SFLOAT_BLOCK_EXT = 1000066011,
    ASTC_12x10_SFLOAT_BLOCK_EXT = 1000066012,
    ASTC_12x12_SFLOAT_BLOCK_EXT = 1000066013,
};

}  // namespace vk

}  // namespace khrpp

#endif  // khronos_khr_hpp
