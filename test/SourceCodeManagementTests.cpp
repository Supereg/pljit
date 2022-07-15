#include "pljit/SourceCodeManagement.hpp"
#include <gtest/gtest.h>
//---------------------------------------------------------------------------
using namespace pljit;
//---------------------------------------------------------------------------
TEST(SourceCodeManagement, test_iterator_concept) {
    ASSERT_TRUE(std::bidirectional_iterator<SourceCodeManagement::iterator>);
}

TEST(SourceCodeManagement, test_program_storage) {
    std::string program = "Hello World;";
    SourceCodeManagement management{ std::move(program) };

    ASSERT_EQ(management.content(), "Hello World;");
}
