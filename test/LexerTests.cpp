#include "pljit/SourceCodeManagement.hpp"
#include "pljit/Lexer.hpp"
#include <gtest/gtest.h>
#include "utils/CaptureCOut.hpp"
//---------------------------------------------------------------------------
using namespace pljit;
//---------------------------------------------------------------------------
TEST(Lexer, testBasicTokens) {
    std::string program = "PARAM width, height, depth;";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Token token;
    bool result;

    result = lexer.next(token);
    ASSERT_TRUE(result);
    ASSERT_EQ(token.getType(), Token::TokenType::KEYWORD);
    ASSERT_EQ(token.reference().content(), "PARAM");
    token = {};

    result = lexer.next(token);
    ASSERT_TRUE(result);
    ASSERT_EQ(token.getType(), Token::TokenType::IDENTIFIER);
    ASSERT_EQ(token.reference().content(), "width");
    token = {};

    result = lexer.next(token);
    ASSERT_TRUE(result);
    ASSERT_EQ(token.getType(), Token::TokenType::IDENTIFIER);
    ASSERT_EQ(token.reference().content(), "height");
    token = {};

    result = lexer.next(token);
    ASSERT_TRUE(result);
    ASSERT_EQ(token.getType(), Token::TokenType::IDENTIFIER);
    ASSERT_EQ(token.reference().content(), "depth");
    token = {};

    result = lexer.next(token);
    ASSERT_TRUE(result);
    ASSERT_EQ(token.getType(), Token::TokenType::EMPTY);
}

TEST(Lexer, testIllegalToken) {
    CaptureCOut capture;

    std::string program = "PARAM width? height";
    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Token token;
    bool result;

    result = lexer.next(token);
    ASSERT_TRUE(result);
    ASSERT_EQ(token.getType(), Token::TokenType::KEYWORD);
    ASSERT_EQ(token.reference().content(), "PARAM");
    token = {};

    result = lexer.next(token);
    ASSERT_FALSE(result);
    EXPECT_EQ(capture.str(), "1:12: error: unexpected character\n"
                             "PARAM width? height\n"
                             "           ^\n");
}

// TODO literal follwoing identifier test (with and without whitespace!)!
//  also e.g. operators and braces!
