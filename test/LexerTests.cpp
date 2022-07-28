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

#define ASSERT_NEXT_TOKEN(lexer, result, type, exp_content) \
    (result) = (lexer).next();                                  \
    ASSERT_TOKEN(result, type, exp_content)

#define ASSERT_PROGRAM_END(lexer, result) \
    (result) = (lexer).next(); \
    ASSERT_TRUE((result).success()); \
    ASSERT_EQ((result).token().getType(), Token::TokenType::EMPTY)
//---------------------------------------------------------------------------
TEST(Lexer, testBasicTokens) {
    std::string program = "PARAM width, height, depth;";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Lexer::LexerResult result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::KEYWORD, "PARAM");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "width");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ",");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "height");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ",");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "depth");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ";");

    ASSERT_PROGRAM_END(lexer, result);
}

TEST(Lexer, testIllegalToken) {
    CaptureCOut capture; // TODO remove requirement for capture!

    std::string program = "PARAM width? height";
    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Lexer::LexerResult result;


    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::KEYWORD, "PARAM");

    result = lexer.next();
    ASSERT_TRUE(result.failure());
    result.error().printCompilerError();
    EXPECT_EQ(capture.str(), "1:12: error: unexpected character\n"
                             "PARAM width? height\n"
                             "           ^\n");
}

TEST(Lexer, testVar) {
    std::string program = "VAR volume;";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Lexer::LexerResult result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::KEYWORD, "VAR");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "volume");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ";");
}

TEST(Lexer, testConst) {
    std::string program = "CONST density = 2400;";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Lexer::LexerResult result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::KEYWORD, "CONST");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "density");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "=");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::INTEGER_LITERAL, "2400");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ";");

    ASSERT_PROGRAM_END(lexer, result);
}

TEST(Lexer, testArithmeticExpression) {
    std::string program = "(23* 2) + (3 / (4-2))";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Lexer::LexerResult result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::PARENTHESIS, "(");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::INTEGER_LITERAL, "23");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "*");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::INTEGER_LITERAL, "2");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::PARENTHESIS, ")");


    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "+");


    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::PARENTHESIS, "(");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::INTEGER_LITERAL, "3");

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "/");

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::PARENTHESIS, "(");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::INTEGER_LITERAL, "4");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "-");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::INTEGER_LITERAL, "2");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::PARENTHESIS, ")");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::PARENTHESIS, ")");

    ASSERT_PROGRAM_END(lexer, result);
}

TEST(Lexer, testIdentifierLiteralsConcatenated) {
    std::string program = "asdf231index465fooRETURN987sBEGIN0abcdefghijklmnopqrstuvwxyz1ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Lexer::LexerResult result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "asdf");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::INTEGER_LITERAL, "231");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "index");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::INTEGER_LITERAL, "465");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "fooRETURN");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::INTEGER_LITERAL, "987");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "sBEGIN");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::INTEGER_LITERAL, "0");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "abcdefghijklmnopqrstuvwxyz");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::INTEGER_LITERAL, "1");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    ASSERT_PROGRAM_END(lexer, result);
}

TEST(Lexer, testConcatenatedOperators) {
    std::string program = "++/-+;*:=*/ /";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Lexer::LexerResult result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "+");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "+");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "/");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "-");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "+");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ";");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "*");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, ":=");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "*");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "/");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "/");

    ASSERT_PROGRAM_END(lexer, result);
}

TEST(Lexer, testConcatenatedParenthesis) {
    std::string program = "(( )((,)( ((";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Lexer::LexerResult result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::PARENTHESIS, "(");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::PARENTHESIS, "(");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::PARENTHESIS, ")");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::PARENTHESIS, "(");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::PARENTHESIS, "(");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ",");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::PARENTHESIS, ")");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::PARENTHESIS, "(");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::PARENTHESIS, "(");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::PARENTHESIS, "(");

    ASSERT_PROGRAM_END(lexer, result);
}

TEST(Lexer, testConcatenatedSeparators) {
    std::string program = ",;,,;;";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Lexer::LexerResult result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ",");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ";");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ",");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ",");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ";");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ";");

    ASSERT_PROGRAM_END(lexer, result);
}

TEST(Lexer, testAssignment) {
    std::string program = "volume := width * height * depth;";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Lexer::LexerResult result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "volume");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, ":=");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "width");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "*");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "height");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "*");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "depth");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ";");

    ASSERT_PROGRAM_END(lexer, result);
}

TEST(Lexer, testReturn) {
    std::string program = "RETURN density*volume;";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Lexer::LexerResult result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::KEYWORD, "RETURN");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "density");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "*");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "volume");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ";");

    ASSERT_PROGRAM_END(lexer, result);
}

TEST(Lexer, testWholeProgram) {
    std::string program = "PARAM width, height, depth;\n"
                          "VAR volume;\n"
                          "CONST density = 2400;\n"
                          "BEGIN\n"
                          "\tvolume := width * height * depth;\n"
                          "\tRETURN density * volume\n"
                          "END.";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Lexer::LexerResult result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::KEYWORD, "PARAM");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "width");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ",");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "height");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ",");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "depth");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ";");

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::KEYWORD, "VAR");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "volume");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ";");

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::KEYWORD, "CONST");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "density");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "=");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::INTEGER_LITERAL, "2400");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ";");

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::KEYWORD, "BEGIN");

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "volume");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, ":=");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "width");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "*");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "height");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "*");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "depth");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::SEPARATOR, ";");

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::KEYWORD, "RETURN");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "density");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::OPERATOR, "*");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::IDENTIFIER, "volume");

    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::KEYWORD, "END");

    ASSERT_PROGRAM_END(lexer, result);
}

TEST(Lexer, testCharacterAfterProgramTerminator) {
    CaptureCOut capture; // TODO remove requirement for capture!

    std::string program = "BEGIN\n"
                          "\tRETURN 0\n"
                          "END.\n"
                          "RETURN 1";
    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Lexer::LexerResult result;


    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::KEYWORD, "BEGIN");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::KEYWORD, "RETURN");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::INTEGER_LITERAL, "0");
    ASSERT_NEXT_TOKEN(lexer, result, Token::TokenType::KEYWORD, "END");

    result = lexer.next();
    ASSERT_TRUE(result.failure());
    result.error().printCompilerError();
    EXPECT_EQ(capture.str(), "4:1: error: unexpected character after end of program terminator!\n"
                             "RETURN 1\n"
                             "^\n");
}
