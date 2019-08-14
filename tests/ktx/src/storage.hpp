//
//  Created by Bradley Austin Davis on 2016/02/17
//  Copyright 2013-2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#elif defined(_WIN32)
#include <Windows.h>
#endif

namespace khrpp { namespace utils {

// Abstract class to represent memory that stored _somewhere_ (in system memory or in a file, for example)
class Storage : public ::std::enable_shared_from_this<Storage> {
public:
    using Pointer = ::std::shared_ptr<Storage>;
    using ConstPointer = std::shared_ptr<const Storage>;

    Storage(const Storage&) = delete;
    Storage() = default;
    virtual ~Storage() = default;

    virtual const uint8_t* data() const = 0;
    virtual size_t size() const = 0;
    virtual bool isFast() const = 0;

    static ConstPointer wrap(size_t size, uint8_t* data);
    static ConstPointer create(size_t size, uint8_t* data = nullptr);
    static ConstPointer readFile(const std::string& filename);
    inline ConstPointer createView(size_t size = 0, size_t offset = 0) const;

#if defined(__ANDROID__)
protected:
    static inline AAssetManager*& getAssetManager() {
        static AAssetManager* ASSET_MANAGER;
        return ASSET_MANAGER;
    }

public:
    static inline void setAssetManager(AAssetManager* assetManager) { getAssetManager() = assetManager; }
#endif
};

class ViewStorage : public Storage {
public:
    ViewStorage(const ConstPointer& owner, size_t size, size_t offset)
        : _owner(owner)
        , _size(size)
        , _offset(offset) {}
    inline const uint8_t* data() const override { return _owner->data() + _offset; }
    inline size_t size() const override { return _size; }
    inline bool isFast() const override { return _owner->isFast(); }

private:
    const ConstPointer _owner;
    const size_t _size;
    const size_t _offset;
};

Storage::ConstPointer Storage::createView(size_t viewSize, size_t offset) const {
    auto selfSize = size();
    if (0 == viewSize) {
        viewSize = selfSize;
    }
    if ((viewSize + offset) > selfSize) {
        throw std::runtime_error("Invalid mapping range");
    }
    return std::make_shared<ViewStorage>(shared_from_this(), viewSize, offset);
}

class WrapperStorage : public Storage {
public:
    WrapperStorage(size_t size, const uint8_t* data)
        : _size(size)
        , _data(data) {}
    const size_t _size;
    const uint8_t* const _data;

    const uint8_t* data() const override { return _data; }
    size_t size() const override { return _size; }
    bool isFast() const override { return true; }
};

inline Storage::ConstPointer Storage::wrap(size_t size, uint8_t* data) {
    return std::make_shared<const WrapperStorage>(size, data);
}

class MemoryStorage : public Storage {
public:
    MemoryStorage(size_t size, const uint8_t* data = nullptr) {
        _data.resize(size);
        if (data) {
            memcpy(_data.data(), data, size);
        }
    }
    const uint8_t* data() const override { return _data.data(); }
    size_t size() const override { return _data.size(); }
    bool isFast() const override { return true; }

private:
    std::vector<uint8_t> _data;
};

inline Storage::ConstPointer Storage::create(size_t size, uint8_t* data) {
    return std::make_shared<MemoryStorage>(size, data);
}

class FileStorage : public Storage {
public:
    FileStorage(const std::string& filename);
    ~FileStorage();
    // Prevent copying & assignment
    FileStorage(const FileStorage& other) = delete;
    FileStorage& operator=(const FileStorage& other) = delete;

    const uint8_t* data() const override { return _mapped; }
    size_t size() const override { return _size; }
    bool isFast() const override { return false; }

private:
    size_t _size{ 0 };
    uint8_t* _mapped{ nullptr };
#if defined(__ANDROID__)
    AAsset* _asset{ nullptr };
#elif defined(_WIN32)
    HANDLE _file{ INVALID_HANDLE_VALUE };
    HANDLE _mapFile{ INVALID_HANDLE_VALUE };
#else
    // FIXME add support for posix memory mapped data
    std::vector<uint8_t> _data;
#endif
};

inline FileStorage::FileStorage(const std::string& filename) {
#if defined(__ANDROID__)
    // Load shader from compressed asset
    _asset = AAssetManager_open(getAssetManager(), filename.c_str(), AASSET_MODE_BUFFER);
    assert(_asset);
    _size = AAsset_getLength(_asset);
    assert(_size > 0);
    _mapped = (uint8_t*)(AAsset_getBuffer(_asset));
#elif defined(_WIN32)
    _file = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (_file == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to open file");
    }
    {
        DWORD dwFileSizeHigh;
        _size = GetFileSize(_file, &dwFileSizeHigh);
        _size += (((size_t)dwFileSizeHigh) << 32);
    }
    _mapFile = CreateFileMappingA(_file, NULL, PAGE_READONLY, 0, 0, NULL);
    if (_mapFile == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to create mapping");
    }
    _mapped = (uint8_t*)MapViewOfFile(_mapFile, FILE_MAP_READ, 0, 0, 0);
#else
    // FIXME move to posix memory mapped files
    // open the file:
    std::ifstream file(filename, std::ios::binary);
    // Stop eating new lines in binary mode!!!
    file.unsetf(std::ios::skipws);

    // get its size:
    std::streampos fileSize;

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // reserve capacity
    _data.reserve(fileSize);

    // read the data:
    _data.insert(vec.begin(), std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());
    file.close();
#endif
}

inline FileStorage::~FileStorage() {
#if defined(__ANDROID__)
    AAsset_close(_asset);
#elif defined(_WIN32)
    UnmapViewOfFile(_mapped);
    CloseHandle(_mapFile);
    CloseHandle(_file);
#else
#endif
}

inline Storage::ConstPointer Storage::readFile(const std::string& filename) {
    return std::make_shared<const FileStorage>(filename);
}

}}  // namespace khrpp::utils
