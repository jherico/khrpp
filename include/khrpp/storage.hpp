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
#include <fstream>
#include <memory>

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#elif defined(_WIN32)
#include <Windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

    static ConstPointer wrap(size_t size, uint8_t* data, bool fast = false);
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
        : _owner{ owner }
        , _size{ size }
        , _offset{ offset } {
        if (offset + size > owner->size()) {
            throw new std::runtime_error("Invalid view range");
        }
    }
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
        viewSize = selfSize - offset;
    }
    return std::make_shared<ViewStorage>(shared_from_this(), viewSize, offset);
}

class WrapperStorage : public Storage {
public:
    WrapperStorage(size_t size, const uint8_t* data, bool fast = false)
        : _size{ size }
        , _data{ data }
        , _fast{ fast } {}
    const size_t _size;
    const uint8_t* const _data;
    const bool _fast;

    const uint8_t* data() const override { return _data; }
    size_t size() const override { return _size; }
    bool isFast() const override { return true; }
};

inline Storage::ConstPointer Storage::wrap(size_t size, uint8_t* data, bool fast) {
    return std::make_shared<const WrapperStorage>(size, data, fast);
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
    template <typename T = void>
    static void withBinaryFileContents(const std::string& filename, const std::function<void(const char* filename, size_t size, const T * data)>& handler) {
        FileStorage storage{ filename };
        handler(filename.c_str(), storage.size(), (const T*)storage.data());
    }

    template <typename T = void>
    static void withBinaryFileContents(const std::string& filename, const std::function<void(size_t size, const T* data)>& handler) {
        withBinaryFileContents<T>(filename, [&](const char* filename, size_t size, const T* data) { handler(size, data); });
    }

    FileStorage(const std::string& filename);
    ~FileStorage();
    // Prevent copying & assignment
    FileStorage(const FileStorage& other) = delete;
    FileStorage& operator=(const FileStorage& other) = delete;

    const uint8_t* data() const override { return _mapped; }
    size_t size() const override { return _size; }
    bool isFast() const override { return false; }

private:
    const size_t _size{ 0 };
    const uint8_t* const _mapped{ nullptr };
#if defined(__ANDROID__)
    AAsset* _asset{ nullptr };
#elif defined(_WIN32)
    HANDLE _file{ INVALID_HANDLE_VALUE };
    HANDLE _mapFile{ INVALID_HANDLE_VALUE };
#else
    int _fd{ -1 };
#endif
};


inline FileStorage::FileStorage(const std::string& filename) {
#if defined(__ANDROID__)
    // Load shader from compressed asset
    _asset = AAssetManager_open(getAssetManager(), filename.c_str(), AASSET_MODE_BUFFER);
    assert(_asset);
    const_cast<size_t&>(_size) = AAsset_getLength(_asset);
    assert(_size > 0);
    const_cast<const uint8_t*&>(_mapped) = (uint8_t*)(AAsset_getBuffer(_asset));
#elif defined(_WIN32)
    _file = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (_file == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to open file");
    }
    {
        DWORD dwFileSizeHigh;
        const_cast<size_t&>(_size) = GetFileSize(_file, &dwFileSizeHigh);
        const_cast<size_t&>(_size) += (((size_t)dwFileSizeHigh) << 32);
    }
    _mapFile = CreateFileMappingA(_file, NULL, PAGE_READONLY, 0, 0, NULL);
    if (0 == _mapFile || _mapFile == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to create mapping");
    }
    const_cast<const uint8_t*&>(_mapped) = (uint8_t*)MapViewOfFile(_mapFile, FILE_MAP_READ, 0, 0, 0);
#else
    _fd = open(filename.c_str(), O_RDONLY);
    if (-1 == _fd) {
        throw std::runtime_error("Failed to open file");
    }

    {
        struct stat sb;
        if (-1 == fstat(_fd, &sb)) {
            throw std::runtime_error("Unable to stat file");
        }
        const_cast<size_t&>(_size) = sb.st_size;
    }
    const_cast<const uint8_t*&>(_mapped) = static_cast<uint8_t*>(mmap(nullptr, _size, PROT_READ, MAP_PRIVATE, _fd, 0));
    if (_mapped == MAP_FAILED) {
        throw std::runtime_error("Unable to mmap file");
    }
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
    munmap(const_cast<uint8_t*&>(_mapped), _size);
    close(_fd);
#endif
}

inline Storage::ConstPointer Storage::readFile(const std::string& filename) {
    return std::make_shared<const FileStorage>(filename);
}

}}  // namespace khrpp::utils
