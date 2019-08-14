//
//  Created by Bradley Austin Davis on 2016/07/01
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <khrpp/ktx.hpp>

#include <mutex>
#include <string>
#include <iostream>
#include <filesystem>
#include <gtest/gtest.h>

#include "storage.hpp"

namespace fs {
using namespace std::filesystem;
}

const fs::path& getKtxResourcePath() {
    static const fs::path RESOURCE_PATH = fs::path(__FILE__ "/../../../../external/ktx/tests/testimages").lexically_normal();
    return RESOURCE_PATH;
}

std::vector<fs::path> getKtxTestFiles() {
    std::vector<fs::path> result;
    for (const auto& p : fs::directory_iterator(getKtxResourcePath())) {
        if (p.is_regular_file() && p.path().extension() == ".ktx") {
            result.push_back(p.path());
        }
    }
    return result;
}

class KtxTest : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(KtxTest, testKtxEvalFunctions) {
    using namespace khrpp;
    EXPECT_EQ(sizeof(ktx::Descriptor::Header), (size_t)ktx::Descriptor::KTX_HEADER_SIZE);
    EXPECT_EQ(ktx::evalPadding(0x0), (uint8_t)0);
    EXPECT_EQ(ktx::evalPadding(0x1), (uint8_t)3);
    EXPECT_EQ(ktx::evalPadding(0x2), (uint8_t)2);
    EXPECT_EQ(ktx::evalPadding(0x3), (uint8_t)1);
    EXPECT_EQ(ktx::evalPadding(0x4), (uint8_t)0);
    EXPECT_EQ(ktx::evalPadding(0x400), (uint8_t)0);
    EXPECT_EQ(ktx::evalPadding(0x401), (uint8_t)3);
    EXPECT_EQ(ktx::evalPaddedSize(0x0), 0x0);
    EXPECT_EQ(ktx::evalPaddedSize(0x1), 0x4);
    EXPECT_EQ(ktx::evalPaddedSize(0x2), 0x4);
    EXPECT_EQ(ktx::evalPaddedSize(0x3), 0x4);
    EXPECT_EQ(ktx::evalPaddedSize(0x4), 0x4);
    EXPECT_EQ(ktx::evalPaddedSize(0x400), 0x400);
    EXPECT_EQ(ktx::evalPaddedSize(0x401), 0x404);
    EXPECT_EQ(ktx::evalAlignedCount((uint32_t)0x0), (uint32_t)0x0);
    EXPECT_EQ(ktx::evalAlignedCount((uint32_t)0x1), (uint32_t)0x1);
    EXPECT_EQ(ktx::evalAlignedCount((uint32_t)0x4), (uint32_t)0x1);
    EXPECT_EQ(ktx::evalAlignedCount((uint32_t)0x5), (uint32_t)0x2);
}

TEST_F(KtxTest, testKhronosCompressionFunctions) {
    using namespace khrpp::gl::texture;
    EXPECT_EQ(evalAlignedCompressedBlockCount<4>(0), (uint32_t)0x0);
    EXPECT_EQ(evalAlignedCompressedBlockCount<4>(1), (uint32_t)0x1);
    EXPECT_EQ(evalAlignedCompressedBlockCount<4>(4), (uint32_t)0x1);
    EXPECT_EQ(evalAlignedCompressedBlockCount<4>(5), (uint32_t)0x2);
    EXPECT_EQ(evalCompressedBlockCount(InternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT, 0x00), (uint32_t)0x00);
    EXPECT_EQ(evalCompressedBlockCount(InternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT, 0x01), (uint32_t)0x01);
    EXPECT_EQ(evalCompressedBlockCount(InternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT, 0x04), (uint32_t)0x01);
    EXPECT_EQ(evalCompressedBlockCount(InternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT, 0x05), (uint32_t)0x02);
    EXPECT_EQ(evalCompressedBlockCount(InternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT, 0x1000), (uint32_t)0x400);

    try {
        evalCompressedBlockCount(InternalFormat::RGBA8, 0x00);
        ASSERT_TRUE(false);
    } catch (const std::runtime_error&) {
        ASSERT_TRUE(true);
    }
}

TEST_F(KtxTest, testValidation) {
    using namespace khrpp;
    for (const auto& file : getKtxTestFiles()) {
        auto storage = khrpp::utils::Storage::readFile(file.string());
        ASSERT_TRUE(ktx::Descriptor::validate(storage->data(), storage->size()));
    }
}
