#include "pljit/SourceCodeManagement.hpp"
#include "pljit/Lexer.hpp"
#include <gtest/gtest.h>
#include "utils/CaptureCOut.hpp"
//---------------------------------------------------------------------------
using namespace pljit;
//---------------------------------------------------------------------------

#define ASSERT_TOKEN(result, type, exp_content) \
    ASSERT_TRUE((result).success()); \
    ASSERT_EQ((result).token().getType(), (type)); \
    ASSERT_EQ((result).token().reference().content(), (exp_content))

//---------------------------------------------------------------------------
TEST(Lexer, testBasicTokens) {
    std::string program = "PARAM width, height, depth;";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Lexer::LexerResult result;

    result = lexer.next();
    ASSERT_TOKEN(result, Token::TokenType::KEYWORD, "PARAM");

    result = lexer.next();
    ASSERT_TOKEN(result, Token::TokenType::IDENTIFIER, "width");

    result = lexer.next();
    ASSERT_TOKEN(result, Token::TokenType::SEPARATOR, ",");

    result = lexer.next();
    ASSERT_TOKEN(result, Token::TokenType::IDENTIFIER, "height");

    result = lexer.next();
    ASSERT_TOKEN(result, Token::TokenType::SEPARATOR, ",");

    result = lexer.next();
    ASSERT_TOKEN(result, Token::TokenType::IDENTIFIER, "depth");

    result = lexer.next();
    ASSERT_TOKEN(result, Token::TokenType::SEPARATOR, ";");

    result = lexer.next();
    ASSERT_TRUE(result.success());
    ASSERT_EQ(result.token().getType(), Token::TokenType::EMPTY);
}

TEST(Lexer, testIllegalToken) {
    CaptureCOut capture; // TODO remove requirement for capture!

    std::string program = "PARAM width? height";
    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Lexer::LexerResult result;


    result = lexer.next();
    ASSERT_TOKEN(result, Token::TokenType::KEYWORD, "PARAM");

    result = lexer.next();
    ASSERT_TRUE(result.failure());
    result.error().printCompilerError();
    EXPECT_EQ(capture.str(), "1:12: error: unexpected character\n"
                             "PARAM width? height\n"
                             "           ^\n");
}

// TODO literal follwoing identifier test (with and without whitespace!)!
//  also e.g. operators and braces!

// TODO := operator

// TODO . terminator!
