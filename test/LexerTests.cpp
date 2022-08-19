#include "pljit/code/SourceCodeManagement.hpp"
#include "pljit/lang.hpp"
#include "pljit/lex/Lexer.hpp"
#include <gtest/gtest.h>
#include "./utils/assert_macros.hpp"
//---------------------------------------------------------------------------
using namespace pljit;
using namespace pljit::code;
using namespace pljit::lex;
//---------------------------------------------------------------------------
TEST(Lexer, testBasicTokens) {
    std::string program = "PARAM width, height, depth;";

    code::SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Result<Token> result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::KEYWORD, Keyword::PARAM);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "width");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::COMMA);

    ASSERT_EQ(*lexer.cur_position(), 'h');

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "height");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::COMMA);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "depth");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::SEMICOLON);

    ASSERT_TRUE(lexer.endOfStream());

    // accessing Lexer after end of stream is considered an error!
    result = lexer.consume_next();
    ASSERT_SRC_ERROR(result, CodePosition(1, 28), "unexpected end of stream!", "");
}

TEST(Lexer, testIllegalToken) {
    std::string program = "PARAM width? height";
    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Result<Token> result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::KEYWORD, Keyword::PARAM);

    result = lexer.consume_next();
    ASSERT_SRC_ERROR(result, CodePosition(1, 12), "unexpected character!", "?");
}

TEST(Lexer, testVar) {
    std::string program = "VAR volume;";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Result<Token> result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::KEYWORD, Keyword::VAR);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "volume");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::SEMICOLON);
}

TEST(Lexer, testConst) {
    std::string program = "CONST density = 2400;";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Result<Token> result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::KEYWORD, Keyword::CONST);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "density");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::INIT);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::LITERAL, "2400");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::SEMICOLON);

    ASSERT_TRUE(lexer.endOfStream());
}

TEST(Lexer, testArithmeticExpression) {
    std::string program = "(23* 2) + (3 / (4-2))";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Result<Token> result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::PARENTHESIS, Parenthesis::ROUND_OPEN);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::LITERAL, "23");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::MULTIPLICATION);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::LITERAL, "2");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::PARENTHESIS, Parenthesis::ROUND_CLOSE);


    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::PLUS);


    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::PARENTHESIS, Parenthesis::ROUND_OPEN);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::LITERAL, "3");

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::DIVISION);

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::PARENTHESIS, Parenthesis::ROUND_OPEN);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::LITERAL, "4");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::MINUS);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::LITERAL, "2");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::PARENTHESIS, Parenthesis::ROUND_CLOSE);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::PARENTHESIS, Parenthesis::ROUND_CLOSE);

    ASSERT_TRUE(lexer.endOfStream());
}

TEST(Lexer, testIdentifierLiteralsConcatenated) {
    std::string program = "asdf231index465fooRETURN987sBEGIN0abcdefghijklmnopqrstuvwxyz1ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Result<Token> result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "asdf");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::LITERAL, "231");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "index");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::LITERAL, "465");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "fooRETURN");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::LITERAL, "987");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "sBEGIN");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::LITERAL, "0");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "abcdefghijklmnopqrstuvwxyz");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::LITERAL, "1");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    ASSERT_TRUE(lexer.endOfStream());
}

TEST(Lexer, testConcatenatedOperators) {
    std::string program = "++/-+;*:=*/ /";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Result<Token> result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::PLUS);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::PLUS);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::DIVISION);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::MINUS);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::PLUS);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::SEMICOLON);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::MULTIPLICATION);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::ASSIGNMENT);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::MULTIPLICATION);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::DIVISION);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::DIVISION);

    ASSERT_TRUE(lexer.endOfStream());
}

TEST(Lexer, testConcatenatedParenthesis) {
    std::string program = "(( )((,)( ((";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Result<Token> result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::PARENTHESIS, Parenthesis::ROUND_OPEN);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::PARENTHESIS, Parenthesis::ROUND_OPEN);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::PARENTHESIS, Parenthesis::ROUND_CLOSE);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::PARENTHESIS, Parenthesis::ROUND_OPEN);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::PARENTHESIS, Parenthesis::ROUND_OPEN);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::COMMA);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::PARENTHESIS, Parenthesis::ROUND_CLOSE);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::PARENTHESIS, Parenthesis::ROUND_OPEN);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::PARENTHESIS, Parenthesis::ROUND_OPEN);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::PARENTHESIS, Parenthesis::ROUND_OPEN);

    ASSERT_TRUE(lexer.endOfStream());
}

TEST(Lexer, testConcatenatedSeparators) {
    std::string program = ",;,,;;";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Result<Token> result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::COMMA);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::SEMICOLON);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::COMMA);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::COMMA);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::SEMICOLON);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::SEMICOLON);

    ASSERT_TRUE(lexer.endOfStream());
}

TEST(Lexer, testAssignment) {
    std::string program = "volume := width * height * depth;";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Result<Token> result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "volume");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::ASSIGNMENT);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "width");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::MULTIPLICATION);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "height");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::MULTIPLICATION);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "depth");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::SEMICOLON);

    ASSERT_TRUE(lexer.endOfStream());
}

TEST(Lexer, testReturn) {
    std::string program = "RETURN density*volume;";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Result<Token> result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::KEYWORD, Keyword::RETURN);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "density");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::MULTIPLICATION);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "volume");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::SEMICOLON);

    ASSERT_TRUE(lexer.endOfStream());
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

    Result<Token> result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::KEYWORD, Keyword::PARAM);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "width");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::COMMA);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "height");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::COMMA);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "depth");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::SEMICOLON);

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::KEYWORD, Keyword::VAR);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "volume");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::SEMICOLON);

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::KEYWORD, Keyword::CONST);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "density");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::INIT);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::LITERAL, "2400");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::SEMICOLON);

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::KEYWORD, Keyword::BEGIN);

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "volume");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::ASSIGNMENT);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "width");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::MULTIPLICATION);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "height");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::MULTIPLICATION);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "depth");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::SEMICOLON);

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::KEYWORD, Keyword::RETURN);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "density");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::OPERATOR, Operator::MULTIPLICATION);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "volume");

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::KEYWORD, Keyword::END);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::END_OF_PROGRAM);

    ASSERT_TRUE(lexer.endOfStream());
}

TEST(Lexer, testCharacterAfterProgramTerminator) {
    std::string program = "BEGIN\n"
                          "\tRETURN 0\n"
                          "END.\n"
                          "RETURN 1";
    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Result<Token> result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::KEYWORD, Keyword::BEGIN);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::KEYWORD, Keyword::RETURN);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::LITERAL, "0");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::KEYWORD, Keyword::END);
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::END_OF_PROGRAM);

    ASSERT_FALSE(lexer.endOfStream());
}

TEST(Lexer, testWhitespacesAfterTerminator) {
    std::string program = "program.\n ";
    SourceCodeManagement management{ std::move(program) };
    Lexer lexer { management };

    Result<Token> result;

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::IDENTIFIER, "program");
    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::SEPARATOR, Separator::END_OF_PROGRAM);

    ASSERT_TRUE(lexer.endOfStream());
}

TEST(Lexer, testPeekNext) {
    std::string program = "PARAM width, height, depth;";

    SourceCodeManagement management{ std::move(program) };
    Lexer lexer{ management };

    Result<Token> result;

    result = lexer.peek_next();
    ASSERT_TOKEN(result, Token::Type::KEYWORD, Keyword::PARAM);

    result = lexer.peek_next();
    ASSERT_TOKEN(result, Token::Type::KEYWORD, Keyword::PARAM);

    result = lexer.peek_next();
    ASSERT_TOKEN(result, Token::Type::KEYWORD, Keyword::PARAM);

    ASSERT_NEXT_TOKEN(lexer, result, Token::Type::KEYWORD, Keyword::PARAM);

    result = lexer.peek_next();
    ASSERT_TOKEN(result, Token::Type::IDENTIFIER, "width");

    result = lexer.peek_next();
    ASSERT_TOKEN(result, Token::Type::IDENTIFIER, "width");
}
