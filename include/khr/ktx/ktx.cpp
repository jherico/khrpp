#include "ktx.hpp"

#include <cassert>

#include <algorithm>  //min max and more
#include <iostream>

using namespace khronos;
using namespace khronos::ktx;

const Header::Identifier ktx::Header::IDENTIFIER{ { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A } };

Header::Header() {
    memcpy(identifier, IDENTIFIER.data(), IDENTIFIER_LENGTH);
}

uint32_t Header::evalMaxDimension() const {
    return std::max(getPixelWidth(), std::max(getPixelHeight(), getPixelDepth()));
}

uint32_t Header::evalPixelOrBlockDimension(uint32_t pixelDimension) const {
    if (isCompressed()) {
        return khronos::gl::texture::evalCompressedBlockCount(getGLInternaFormat(), pixelDimension);
    }
    return pixelDimension;
}

uint32_t Header::evalMipPixelOrBlockDimension(uint32_t mipLevel, uint32_t pixelDimension) const {
    uint32_t mipPixelDimension = evalMipDimension(mipLevel, pixelDimension);
    return evalPixelOrBlockDimension(mipPixelDimension);
}

uint32_t Header::evalPixelOrBlockWidth(uint32_t level) const {
    return evalMipPixelOrBlockDimension(level, getPixelWidth());
}

uint32_t Header::evalPixelOrBlockHeight(uint32_t level) const {
    return evalMipPixelOrBlockDimension(level, getPixelHeight());
}

uint32_t Header::evalPixelOrBlockDepth(uint32_t level) const {
    return evalMipDimension(level, getPixelDepth());
}

size_t Header::evalPixelOrBlockBitSize() const {
    size_t result = 0;
    auto format = getGLInternaFormat();
    if (isCompressed()) {
        result = khronos::gl::texture::evalCompressedBlockBitSize(format);
    } else {
        result = khronos::gl::texture::evalUncompressedBlockBitSize(format);
    }

    if (0 == result) {
        std::cerr << "Unknown ktx format: " << glFormat << " " << glBaseInternalFormat << " " << glInternalFormat;
    }
    return result;
}

size_t Header::evalUnalignedRowSize(uint32_t level) const {
    auto pixWidth = evalPixelOrBlockWidth(level);
    auto pixSize = evalPixelOrBlockBitSize();
    if (pixSize == 0) {
        return 0;
    }
    auto totalByteSize = pixWidth * pixSize;
    // Round to the nearest upper byte size
    return totalByteSize = (totalByteSize / 8) + (((totalByteSize % 8) != 0) & 1);
}

size_t Header::evalRowSize(uint32_t level) const {
    return evalPaddedSize(evalUnalignedRowSize(level));
}

size_t Header::evalUnalignedFaceSize(uint32_t level) const {
    auto pixHeight = evalPixelOrBlockHeight(level);
    auto pixDepth = evalPixelOrBlockDepth(level);
    auto rowSize = evalUnalignedRowSize(level);
    return pixDepth * pixHeight * rowSize;
}

size_t Header::evalFaceSize(uint32_t level) const {
    auto pixHeight = evalPixelOrBlockHeight(level);
    auto pixDepth = evalPixelOrBlockDepth(level);
    auto rowSize = evalRowSize(level);
    return pixDepth * pixHeight * rowSize;
}

size_t Header::evalImageSize(uint32_t level) const {
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

size_t KTXDescriptor::getValueOffsetForKey(const std::string& key) const {
    size_t offset{ 0 };
    for (auto& kv : keyValues) {
        if (kv._key == key) {
            return offset + ktx::KV_SIZE_WIDTH + kv._key.size() + 1;
        }
        offset += kv.serializedByteSize();
    }
    return 0;
}


size_t KTXDescriptor::getImagesOffset() const {
    size_t offset = sizeof(Header);

    if (!keyValues.empty()) {
        size_t keyValuesSize = KeyValue::serializedKeyValuesByteSize(keyValues);
        offset += keyValuesSize;
    }

    return offset;
}

size_t KTXDescriptor::evalStorageSize() const {
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


ImageDescriptors Header::generateImageDescriptors() const {
    ImageDescriptors descriptors;

    size_t imageOffset = 0;
    for (uint32_t level = 0; level < numberOfMipmapLevels; ++level) {
        auto imageSize = static_cast<uint32_t>(evalImageSize(level));
        if (!checkAlignment(imageSize)) {
            return ImageDescriptors();
        }
        if (imageSize == 0) {
            return ImageDescriptors();
        }
        ImageHeader header{ numberOfFaces == NUM_CUBEMAPFACES, imageOffset, imageSize, 0 };

        imageOffset += (imageSize * numberOfFaces) + ktx::IMAGE_SIZE_WIDTH;

        ImageHeader::FaceOffsets offsets;
        // TODO Add correct face offsets
        for (uint32_t i = 0; i < numberOfFaces; ++i) {
            offsets.push_back(0);
        }
        descriptors.push_back(ImageDescriptor(header, offsets));
    }

    return descriptors;
}

KeyValue::KeyValue(const std::string& key, uint32_t valueByteSize, const Byte* value)
    : _byteSize((uint32_t)key.size() + 1 + valueByteSize)
    ,  // keyString size + '\0' ending char + the value size
    _key(key)
    , _value(valueByteSize) {
    if (_value.size() && value) {
        memcpy(_value.data(), value, valueByteSize);
    }
}

KeyValue::KeyValue(const std::string& key, const std::string& value)
    : KeyValue(key, (uint32_t)value.size(), (const Byte*)value.data()) {
}

uint32_t KeyValue::serializedByteSize() const {
    return (uint32_t)sizeof(uint32_t) + evalPaddedSize(_byteSize);
}

uint32_t KeyValue::serializedKeyValuesByteSize(const KeyValues& keyValues) {
    uint32_t keyValuesSize = 0;
    for (auto& keyval : keyValues) {
        keyValuesSize += keyval.serializedByteSize();
    }
    assert(keyValuesSize % 4 == 0);
    return keyValuesSize;
}

void KTX::resetStorage(const StoragePointer& storage) {
    _storage = storage;
    if (_storage->size() >= sizeof(Header)) {
        memcpy(&_header, _storage->data(), sizeof(Header));
    }
}

const Header& KTX::getHeader() const {
    return _header;
}

size_t KTX::getKeyValueDataSize() const {
    return _header.bytesOfKeyValueData;
}

size_t KTX::getTexelsDataSize() const {
    if (!_storage) {
        return 0;
    }
    return _storage->size() - sizeof(Header) - getKeyValueDataSize();
}

const Byte* KTX::getKeyValueData() const {
    if (!_storage) {
        return nullptr;
    }
    return (_storage->data() + sizeof(Header));
}

const Byte* KTX::getTexelsData() const {
    if (!_storage) {
        return nullptr;
    }
    return (_storage->data() + sizeof(Header) + getKeyValueDataSize());
}

storage::StoragePointer KTX::getMipFaceTexelsData(uint16_t mip, uint8_t face) const {
    storage::StoragePointer result;
    if (mip < _images.size()) {
        const auto& faces = _images[mip];
        if (face < faces._numFaces) {
            auto faceOffset = faces._faceBytes[face] - _storage->data();
            auto faceSize = faces._faceSize;
            result = _storage->createView(faceSize, faceOffset);
        }
    }
    return result;
}

size_t KTXDescriptor::getMipFaceTexelsSize(uint16_t mip, uint8_t face) const {
    size_t result{ 0 };
    if (mip < images.size()) {
        const auto& faces = images[mip];
        if (face < faces._numFaces) {
            result = faces._faceSize;
        }
    }
    return result;
}

size_t KTXDescriptor::getMipFaceTexelsOffset(uint16_t mip, uint8_t face) const {
    size_t result{ 0 };
    if (mip < images.size()) {
        const auto& faces = images[mip];
        if (face < faces._numFaces) {
            result = faces._faceOffsets[face];
        }
    }
    return result;
}

ImageDescriptor Image::toImageDescriptor(const Byte* baseAddress) const {
    FaceOffsets offsets;
    offsets.resize(_faceBytes.size());
    for (size_t face = 0; face < _numFaces; ++face) {
        offsets[face] = _faceBytes[face] - baseAddress;
    }
    // Note, implicit cast of *this to const ImageHeader&
    return ImageDescriptor(*this, offsets);
}

Image ImageDescriptor::toImage(const ktx::StoragePointer& storage) const {
    FaceBytes faces;
    faces.resize(_faceOffsets.size());
    for (size_t face = 0; face < _numFaces; ++face) {
        faces[face] = storage->data() + _faceOffsets[face];
    }
    // Note, implicit cast of *this to const ImageHeader&
    return Image(*this, faces);
}

KTXDescriptor KTX::toDescriptor() const {
    ImageDescriptors newDescriptors;
    auto storageStart = _storage ? _storage->data() : nullptr;
    for (size_t i = 0; i < _images.size(); ++i) {
        newDescriptors.emplace_back(_images[i].toImageDescriptor(storageStart));
    }
    return { this->_header, newDescriptors, this->_keyValues};
}

KTX::KTX(const StoragePointer& storage, const Header& header, const KeyValues& keyValues, const Images& images)
    : _header(header)
    , _storage(storage)
    , _keyValues(keyValues)
    , _images(images) {
}
