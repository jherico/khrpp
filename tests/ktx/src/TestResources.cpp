#include "TestResources.h"
#include <khrpp/helpers.hpp>


std::list<std::string> getKtxTestFiles() {
    return khrpp::splitString(KTX_TEST_FILES, ':');
}

std::list<std::string> getKtx2TestFiles() {
    return khrpp::splitString(KTX2_TEST_FILES, ':');
}


