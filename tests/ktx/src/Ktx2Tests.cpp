//
//  Created by Bradley Austin Davis on 2019/08/14
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <khrpp/ktx2.hpp>

#include <mutex>
#include <string>
#include <iostream>
#include <filesystem>
#include <gtest/gtest.h>

#include "storage.hpp"

namespace fs {
using namespace std::filesystem;
}

const fs::path& getKtx2ResourcePath() {
    static const fs::path RESOURCE_PATH = fs::path(__FILE__ "/../../../../external/ktx/tests/testimages").lexically_normal();
    return RESOURCE_PATH;
}

std::vector<fs::path> getKtx2TestFiles() {
    std::vector<fs::path> result;
    for (const auto& p : fs::directory_iterator(getKtx2ResourcePath())) {
        if (p.is_regular_file() && p.path().extension() == ".ktx2") {
            result.push_back(p.path());
        }
    }
    return result;
}

class Ktx2Test : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(Ktx2Test, testValidation) {
    using namespace khrpp;
    for (const auto& file : getKtx2TestFiles()) {
        auto storage = khrpp::utils::Storage::readFile(file.string());
        ASSERT_TRUE(ktx2::Descriptor::validate(storage->data(), storage->size()));
    }
}
	