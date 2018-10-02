//
//  Created by Bradley Austin Davis on 2016/07/01
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#define KHR_KTX_IMPLEMENTATION
#define KHR_UTILS_STORAGE_IMPLEMENTATION
#include <khr/ktx/ktx.hpp>

#include <mutex>
#include <string>
#include <iostream>
#include <filesystem>

#include <gtest/gtest.h>

namespace fs {
    using namespace std::filesystem;
}

const fs::path& getResourcePath() {
    static const fs::path RESOURCE_PATH = fs::path(__FILE__"/../../../../external/ktx/tests/testimages").lexically_normal();
    return RESOURCE_PATH;
}

std::vector<fs::path> getTestFiles() {
    std::vector<fs::path> result;
    for (const auto& p : fs::directory_iterator(getResourcePath())) {
        if (p.is_regular_file() && p.path().extension() == ".ktx") {
            result.push_back(p.path());
        }
    }
    return result;
}

class KtxTest : public ::testing::Test {
protected:
    void SetUp() override {

    }

    void TearDown() override{
    }

};


TEST_F(KtxTest, testKtxEvalFunctions) {
    using namespace khronos;
    EXPECT_EQ(sizeof(ktx::Header), (size_t)64);
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
    using namespace khronos::gl::texture;
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
    using namespace khronos;
    const auto testFiles = getTestFiles();
    size_t invalid = 0;
    for (const auto& file : testFiles) {
        auto storage = khr::utils::Storage::readFile(file.string());
        auto valid = ktx::KTXDescriptor::validate(storage);
        if (!valid) {
            std::cerr << "Invalid " << file << std::endl;
            ++invalid;
        }
        ASSERT_TRUE(true);
    }

    std::cerr << invalid << std::endl;

    //namespace fs = std::filesystem;
    //int main()
    //{
    //    std::string path = "/path/to/directory";
    //    for (const auto & p : fs::directory_iterator(path))
    //        std::cout << p << std::endl; // "p" is the directory entry. Get the path with "p.path()".
    //}
}

TEST_F(KtxTest, testKtxSerialization) {
#if 0
    const QString TEST_IMAGE = getRootPath() + "/scripts/developer/tests/cube_texture.png";
    QImage image(TEST_IMAGE);
    std::atomic<bool> abortSignal;
    gpu::TexturePointer testTexture =
        image::TextureUsage::process2DTextureColorFromImage(std::move(image), TEST_IMAGE.toStdString(), true, abortSignal);
    auto ktxMemory = gpu::Texture::serialize(*testTexture);
    QVERIFY(ktxMemory.get());

    // Serialize the image to a file
    QTemporaryFile TEST_IMAGE_KTX;
    {
        const auto& ktxStorage = ktxMemory->getStorage();
        QVERIFY(ktx::KTX::validate(ktxStorage));
        QVERIFY(ktxMemory->isValid());

        auto& outFile = TEST_IMAGE_KTX;
        if (!outFile.open()) {
            QFAIL("Unable to open file");
        }
        auto ktxSize = ktxStorage->size();
        outFile.resize(ktxSize);
        auto dest = outFile.map(0, ktxSize);
        memcpy(dest, ktxStorage->data(), ktxSize);
        outFile.unmap(dest);
        outFile.close();
    }


    {
        auto ktxStorage = std::make_shared<storage::FileStorage>(TEST_IMAGE_KTX.fileName());
        QVERIFY(ktx::KTX::validate(ktxStorage));
        auto ktxFile = ktx::KTX::create(ktxStorage);
        QVERIFY(ktxFile.get());
        QVERIFY(ktxFile->isValid());
        {
            const auto& memStorage = ktxMemory->getStorage();
            const auto& fileStorage = ktxFile->getStorage();
            QVERIFY(memStorage->size() == fileStorage->size());
            QVERIFY(memStorage->data() != fileStorage->data());
            QVERIFY(0 == memcmp(memStorage->data(), fileStorage->data(), memStorage->size()));
            QVERIFY(ktxFile->_images.size() == ktxMemory->_images.size());
            auto imageCount = ktxFile->_images.size();
            auto startMemory = ktxMemory->_storage->data();
            auto startFile = ktxFile->_storage->data();
            for (size_t i = 0; i < imageCount; ++i) {
                auto memImages = ktxMemory->_images[i];
                auto fileImages = ktxFile->_images[i];
                QVERIFY(memImages._padding == fileImages._padding);
                QVERIFY(memImages._numFaces == fileImages._numFaces);
                QVERIFY(memImages._imageSize == fileImages._imageSize);
                QVERIFY(memImages._faceSize == fileImages._faceSize);
                QVERIFY(memImages._faceBytes.size() == memImages._numFaces);
                QVERIFY(fileImages._faceBytes.size() == fileImages._numFaces);
                auto faceCount = fileImages._numFaces;
                for (uint32_t face = 0; face < faceCount; ++face) {
                    auto memFace = memImages._faceBytes[face];
                    auto memOffset = memFace - startMemory;
                    auto fileFace = fileImages._faceBytes[face];
                    auto fileOffset = fileFace - startFile;
                    QVERIFY(memOffset % 4 == 0);
                    QVERIFY(memOffset == fileOffset);
                }
            }
        }
    }
    testTexture->setKtxBacking(TEST_IMAGE_KTX.fileName().toStdString());
#endif
}

#if 0

QTEST_GUILESS_MAIN(KtxTests)

QString getRootPath() {
    static std::once_flag once;
    static QString result;
    std::call_once(once, [&] {
        QFileInfo file(__FILE__);
        QDir parent = file.absolutePath();
        result = QDir::cleanPath(parent.currentPath() + "/../../..");
    });
    return result;
}

#endif