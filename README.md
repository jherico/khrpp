# khrpp

khrpp is intended to be a set of header-only, modern C++ helpers for interacting with various Khronos APIs and formats.  

## Current Features

### KTX/KTX2 file parsing

There are two classes, `khrpp::ktx::Descriptor` and `khrpp::ktx2::Descriptor` that are provided by the includes `<khrpp/ktx.hpp>` and `#include <khrpp/ktx2.hpp>` respectively and are able to parse and validate KTX and KTX2 data.

Each has a static `bool validate(const uint8_t* const data, size_t size)` method which will return true or false depending on whether the passed memory buffer is a valid representation.  Each also has a `void parse(const uint8_t* const data, size_t size)` method which will attempt to parse a memory buffer and throw an exception if it is invalid.

The interface designed to be platform neutral and optimized for working with memory mapped files.

### Memory-Mapped file wrapping

The `<khrpp/storage.hpp>` header provides the `khrpp::utils::Storage` class and associated child classes.  `khrpp::utils::Storage` is an abstraction for wrapping read-only memory and provides `size_t size() const` and `const uint8_t* data() const` members as well as an `bool isFast() const` member to act as a heuristic as to whether the wrapped buffer is backed by RAM or disk.  

`khrpp::utils::Storage::ConstPointer` is an alias for `std::shared_ptr<const khrpp::utils::Storage>` and it's assumed most interaction will be through `ConstPointer` instances created by one of the static Storage methods below:

#### `static ConstPointer Storage::wrap(size_t size, uint8_t* data, bool fast = false);`

Wraps an existing memory buffer without taking ownership of it.  Caller may indicate the speed heuristic, but it's assumed false if not provided.

#### `static ConstPointer Storage::create(size_t size, uint8_t* data = nullptr);`

Creates a memory buffer of the specified size and optionally populates it with initial values provided by `data`.  If `data` is not provided, the initial contents are undefined.

#### `static ConstPointer readFile(const std::string& filename);`
    
Maps a file from disk into memory using the platform specific API

Additionally the `Storgage` class provides a `ConstPointer createView(size_t size = 0, size_t offset = 0) const` member function which will return a view of a given offset and size of the parent buffer.  The child buffer will retain a reference to the parent buffer so that even if the parent leaves scope, the child buffer is still valid.  The `fast` heuristic of the child is inherited from the parent.

