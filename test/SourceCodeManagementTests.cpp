#include "pljit/SourceCodeManagement.hpp"
#include <gtest/gtest.h>
#include "utils/CaptureCOut.hpp"
//---------------------------------------------------------------------------
using namespace pljit;
//---------------------------------------------------------------------------
TEST(SourceCodeManagement, testIteratorConcept) {
    ASSERT_TRUE(std::bidirectional_iterator<SourceCodeManagement::iterator>);
}

TEST(SourceCodeManagement, testProgramStorage) {
    std::string program = "Hello World;";
    SourceCodeManagement management{ std::move(program) };

    ASSERT_EQ(management.content(), "Hello World;");
}

TEST(SourceCodeManagement, testSourceCodeIterator) {
    std::string program = "Hello World;";
    SourceCodeManagement management{ std::move(program) };

    auto iterator = management.begin();
    ASSERT_EQ(*iterator, 'H');
    ++iterator;
    ASSERT_EQ(*iterator, 'e');
    iterator++;
    ASSERT_EQ(*iterator, 'l');
    --iterator;
    ASSERT_EQ(*iterator, 'e');
    iterator--;
    ASSERT_EQ(*iterator, 'H');

    auto reference = iterator.codeReference();
    ASSERT_EQ(reference.content(), "H");

    ASSERT_NE(iterator, SourceCodeManagement::iterator());
    ASSERT_EQ(SourceCodeManagement::iterator(), SourceCodeManagement::iterator());
}

TEST(SourceCodeManagement, testSourceCodeError) {
    std::string program = "Hello world";
    SourceCodeManagement management{ std::move(program) };

    auto reference = management.begin().codeReference();
    reference.extend(10);

    SourceCodeError error{ pljit::SourceCodeManagement::ErrorType::NOTE, "some error!", reference };

    ASSERT_EQ(error.reference().content(), "Hello world");
    ASSERT_EQ(error.message(), "some error!");
    ASSERT_EQ(error.type(), pljit::SourceCodeManagement::ErrorType::NOTE);

    CaptureCOut capture;

    error.printCompilerError();
    EXPECT_EQ(capture.str(), "1:1: note: some error!\n"
                             "Hello world\n"
                             "^~~~~~~~~~~\n");
}
