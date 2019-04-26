//
//  KTX.h
//  ktx/src/ktx
//
//  Created by Zach Pomerantz on 2/08/2017.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#include <array>
#include <list>
#include <vector>
#include <cstdint>
#include <cstring>
#include <string>
#include <memory>
#include <cassert>
#include <algorithm>
#include <unordered_set>

#include "../constants.hpp"
#include "../utils/storage.hpp"

namespace khronos { namespace ktx {

/*

    KTX Specification: https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/


    **** A KTX header is 64 bytes layed out as follows

    Byte[12] identifier
    UInt32 endianness
    UInt32 glType
    UInt32 glTypeSize
    UInt32 glFormat
    Uint32 glInternalFormat
    Uint32 glBaseInternalFormat
    UInt32 pixelWidth
    UInt32 pixelHeight
    UInt32 pixelDepth
    UInt32 numberOfArrayElements
    UInt32 numberOfFaces
    UInt32 numberOfMipmapLevels
    UInt32 bytesOfKeyValueData

    **** Each KTX key value pair block is 4 byte aligned

    for each keyValuePair that fits in bytesOfKeyValueData
        UInt32   keyAndValueByteSize
        Byte     keyAndValue[keyAndValueByteSize]
        Byte     valuePadding[3 - ((keyAndValueByteSize + 3) % 4)]
    end

    **** Each mip and cube face is 4 byte aligned

    for each mipmap_level in numberOfMipmapLevels*
        UInt32 imageSize;
        for each array_element in numberOfArrayElements*
           for each face in numberOfFaces
               for each z_slice in pixelDepth*
                   for each row or row_of_blocks in pixelHeight*
                       for each pixel or block_of_pixels in pixelWidth
                           Byte data[format-specific-number-of-bytes]**
                       end
                   end
               end
               Byte cubePadding[0-3]
           end
        end
        Byte mipPadding[3 - ((imageSize + 3) % 4)]
    end

    * Replace with 1 if this field is 0.

    ** Uncompressed texture data matches a GL_UNPACK_ALIGNMENT of 4.
    */

// Alignment constants
static const uint32_t ALIGNMENT{ sizeof(uint32_t) };
static const uint32_t ALIGNMENT_REMAINDER{ ALIGNMENT - 1 };
static const uint32_t NUM_CUBEMAPFACES = khronos::gl::texture::cubemap::NUM_CUBEMAPFACES;

using Byte = uint8_t;

using GLType = khronos::gl::Type;
using GLFormat = khronos::gl::texture::Format;
using GLInternalFormat = khronos::gl::texture::InternalFormat;
using GLBaseInternalFormat = khronos::gl::texture::BaseInternalFormat;

using Storage = khr::utils::Storage;
using StoragePtr = Storage::ConstPointer;

struct ImageDescriptor;
using ImageDescriptors = std::vector<ImageDescriptor>;

// Returns the number of bytes required be added to the passed value to make it 4 byte aligned
template <typename T>
inline uint8_t evalPadding(T value) {
    return ALIGNMENT_REMAINDER - ((value + ALIGNMENT_REMAINDER) % ALIGNMENT);
}

// Returns the passed value rounded up to the next 4 byte aligned value, if it's not already 4 byte aligned
template <typename T>
inline T evalPaddedSize(T value) {
    return (value + ALIGNMENT_REMAINDER) & ~(T)ALIGNMENT_REMAINDER;
}

template <typename T>
inline T evalAlignedCount(T value) {
    return (value + ALIGNMENT_REMAINDER) / ALIGNMENT;
}

template <typename T>
inline bool checkAlignment(T value) {
    return ((value & ALIGNMENT_REMAINDER) == 0);
}

// Header
struct Header {
    static const uint32_t COMPRESSED_TYPE_SIZE{ 1 };
    static const size_t IDENTIFIER_LENGTH{ 12 };
    using Identifier = std::array<uint8_t, IDENTIFIER_LENGTH>;

    static const Identifier& IDENTIFIER() {
        static const Identifier IDENTIFIER_VALUE{ { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A } };
        return IDENTIFIER_VALUE;
   };

    static const uint32_t ENDIAN_TEST = 0x04030201;
    static const uint32_t REVERSE_ENDIAN_TEST = 0x01020304;

    Header();

    Byte identifier[IDENTIFIER_LENGTH];
    uint32_t endianness{ ENDIAN_TEST };

    GLType glType{ GLType::UNSIGNED_BYTE };
    uint32_t glTypeSize{ 0 };
    GLFormat glFormat{ GLFormat::RGBA };
    GLInternalFormat glInternalFormat{ GLInternalFormat::RGBA8 };
    GLBaseInternalFormat glBaseInternalFormat{ GLBaseInternalFormat::RGBA };

    uint32_t pixelWidth{ 1 };
    uint32_t pixelHeight{ 1 };
    uint32_t pixelDepth{ 0 };
    uint32_t numberOfArrayElements{ 0 };
    uint32_t numberOfFaces{ 1 };
    uint32_t numberOfMipmapLevels{ 1 };

    uint32_t bytesOfKeyValueData{ 0 };

    uint32_t getPixelWidth() const { return (pixelWidth ? pixelWidth : 1); }
    uint32_t getPixelHeight() const { return (pixelHeight ? pixelHeight : 1); }
    uint32_t getPixelDepth() const { return (pixelDepth ? pixelDepth : 1); }
    uint32_t getNumberOfSlices() const { return (numberOfArrayElements ? numberOfArrayElements : 1); }
    uint32_t getNumberOfLevels() const { return (numberOfMipmapLevels ? numberOfMipmapLevels : 1); }
    bool isArray() const { return numberOfArrayElements > 0; }
    bool isCompressed() const { return glFormat == GLFormat::COMPRESSED; }

    uint32_t evalMaxDimension() const;
    uint32_t evalPixelOrBlockWidth(uint32_t level) const;
    uint32_t evalPixelOrBlockHeight(uint32_t level) const;
    uint32_t evalPixelOrBlockDepth(uint32_t level) const;

    size_t evalPixelOrBlockBitSize() const;
    size_t evalRowSize(uint32_t level) const;
    size_t evalUnalignedRowSize(uint32_t level) const;
    size_t evalFaceSize(uint32_t level) const;
    size_t evalUnalignedFaceSize(uint32_t level) const;
    size_t evalImageSize(uint32_t level) const;

    GLType getGLType() const { return glType; }
    uint32_t getTypeSize() const { return glTypeSize; }
    GLFormat getGLFormat() const { return (GLFormat)glFormat; }
    GLInternalFormat getGLInternaFormat() const { return (GLInternalFormat)glInternalFormat; }
    GLBaseInternalFormat getGLBaseInternalFormat() const { return (GLBaseInternalFormat)glBaseInternalFormat; }

    bool isValid() const;

private:
    uint32_t evalPixelOrBlockDimension(uint32_t pixelDimension) const;
    uint32_t evalMipPixelOrBlockDimension(uint32_t level, uint32_t pixelDimension) const;
    static inline uint32_t evalMipDimension(uint32_t mipLevel, uint32_t pixelDimension) { return std::max(pixelDimension >> mipLevel, 1U); }
};

// Size as specified by the KTX specification
static const size_t KTX_HEADER_SIZE{ 64 };
static_assert(sizeof(Header) == KTX_HEADER_SIZE, "KTX Header size is static and should not change from the spec");
static const size_t KV_SIZE_WIDTH{ ALIGNMENT };     // Number of bytes for keyAndValueByteSize
static const size_t IMAGE_SIZE_WIDTH{ ALIGNMENT };  // Number of bytes for imageSize

// Key Values
struct KeyValue {
    using List = std::list<KeyValue>;
    uint32_t _byteSize{ 0 };
    std::string _key;
    std::vector<Byte> _value;

    KeyValue(const std::string& key, uint32_t valueByteSize, const Byte* value);

    KeyValue(const std::string& key, const std::string& value);

    uint32_t serializedByteSize() const;

    static KeyValue parseSerializedKeyAndValue(uint32_t srcSize, const Byte* srcBytes);
    static uint32_t writeSerializedKeyAndValue(Byte* destBytes, uint32_t destByteSize, const KeyValue& keyval);
    static uint32_t serializedKeyValuesByteSize(const List& keyValues);
};

using KeyValues = KeyValue::List;

struct ImageHeader {
    using Offsets = std::vector<size_t>;

    const uint32_t _numFaces;
    // This is the byte offset from the _start_ of the image region. For example, level 0
    // will have a byte offset of 0.
    const size_t _imageOffset;
    const uint32_t _imageSize;
    const uint32_t _faceSize;
    const uint32_t _padding;

    ImageHeader(bool cube, size_t imageOffset, uint32_t imageSize, uint32_t padding)
        : _numFaces(cube ? NUM_CUBEMAPFACES : 1)
        , _imageOffset(imageOffset)
        , _imageSize(imageSize * _numFaces)
        , _faceSize(imageSize)
        , _padding(padding) {}
};

// Image without the image data itself
struct ImageDescriptor : public ImageHeader {
    const Offsets _faceOffsets;
    ImageDescriptor(const ImageHeader& header, const Offsets& offsets)
        : ImageHeader(header)
        , _faceOffsets(offsets) {}
};


// A KTX descriptor is a lightweight container for all the information about a serialized KTX file, but without the
// actual image / face data available.
struct KTXDescriptor {
    static bool validate(const StoragePtr& src);
    static std::shared_ptr<const KTXDescriptor> read(const StoragePtr& src);

    KTXDescriptor(const Header& header, const ImageDescriptors& imageDescriptors, const KeyValues& keyValues = {})
        : header(header)
        , keyValues(keyValues)
        , images(imageDescriptors) {}
    const Header header;
    const KeyValues keyValues;
    const ImageDescriptors images;
    size_t getMipFaceTexelsSize(uint16_t mip = 0, uint8_t face = 0) const;
    size_t getMipFaceTexelsOffset(uint16_t mip = 0, uint8_t face = 0) const;
    size_t getValueOffsetForKey(const std::string& key) const;
    size_t getImagesOffset() const;
    size_t evalStorageSize() const;
    bool isValid() const;
};

}}  // namespace khronos::ktx


namespace khronos { namespace ktx {

inline Header::Header() {
}

inline uint32_t Header::evalMaxDimension() const {
    return std::max(getPixelWidth(), std::max(getPixelHeight(), getPixelDepth()));
}

inline uint32_t Header::evalPixelOrBlockDimension(uint32_t pixelDimension) const {
    if (isCompressed()) {
        return khronos::gl::texture::evalCompressedBlockCount(getGLInternaFormat(), pixelDimension);
    }
    return pixelDimension;
}

inline uint32_t Header::evalMipPixelOrBlockDimension(uint32_t mipLevel, uint32_t pixelDimension) const {
    uint32_t mipPixelDimension = evalMipDimension(mipLevel, pixelDimension);
    return evalPixelOrBlockDimension(mipPixelDimension);
}

inline uint32_t Header::evalPixelOrBlockWidth(uint32_t level) const {
    return evalMipPixelOrBlockDimension(level, getPixelWidth());
}

inline uint32_t Header::evalPixelOrBlockHeight(uint32_t level) const {
    return evalMipPixelOrBlockDimension(level, getPixelHeight());
}

inline uint32_t Header::evalPixelOrBlockDepth(uint32_t level) const {
    return evalMipDimension(level, getPixelDepth());
}

inline size_t Header::evalPixelOrBlockBitSize() const {
    size_t result = 0;
    auto format = getGLInternaFormat();
    if (isCompressed()) {
        result = khronos::gl::texture::evalCompressedBlockBitSize(format);
    } else {
        result = khronos::gl::texture::evalUncompressedBlockBitSize(format);
    }

    if (0 == result) {
        throw std::runtime_error("Unknown ktx format");
    }
    return result;
}

inline size_t Header::evalUnalignedRowSize(uint32_t level) const {
    auto pixWidth = evalPixelOrBlockWidth(level);
    auto pixSize = evalPixelOrBlockBitSize();
    if (pixSize == 0) {
        return 0;
    }
    auto totalByteSize = pixWidth * pixSize;
    // Round to the nearest upper byte size
    return totalByteSize = (totalByteSize / 8) + (((totalByteSize % 8) != 0) & 1);
}

inline size_t Header::evalRowSize(uint32_t level) const {
    return evalPaddedSize(evalUnalignedRowSize(level));
}

inline size_t Header::evalUnalignedFaceSize(uint32_t level) const {
    auto pixHeight = evalPixelOrBlockHeight(level);
    auto pixDepth = evalPixelOrBlockDepth(level);
    auto rowSize = evalUnalignedRowSize(level);
    return pixDepth * pixHeight * rowSize;
}

inline size_t Header::evalFaceSize(uint32_t level) const {
    auto pixHeight = evalPixelOrBlockHeight(level);
    auto pixDepth = evalPixelOrBlockDepth(level);
    auto rowSize = evalRowSize(level);
    return pixDepth * pixHeight * rowSize;
}

inline size_t Header::evalImageSize(uint32_t level) const {
    auto faceSize = evalFaceSize(level);
    if (!checkAlignment(faceSize)) {
        return 0;
    }
    if (numberOfFaces == NUM_CUBEMAPFACES && numberOfArrayElements == 0) {
        return faceSize;
    } else {
        return (getNumberOfSlices() * numberOfFaces * faceSize);
    }
}

inline size_t KTXDescriptor::getValueOffsetForKey(const std::string& key) const {
    size_t offset{ 0 };
    for (auto& kv : keyValues) {
        if (kv._key == key) {
            return offset + ktx::KV_SIZE_WIDTH + kv._key.size() + 1;
        }
        offset += kv.serializedByteSize();
    }
    return 0;
}

inline size_t KTXDescriptor::getImagesOffset() const {
    size_t offset = sizeof(Header);

    if (!keyValues.empty()) {
        size_t keyValuesSize = KeyValue::serializedKeyValuesByteSize(keyValues);
        offset += keyValuesSize;
    }

    return offset;
}

inline size_t KTXDescriptor::evalStorageSize() const {
    size_t storageSize = sizeof(Header);

    if (!keyValues.empty()) {
        size_t keyValuesSize = KeyValue::serializedKeyValuesByteSize(keyValues);
        storageSize += keyValuesSize;
    }

    auto numMips = header.getNumberOfLevels();
    for (uint32_t l = 0; l < numMips; l++) {
        if (images.size() > l) {
            storageSize += sizeof(uint32_t);
            storageSize += evalPaddedSize(images[l]._imageSize);
        }
    }
    return storageSize;
}


inline KeyValue::KeyValue(const std::string& key, uint32_t valueByteSize, const Byte* value)
    : _byteSize((uint32_t)key.size() + 1 + valueByteSize)
    ,  // keyString size + '\0' ending char + the value size
    _key(key)
    , _value(valueByteSize) {
    if (_value.size() && value) {
        memcpy(_value.data(), value, valueByteSize);
    }
}

inline KeyValue::KeyValue(const std::string& key, const std::string& value)
    : KeyValue(key, (uint32_t)value.size(), (const Byte*)value.data()) {
}

inline uint32_t KeyValue::serializedByteSize() const {
    return (uint32_t)sizeof(uint32_t) + evalPaddedSize(_byteSize);
}

inline uint32_t KeyValue::serializedKeyValuesByteSize(const KeyValues& keyValues) {
    uint32_t keyValuesSize = 0;
    for (auto& keyval : keyValues) {
        keyValuesSize += keyval.serializedByteSize();
    }
    assert(keyValuesSize % 4 == 0);
    return keyValuesSize;
}

static const std::unordered_set<GLType> VALID_GL_TYPES {
    GLType::UNSIGNED_BYTE,
    GLType::BYTE,
    GLType::UNSIGNED_SHORT,
    GLType::SHORT,
    GLType::UNSIGNED_INT,
    GLType::INT,
    GLType::HALF_FLOAT,
    GLType::FLOAT,
    GLType::UNSIGNED_BYTE_3_3_2,
    GLType::UNSIGNED_BYTE_2_3_3_REV,
    GLType::UNSIGNED_SHORT_5_6_5,
    GLType::UNSIGNED_SHORT_5_6_5_REV,
    GLType::UNSIGNED_SHORT_4_4_4_4,
    GLType::UNSIGNED_SHORT_4_4_4_4_REV,
    GLType::UNSIGNED_SHORT_5_5_5_1,
    GLType::UNSIGNED_SHORT_1_5_5_5_REV,
    GLType::UNSIGNED_INT_8_8_8_8,
    GLType::UNSIGNED_INT_8_8_8_8_REV,
    GLType::UNSIGNED_INT_10_10_10_2,
    GLType::UNSIGNED_INT_2_10_10_10_REV,
    GLType::UNSIGNED_INT_24_8,
    GLType::UNSIGNED_INT_10F_11F_11F_REV,
    GLType::UNSIGNED_INT_5_9_9_9_REV,
    GLType::FLOAT_32_UNSIGNED_INT_24_8_REV,
};

static const std::unordered_set<GLFormat> VALID_GL_FORMATS {
    GLFormat::STENCIL_INDEX,
    GLFormat::DEPTH_COMPONENT,
    GLFormat::DEPTH_STENCIL,
    GLFormat::LUMINANCE,
    GLFormat::RED,
    GLFormat::GREEN,
    GLFormat::BLUE,
    GLFormat::RG,
    GLFormat::RGB,
    GLFormat::RGBA,
    GLFormat::BGR,
    GLFormat::BGRA,
    GLFormat::RG_INTEGER,
    GLFormat::RED_INTEGER,
    GLFormat::GREEN_INTEGER,
    GLFormat::BLUE_INTEGER,
    GLFormat::RGB_INTEGER,
    GLFormat::RGBA_INTEGER,
    GLFormat::BGR_INTEGER,
    GLFormat::BGRA_INTEGER,
};

static const std::unordered_set<GLInternalFormat> VALID_GL_INTERNAL_FORMATS {
    GLInternalFormat::LUMINANCE8,
    GLInternalFormat::R8,
    GLInternalFormat::R8_SNORM,
    GLInternalFormat::R16,
    GLInternalFormat::R16_SNORM,
    GLInternalFormat::RG8,
    GLInternalFormat::RG8_SNORM,
    GLInternalFormat::RG16,
    GLInternalFormat::RG16_SNORM,
    GLInternalFormat::R3_G3_B2,
    GLInternalFormat::RGB4,
    GLInternalFormat::RGB5,
    GLInternalFormat::RGB565,
    GLInternalFormat::RGB8,
    GLInternalFormat::RGB8_SNORM,
    GLInternalFormat::RGB10,
    GLInternalFormat::RGB12,
    GLInternalFormat::RGB16,
    GLInternalFormat::RGB16_SNORM,
    GLInternalFormat::RGBA2,
    GLInternalFormat::RGBA4,
    GLInternalFormat::RGB5_A1,
    GLInternalFormat::RGBA8,
    GLInternalFormat::RGBA8_SNORM,
    GLInternalFormat::RGB10_A2,
    GLInternalFormat::RGB10_A2UI,
    GLInternalFormat::RGBA12,
    GLInternalFormat::RGBA16,
    GLInternalFormat::RGBA16_SNORM,
    GLInternalFormat::SRGB8,
    GLInternalFormat::SRGB8_ALPHA8,
    GLInternalFormat::R16F,
    GLInternalFormat::RG16F,
    GLInternalFormat::RGB16F,
    GLInternalFormat::RGBA16F,
    GLInternalFormat::R32F,
    GLInternalFormat::RG32F,
    GLInternalFormat::RGBA32F,
    GLInternalFormat::R11F_G11F_B10F,
    GLInternalFormat::RGB9_E5,
    GLInternalFormat::R8I,
    GLInternalFormat::R8UI,
    GLInternalFormat::R16I,
    GLInternalFormat::R16UI,
    GLInternalFormat::R32I,
    GLInternalFormat::R32UI,
    GLInternalFormat::RG8I,
    GLInternalFormat::RG8UI,
    GLInternalFormat::RG16I,
    GLInternalFormat::RG16UI,
    GLInternalFormat::RG32I,
    GLInternalFormat::RG32UI,
    GLInternalFormat::RGB8I,
    GLInternalFormat::RGB8UI,
    GLInternalFormat::RGB16I,
    GLInternalFormat::RGB16UI,
    GLInternalFormat::RGB32I,
    GLInternalFormat::RGB32UI,
    GLInternalFormat::RGBA8I,
    GLInternalFormat::RGBA8UI,
    GLInternalFormat::RGBA16I,
    GLInternalFormat::RGBA16UI,
    GLInternalFormat::RGBA32I,
    GLInternalFormat::RGBA32UI,
    GLInternalFormat::DEPTH_COMPONENT16,
    GLInternalFormat::DEPTH_COMPONENT24,
    GLInternalFormat::DEPTH_COMPONENT32,
    GLInternalFormat::DEPTH_COMPONENT32F,
    GLInternalFormat::DEPTH24_STENCIL8,
    GLInternalFormat::DEPTH32F_STENCIL8,
    GLInternalFormat::STENCIL_INDEX1,
    GLInternalFormat::STENCIL_INDEX4,
    GLInternalFormat::STENCIL_INDEX8,
    GLInternalFormat::STENCIL_INDEX16,
};

static const std::unordered_set<GLInternalFormat> VALID_GL_INTERNAL_COMPRESSED_FORMATS {
    GLInternalFormat::COMPRESSED_RED,
    GLInternalFormat::COMPRESSED_RG,
    GLInternalFormat::COMPRESSED_RGB,
    GLInternalFormat::COMPRESSED_RGBA,
    GLInternalFormat::COMPRESSED_SRGB,
    GLInternalFormat::COMPRESSED_SRGB_ALPHA,
    GLInternalFormat::COMPRESSED_ETC1_RGB8_OES,
    GLInternalFormat::COMPRESSED_RGB_S3TC_DXT1_EXT,
    GLInternalFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT,
    GLInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT,
    GLInternalFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT,
    GLInternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT,
    GLInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,
    GLInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,
    GLInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,
    GLInternalFormat::COMPRESSED_RED_RGTC1,
    GLInternalFormat::COMPRESSED_SIGNED_RED_RGTC1,
    GLInternalFormat::COMPRESSED_RG_RGTC2,
    GLInternalFormat::COMPRESSED_SIGNED_RG_RGTC2,
    GLInternalFormat::COMPRESSED_RGBA_BPTC_UNORM,
    GLInternalFormat::COMPRESSED_SRGB_ALPHA_BPTC_UNORM,
    GLInternalFormat::COMPRESSED_RGB_BPTC_SIGNED_FLOAT,
    GLInternalFormat::COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,
    GLInternalFormat::COMPRESSED_RGB8_ETC2,
    GLInternalFormat::COMPRESSED_SRGB8_ETC2,
    GLInternalFormat::COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,
    GLInternalFormat::COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,
    GLInternalFormat::COMPRESSED_RGBA8_ETC2_EAC,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,
    GLInternalFormat::COMPRESSED_R11_EAC,
    GLInternalFormat::COMPRESSED_SIGNED_R11_EAC,
    GLInternalFormat::COMPRESSED_RG11_EAC,
    GLInternalFormat::COMPRESSED_SIGNED_RG11_EAC,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_4x4,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_5x4,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_5x5,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_6x5,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_6x6,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_8x5,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_8x6,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_8x8,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_10x5,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_10x6,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_10x8,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_10x10,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_12x10,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_12x12,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_3x3x3_OES,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_4x3x3_OES,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_4x4x3_OES,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_4x4x4_OES,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_5x4x4_OES,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_5x5x4_OES,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_5x5x5_OES,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_6x5x5_OES,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_6x6x5_OES,
    GLInternalFormat::COMPRESSED_RGBA_ASTC_6x6x6_OES,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x4,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x5,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x5,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x6,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_8x8,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x5,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x6,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x8,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_10x10,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_12x10,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_12x12,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES,
    GLInternalFormat::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES,
};

static const std::unordered_set<GLBaseInternalFormat> VALID_GL_BASE_INTERNAL_FORMATS {
    GLBaseInternalFormat::DEPTH_COMPONENT,
    GLBaseInternalFormat::DEPTH_STENCIL,
    GLBaseInternalFormat::LUMINANCE,
    GLBaseInternalFormat::RED,
    GLBaseInternalFormat::RG,
    GLBaseInternalFormat::RGB,
    GLBaseInternalFormat::RGBA,
    GLBaseInternalFormat::SRGB,
    GLBaseInternalFormat::SRGB_ALPHA,
    GLBaseInternalFormat::STENCIL_INDEX,
};

inline bool Header::isValid() const {
    if (0 != memcmp(identifier, IDENTIFIER().data(), IDENTIFIER_LENGTH)) {
        //qDebug() << "Invalid header identifier";
        return false;
    }

    if (endianness != ENDIAN_TEST && endianness != REVERSE_ENDIAN_TEST) {
        //qDebug("Invalid endian marker 0x%x", endianness);
        return false;
    }

    //
    // GL enum validity
    //
    if (VALID_GL_BASE_INTERNAL_FORMATS.count(glBaseInternalFormat) != 1) {
        //qDebug("Invalid base internal format 0x%x", glBaseInternalFormat);
        return false;
    }

    if (isCompressed()) {
        if (glType != GLType::COMPRESSED) {
            //qDebug("Invalid type for compressed texture 0x%x", glType);
            return false;
        }

        if (glTypeSize != COMPRESSED_TYPE_SIZE) {
            //qDebug("Invalid type size for compressed texture %d", glTypeSize);
            return false;
        }

        if (VALID_GL_INTERNAL_COMPRESSED_FORMATS.count(glInternalFormat) != 1) {
            //qDebug("Invalid compressed internal format 0x%x", glInternalFormat);
            return false;
        }
    } else {
        if (VALID_GL_TYPES.count(glType) != 1) {
            //qDebug("Invalid type 0x%x", glType);
            return false;
        }

        if (VALID_GL_FORMATS.count(glFormat) != 1) {
            //qDebug("Invalid format 0x%x", glFormat);
            return false;
        }

        if (VALID_GL_INTERNAL_FORMATS.count(glInternalFormat) != 1) {
            //qDebug("Invalid internal format 0x%x", glInternalFormat);
            return false;
        }
    }

    //
    // Dimensions validity
    //

    // Textures must at least have a width
    // If they have a depth, they must have a height
    if ((pixelWidth == 0) || (pixelDepth != 0 && pixelHeight == 0)) {
        //qDebug() << "Invalid dimensions " << pixelWidth << "x" << pixelHeight << "x" << pixelDepth;
        return false;
    }


    if (numberOfFaces != 1 && numberOfFaces != NUM_CUBEMAPFACES) {
        //qDebug() << "Invalid number of faces " << numberOfFaces;
        return false;
    }

    // FIXME validate numberOfMipmapLevels based on the dimensions?

    if ((bytesOfKeyValueData % 4) != 0) {
        //qDebug() << "Invalid keyvalue data size " << bytesOfKeyValueData;
        return false;
    }

    return true;
}

struct AlignedStreamBuffer {
    AlignedStreamBuffer(size_t size, const uint8_t* data) 
        : _size(size), _data(data), _start(_data) { }

    AlignedStreamBuffer(const StoragePtr& storage) 
        : AlignedStreamBuffer(storage->size(), storage->data()) { }


    template<typename T>
    bool read(T& t) {
        // Ensure we don't read more than we have
        if (sizeof(T) > _size) {
            return false;
        }

        // Grab the data
        memcpy(&t, _data, sizeof(T));

        // Advance the pointer
        return skip(sizeof(T));
    }

    size_t offset() const {
        return _data - _start;
    }

    bool skip(size_t skipSize) {
        skipSize = ::khronos::ktx::evalPaddedSize(skipSize);
        if (skipSize > _size) {
            return false;
        }
        _data += skipSize;
        _size -= skipSize;
        return true;
    }

    AlignedStreamBuffer front(size_t size) const {
        return AlignedStreamBuffer { std::min(size, _size), _data };
    }

    bool empty() const {
        return _size == 0;
    }

private:
    size_t _size;
    const uint8_t* _data;
    const uint8_t* const _start;
};

static bool validateKeyValueData(AlignedStreamBuffer kvbuffer) {
    while (!kvbuffer.empty()) {
        uint32_t keyValueSize;
        // Try to fetch the size of the next key value block
        if (!kvbuffer.read(keyValueSize)) {
            //qDebug() << "Unable to read past key value size";
            return false;
        }
        if (!kvbuffer.skip(keyValueSize)) {
            //qDebug() << "Unable to skip past key value data";
            return false;
        }
    }
    
    return true;
}

inline std::shared_ptr<const KTXDescriptor> KTXDescriptor::read(const StoragePtr& src) {
    if (!src || !validate(src)) {
        return nullptr;
    }

    Header header;
    AlignedStreamBuffer buffer { src };
    buffer.read(header);

    auto kvbuffer = buffer.front(header.bytesOfKeyValueData);
    buffer.skip(header.bytesOfKeyValueData);

    ImageDescriptors images;

    // Validate the images
    for (uint32_t mip = 0; mip < header.numberOfMipmapLevels; ++mip) {
        uint32_t imageSize;
        buffer.read(imageSize);
        size_t imageOffset = buffer.offset();

        ImageHeader imageHeader{ header.numberOfFaces == 6, buffer.offset(), imageSize, 0 };

        ImageHeader::Offsets faceOffsets;
        uint32_t arrayElements = header.numberOfArrayElements == 0 ? 1 : header.numberOfArrayElements;
        for (uint32_t arrayElement = 0; arrayElement < arrayElements; ++arrayElement) {
            for (uint8_t face = 0; face < header.numberOfFaces; ++face) {
                faceOffsets.push_back(buffer.offset());
                buffer.skip(imageSize);
            }
        }
        images.push_back({ imageHeader, faceOffsets });
    }

    return std::make_shared<KTXDescriptor>(header, images);
}

inline bool KTXDescriptor::validate(const StoragePtr& src) {
    if (!checkAlignment(src->size())) {
        // All KTX data is 4-byte aligned
        //qDebug() << "Invalid size, not 4 byte aligned";
        return false;
    }

    Header header;
    AlignedStreamBuffer buffer { src };
    if (!buffer.read(header)) {
        //qDebug() << "Unable to read header";
        return false;
    }

    // Basic header validation, are the enums and size valid?
    if (!header.isValid()) {
        //qDebug() << "Invalid header";
        return false;
    }

    // Validate the key value pairs
    if (!validateKeyValueData(buffer.front(header.bytesOfKeyValueData))) {
        //qDebug() << "Invalid key value data";
        return false;
    }

    // now skip the KV data
    if (!buffer.skip(header.bytesOfKeyValueData)) {
        //qDebug() << "Unable to read past key value data";
        return false;
    }


    // Validate the images
    for (uint32_t mip = 0; mip < header.numberOfMipmapLevels; ++mip) {
        uint32_t imageSize;
        if (!buffer.read(imageSize)) {
            //qDebug() << "Unable to read image size";
            return false;
        }

        uint32_t arrayElements = header.numberOfArrayElements == 0 ? 1 : header.numberOfArrayElements;
        for (uint32_t arrayElement = 0; arrayElement < arrayElements; ++arrayElement) {
            for (uint8_t face = 0; face < header.numberOfFaces; ++face) {
                if (!buffer.skip(imageSize)) {
                    //qDebug() << "Unable to skip past image data";
                    return false;
                }
            }
        }
    }

    // The buffer should be empty afer we've skipped all of the KTX data
    if (!buffer.empty()) {
        return false;
    }

    return true;
}



inline bool KTXDescriptor::isValid() const {
    if (!header.isValid()) {
        return false;
    }

    if (images.size() != header.numberOfMipmapLevels) {
        return false;
    }

    // FIXME, do key value checks?

    for (uint8_t mip = 0; mip < header.numberOfMipmapLevels; ++mip) {
        for (uint8_t face = 0; face < header.numberOfFaces; ++face) {
            auto faceOffset = getMipFaceTexelsOffset(mip, face);
            // The face start offset must be 4 byte aligned
            if (!checkAlignment(faceOffset)) {
                return false;
            }

            // The face size must be 4 byte aligned
            auto faceSize = getMipFaceTexelsSize(mip, face);
            if (!checkAlignment(faceSize)) {
                return false;
            }
        }
    }

    return true;
}

inline size_t KTXDescriptor::getMipFaceTexelsSize(uint16_t mip, uint8_t face) const {
    size_t result{ 0 };
    if (mip < images.size()) {
        const auto& faces = images[mip];
        if (face < faces._numFaces) {
            result = faces._faceSize;
        }
    }
    return result;
}

inline size_t KTXDescriptor::getMipFaceTexelsOffset(uint16_t mip, uint8_t face) const {
    size_t result{ 0 };
    if (mip < images.size()) {
        const auto& faces = images[mip];
        if (face < faces._numFaces) {
            result = faces._faceOffsets[face];
        }
    }
    return result;
}

}}  // namespace khronos::ktx

