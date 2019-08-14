//
//  Created by Bradley Austin Davis on 2019/08/12
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef khrpp_helpers_hpp
#define khrpp_helpers_hpp

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>

#if defined(HAVE_STD_FORMAT)
#include <format>
#define FORMAT(s, ...) std::format(s, __VA_ARGS__)
#elif defined(HAVE_FMT_FORMAT)
#include <fmt/format.h>
#define FORMAT(s, ...) fmt::format(s, __VA_ARGS__)
#else
#define FORMAT(s, ...) std::string(s)
#endif

namespace khrpp {

struct AlignedStreamBuffer {
    AlignedStreamBuffer(size_t size, const uint8_t* data)
        : _size(size)
        , _data(data)
        , _start(_data) {}

    inline size_t offset() const { return _data - _start; }
    inline bool empty() const { return _size == 0; }
    inline size_t size() const { return _size; }

    template <typename T>
    inline bool read(T& t) {
        size_t count = sizeof(T);
        // Ensure we don't read more than we have
        if (sizeof(T) > _size) {
            return false;
        }

        // Grab the data
        return readBytes(count, &t);
    }

    inline bool readBytes(size_t byteCount, void* outData) {
        memcpy(outData, _data, byteCount);
        return skip(byteCount);
    }

    bool readVector(size_t byteCount, std::vector<uint8_t>& bytes) {
        if (byteCount > _size) {
            return false;
        }
        bytes.resize(byteCount);
        return readBytes(byteCount, bytes.data());
    }

    inline bool skip(size_t skipSize) {
        if (skipSize > _size) {
            return false;
        }
        _data += skipSize;
        _size -= skipSize;
        return true;
    }

    bool align(size_t alignment, bool zeroCheck = true) {
        if (0 == alignment) {
            return true;
        }

        const auto pos = offset();
        const auto alignedPos = (pos + (alignment - 1)) & ~(alignment - 1);
        if (alignedPos == pos) {
            return true;
        }

        const auto diff = alignedPos - pos;
        if (diff > _size) {
            return false;
        }

        if (zeroCheck) {
            for (size_t i = 0; i < diff; ++i) {
                if (_data[i] != 0) {
                    return false;
                }
            }
        }
        return skip(diff);
    }

    AlignedStreamBuffer front(size_t size) const { return AlignedStreamBuffer{ std::min(size, _size), _data }; }

private:
    size_t _size;
    const uint8_t* _data;
    const uint8_t* const _start;
};

using KeyValueMap = std::unordered_map<std::string, Bytes>;

inline void parseKtxKeyValueData(AlignedStreamBuffer kvBuffer, KeyValueMap& result, bool zeroCheck = true) {
    while (!kvBuffer.empty() && kvBuffer.size() > 4) {
        std::vector<uint8_t> bytes;
        uint32_t kvSize;
        if (!kvBuffer.read(kvSize)) {
            throw std::runtime_error("Unable to parse KVD size");
        }
        if (!kvBuffer.readVector(kvSize, bytes)) {
            throw std::runtime_error("Unable to read KVD data");
        }

        auto nullPos = std::find(bytes.begin(), bytes.end(), 0);
        if (nullPos == bytes.end()) {
            throw std::runtime_error("Unable to find key termination null in KVD data");
        }
        result.emplace(std::string{ (const char*)bytes.data() }, Bytes{ nullPos + 1, bytes.end() });
        if (kvBuffer.size() != 0) {
            if (!kvBuffer.align(4)) {
                throw std::runtime_error("Unable to align to key-value interval, or alignment padding is non-Zero");
            }
        }
    }

    if (!kvBuffer.empty()) {
        throw std::runtime_error("Unexpected additional KVD data");
    }
}

}  // namespace khrpp

#endif