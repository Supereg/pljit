#include "pljit/code/SourceCodeManagement.hpp"
#include "utils/CaptureCOut.hpp"
#include <gtest/gtest.h>
//---------------------------------------------------------------------------
using namespace pljit;
//---------------------------------------------------------------------------
TEST(SourceCodeManagement, testIteratorConcept) {
    ASSERT_TRUE(std::bidirectional_iterator<code::SourceCodeManagement::iterator>);
}

TEST(SourceCodeManagement, testProgramStorage) {
    std::string program = "Hello World;";
    code::SourceCodeManagement management{ std::move(program) };

    ASSERT_EQ(management.content(), "Hello World;");
}

TEST(SourceCodeManagement, testSourceCodeIterator) {
    std::string program = "Hello World;";
    code::SourceCodeManagement management{ std::move(program) };

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

    ASSERT_NE(iterator, code::SourceCodeManagement::iterator());
    ASSERT_EQ(code::SourceCodeManagement::iterator(), code::SourceCodeManagement::iterator());
}

TEST(SourceCodeManagement, testSourceCodeError) {
    std::string program = "Hello world";
    code::SourceCodeManagement management{ std::move(program) };

    auto reference = management.begin().codeReference();
    reference.extend(10);

    code::SourceCodeError error = reference.makeError(code::SourceCodeManagement::ErrorType::NOTE, "some error!");

    ASSERT_EQ(error.reference().content(), "Hello world");
    ASSERT_EQ(error.message(), "some error!");
    ASSERT_EQ(error.type(), code::SourceCodeManagement::ErrorType::NOTE);

    CaptureCOut capture;

    error.printCompilerError();
    EXPECT_EQ(capture.str(), "1:1: note: some error!\n"
                             "Hello world\n"
                             "^~~~~~~~~~~\n");
}

TEST(SourceCodeManagement, testEmptySourceCode) {
    std::string program; // empty string
    code::SourceCodeManagement management{ std::move(program) };

    code::SourceCodeError error{
        code::SourceCodeManagement::ErrorType::ERROR,
        "It's an error!",
        management.begin().codeReference()
    };

    CaptureCOut capture;

    error.printCompilerError();
    EXPECT_EQ(capture.str(), "1:1: error: It's an error!\n"
                             "\n"
                             "^\n");
}

TEST(SourceCodeManagement, testErrorAtIteratorEnd) {
    std::string program = "some program\nnew"; // empty string
    code::SourceCodeManagement management{ std::move(program) };

    code::SourceCodeError error{
        code::SourceCodeManagement::ErrorType::ERROR,
        "It's an error!",
        management.end().codeReference()
    };

    CaptureCOut capture;

    error.printCompilerError();
    EXPECT_EQ(capture.str(), "2:4: error: It's an error!\n"
                             "new\n"
                             "   ^\n");
}

TEST(SourceCodeManagement, testErrorAtIteratorEndNewLine) {
    std::string program = "some program\n"; // empty string
    code::SourceCodeManagement management{ std::move(program) };

    code::SourceCodeError error{
        code::SourceCodeManagement::ErrorType::ERROR,
        "It's an error!",
        (--management.end()).codeReference()
    };

    CaptureCOut capture;

    error.printCompilerError();
    EXPECT_EQ(capture.str(), "1:13: error: It's an error!\n"
                             "some program\n"
                             "            ^\n");
}
