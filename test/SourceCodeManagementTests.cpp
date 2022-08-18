#include "pljit/code/SourceCodeManagement.hpp"
#include "utils/CaptureCOut.hpp"
#include <gtest/gtest.h>
//---------------------------------------------------------------------------
using namespace pljit::code;
//---------------------------------------------------------------------------
TEST(SourceCodeManagement, testIteratorConcept) {
    ASSERT_TRUE(std::bidirectional_iterator<SourceIterator>);
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
    ASSERT_EQ(*reference, "H");

    ASSERT_NE(iterator, SourceIterator());
    ASSERT_EQ(SourceIterator(), SourceIterator());
}

TEST(SourceCodeManagement, testSourceCodeError) {
    std::string program = "Hello world";
    SourceCodeManagement management{ std::move(program) };

    auto reference = management.begin().codeReference();
    reference.extend(10);

    SourceCodeError error = reference.makeError(ErrorType::NOTE, "some error!");

    ASSERT_EQ(*error.reference(), "Hello world");
    ASSERT_EQ(error.message(), "some error!");
    ASSERT_EQ(error.type(), ErrorType::NOTE);
    ASSERT_EQ(error.position(), CodePosition(1, 1));
    ASSERT_TRUE(error.attachedCauses().empty());

    CaptureCOut capture;

    error.printCompilerError();
    EXPECT_EQ(capture.str(), "1:1: note: some error!\n"
                             "Hello world\n"
                             "^~~~~~~~~~~\n");
}

TEST(SourceCodeManagement, testEmptySourceCode) {
    std::string program; // empty string
    SourceCodeManagement management{ std::move(program) };

    SourceCodeError error{
        ErrorType::ERROR,
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
    SourceCodeManagement management{ std::move(program) };

    SourceCodeError error{
        ErrorType::ERROR,
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
    SourceCodeManagement management{ std::move(program) };

    SourceCodeError error{
        ErrorType::ERROR,
        "It's an error!",
        (--management.end()).codeReference()
    };

    CaptureCOut capture;

    error.printCompilerError();
    EXPECT_EQ(capture.str(), "1:13: error: It's an error!\n"
                             "some program\n"
                             "            ^\n");
}

TEST(SourceCodeManagement, testErrorInLineWithTabs) {
    SourceCodeManagement management{ "\t \tsome program;\n" };

    auto iterator = management.begin();
    for (unsigned i = 0; i < 6; ++i) {
        iterator++;
    }

    SourceCodeError error{
        pljit::code::ErrorType::ERROR,
        "ERROR!",
        iterator.codeReference()
    };

    CaptureCOut capture;

    error.printCompilerError();
    EXPECT_EQ(capture.str(), "1:7: error: ERROR!\n"
                             "\t \tsome program;\n"
                             "\t \t   ^\n");
}

// TODO test error handling of multiline errors!
