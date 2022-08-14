//
// Created by Andreas Bauer on 14.08.22.
//

#include "pljit/pljit.hpp"
#include <gtest/gtest.h>

//---------------------------------------------------------------------------
using namespace pljit;
//---------------------------------------------------------------------------
TEST(Pljit, testExampleProgram) {
    Pljit pljit;

    auto func = pljit.registerFunction("PARAM width, height, depth;\n"
                                       "VAR volume;\n"
                                       "CONST density = 2400;\n"
                                       "BEGIN\n"
                                       "  volume := width * height * depth;\n"
                                       "  RETURN density * volume\n"
                                       "END.");

    auto result = func({100, 100, 100});
    ASSERT_TRUE(result.success());
    ASSERT_EQ(*result, 2400000000);
}

// TODO test some more programs!

// TODO test calling from multiple threads!
//---------------------------------------------------------------------------
