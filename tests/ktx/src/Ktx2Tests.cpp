//
//  Created by Bradley Austin Davis on 2019/08/14
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <khrpp/ktx2.hpp>
#include <khrpp/storage.hpp>

#include "TestResources.h"

#include <gtest/gtest.h>

using namespace khrpp;

class Ktx2Test : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(Ktx2Test, testValidation) {
     for (const auto& file : getKtx2TestFiles()) {
         auto storage = khrpp::utils::Storage::readFile(file);
         ASSERT_TRUE(ktx2::Descriptor::validate(storage->data(), storage->size()));
     }
 }
	
TEST_F(Ktx2Test, testKtxInalidation) {
    for (const auto& file : getKtxTestFiles()) {
        auto storage = khrpp::utils::Storage::readFile(file);
        ASSERT_FALSE(ktx2::Descriptor::validate(storage->data(), storage->size()));
    }
 }
	
