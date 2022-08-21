#include "pljit/code/SourceCodeManagement.hpp"
#include "pljit/lang.hpp"
#include "pljit/lex/Lexer.hpp"
#include "pljit/parse/ParseTreeDOTVisitor.hpp"
#include "pljit/parse/Parser.hpp"
#include "utils/CaptureCOut.hpp"
#include <gtest/gtest.h>
#include "./utils/assert_macros.hpp"

//---------------------------------------------------------------------------
using namespace pljit;
using namespace pljit::code;
using namespace pljit::parse;
using namespace pljit::lex;
//---------------------------------------------------------------------------
TEST(Parser, testGenericTerminal) {
    {
        SourceCodeManagement management{"BEGIN"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<GenericTerminal> terminal = parser.parseGenericTerminal(Token::Type::KEYWORD, Keyword::BEGIN, "expected BEGIN");
        ASSERT_TRUE(terminal);
        ASSERT_EQ(terminal->value(), "BEGIN");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<GenericTerminal> terminal = parser.parseGenericTerminal(pljit::lex::Token::Type::KEYWORD, Keyword::BEGIN, "expected BEGIN");
        ASSERT_SRC_ERROR(terminal, CodePosition(1, 1), "Unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"BEGIN"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<GenericTerminal> terminal = parser.parseGenericTerminal(pljit::lex::Token::Type::KEYWORD, Keyword::END, "expected END");
        ASSERT_SRC_ERROR(terminal, CodePosition(1, 1), "expected END", "BEGIN");
    }
}

TEST(Parser, testLiteral) {
    {
        SourceCodeManagement management{"02731"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<Literal> literal = parser.parseLiteral();
        ASSERT_TRUE(literal);
        ASSERT_EQ(literal->value(), 2731);
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<Literal> literal = parser.parseLiteral();
        ASSERT_SRC_ERROR(literal, CodePosition(1, 1), "Unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"width"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<Literal> literal = parser.parseLiteral();
        ASSERT_SRC_ERROR(literal, CodePosition(1, 1), "Expected literal!", "width");
    }
    {
        SourceCodeManagement management{"18446744073709551616"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<Literal> literal = parser.parseLiteral();
        ASSERT_SRC_ERROR(literal, CodePosition(1, 1), "Integer literal is out of range. Expected singed 64-bit!", "18446744073709551616");
    }
}

TEST(Parser, testIdentifier) {
    {
        SourceCodeManagement management{"Program"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<Identifier> identifier = parser.parseIdentifier();
        ASSERT_TRUE(identifier);
        ASSERT_EQ(identifier->value(), "Program");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<Identifier> identifier = parser.parseIdentifier();
        ASSERT_SRC_ERROR(identifier, CodePosition(1, 1), "Unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"1"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<Identifier> identifier = parser.parseIdentifier();
        ASSERT_SRC_ERROR(identifier, CodePosition(1, 1), "Expected an identifier!", "1");
    }
}

TEST(Parser, testPrimaryExpression) {
    {
        SourceCodeManagement management{"width"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<PrimaryExpression> expression = parser.parsePrimaryExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(expression->asIdentifier().value(), "width");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"12"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<PrimaryExpression> expression = parser.parsePrimaryExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(expression->asLiteral().value(), 12);
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"(12)"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<PrimaryExpression> expression = parser.parsePrimaryExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "(12)");
        ASSERT_TRUE(lexer.endOfStream());

        ASSERT_PARSE_TREE(*expression, "graph {\n"
                                      "  n_1 [label=\"primary-expression\",shape=box];\n"
                                      "  n_1 -- n_2;\n"
                                      "  n_2 [label=\"\\\"(\\\"\"];\n"
                                      "  n_1 -- n_3;\n"
                                      "  n_3 [label=\"additive-expression\",shape=box];\n"
                                      "  n_3 -- n_4;\n"
                                      "  n_4 [label=\"multiplicative-expression\",shape=box];\n"
                                      "  n_4 -- n_5;\n"
                                      "  n_5 [label=\"unary-expression\",shape=box];\n"
                                      "  n_5 -- n_6;\n"
                                      "  n_6 [label=\"primary-expression\",shape=box];\n"
                                      "  n_6 -- n_7;\n"
                                      "  n_7[label=\"12\"];\n"
                                      "  n_1 -- n_8;\n"
                                      "  n_8 [label=\"\\\")\\\"\"];\n"
                                      "}\n");
    }
    {
        SourceCodeManagement management{"(12("};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<PrimaryExpression> expression = parser.parsePrimaryExpression();
        ASSERT_SRC_ERROR(expression, CodePosition(1, 4), "Expected matching `)` parenthesis!", "(");
    }
    {
        SourceCodeManagement management{"?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<PrimaryExpression> expression = parser.parsePrimaryExpression();
        ASSERT_SRC_ERROR(expression, CodePosition(1, 1), "Unexpected character!", "?");
    }

    {
        SourceCodeManagement management{"()"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<PrimaryExpression> expression = parser.parsePrimaryExpression();
        ASSERT_SRC_ERROR(expression, CodePosition(1, 2), "Expected a primary expression!", ")");
    }
    {
        SourceCodeManagement management{"BEGIN"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<PrimaryExpression> expression = parser.parsePrimaryExpression();
        ASSERT_SRC_ERROR(expression, CodePosition(1, 1), "Expected a primary expression!", "BEGIN");
    }
}

TEST(Parser, testUnaryExpression) {
    {
        SourceCodeManagement management{"12"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<UnaryExpression> expression = parser.parseUnaryExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "12");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"+12"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<UnaryExpression> expression = parser.parseUnaryExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "+12");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"-12"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<UnaryExpression> expression = parser.parseUnaryExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "-12");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"*12"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<UnaryExpression> expression = parser.parseUnaryExpression();
        ASSERT_SRC_ERROR(expression, CodePosition(1, 1), "Unexpected unary operator!", "*");
    }
    {
        SourceCodeManagement management{"/12"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<UnaryExpression> expression = parser.parseUnaryExpression();
        ASSERT_SRC_ERROR(expression, CodePosition(1, 1), "Unexpected unary operator!", "/");
    }
    {
        SourceCodeManagement management{"?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<UnaryExpression> expression = parser.parseUnaryExpression();
        ASSERT_SRC_ERROR(expression, CodePosition(1, 1), "Unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"+?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<UnaryExpression> expression = parser.parseUnaryExpression();
        ASSERT_SRC_ERROR(expression, CodePosition(1, 2), "Unexpected character!", "?");
    }
}

TEST(Parser, testMultiplicativeExpression) {
    {
        SourceCodeManagement management{"+12"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<MultiplicativeExpression> expression = parser.parseMultiplicativeExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "+12");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"+12 * 2"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<MultiplicativeExpression> expression = parser.parseMultiplicativeExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "+12 * 2");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"+12 / 2"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<MultiplicativeExpression> expression = parser.parseMultiplicativeExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "+12 / 2");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"+12 * (-12 / 2)"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<MultiplicativeExpression> expression = parser.parseMultiplicativeExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "+12 * (-12 / 2)");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<MultiplicativeExpression> expression = parser.parseMultiplicativeExpression();
        ASSERT_SRC_ERROR(expression, CodePosition(1, 1), "Unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"+12 ?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<MultiplicativeExpression> expression = parser.parseMultiplicativeExpression();
        ASSERT_SRC_ERROR(expression, CodePosition(1, 5), "Unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"+12 * ?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<MultiplicativeExpression> expression = parser.parseMultiplicativeExpression();
        ASSERT_SRC_ERROR(expression, CodePosition(1, 7), "Unexpected character!", "?");
    }
}

TEST(Parser, testAdditiveExpression) {
    {
        SourceCodeManagement management{"1*1 + 1"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<AdditiveExpression> expression = parser.parseAdditiveExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "1*1 + 1");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"1/1 - 1"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<AdditiveExpression> expression = parser.parseAdditiveExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "1/1 - 1");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"1*1"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<AdditiveExpression> expression = parser.parseAdditiveExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "1*1");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"12"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<AdditiveExpression> expression = parser.parseAdditiveExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "12");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"1 + 1 + 1"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<AdditiveExpression> expression = parser.parseAdditiveExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "1 + 1 + 1");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"1 - 1 - 1"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<AdditiveExpression> expression = parser.parseAdditiveExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "1 - 1 - 1");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"1 * 1 * 1"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<AdditiveExpression> expression = parser.parseAdditiveExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "1 * 1 * 1");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"1++2"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<AdditiveExpression> expression = parser.parseAdditiveExpression();
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "1++2");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"1--2"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<AdditiveExpression> expression = parser.parseAdditiveExpression();
        if (expression.isFailure()) {
            expression.error().printCompilerError();
        }
        ASSERT_TRUE(expression);
        ASSERT_EQ(*expression->reference(), "1--2");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"+12 + ?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<AdditiveExpression> expression = parser.parseAdditiveExpression();
        ASSERT_SRC_ERROR(expression, CodePosition(1, 7), "Unexpected character!", "?");
    }
}

TEST(Parser, testStatement) {
    {
        SourceCodeManagement management{"RETURN a"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<Statement> statement = parser.parseStatement();
        ASSERT_TRUE(statement);
        ASSERT_EQ(*statement->reference(), "RETURN a");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"RETURN ?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<Statement> statement = parser.parseStatement();
        ASSERT_SRC_ERROR(statement, CodePosition(1, 8), "Unexpected character!", "?");
    }

    {
        SourceCodeManagement management{"a := 2"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<Statement> statement = parser.parseStatement();
        ASSERT_TRUE(statement);
        ASSERT_EQ(*statement->reference(), "a := 2");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"a = 2"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<Statement> statement = parser.parseStatement();
        ASSERT_SRC_ERROR(statement, CodePosition(1, 3), "Expected `:=` operator!", "=");
    }
    {
        SourceCodeManagement management{"a := ?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<Statement> statement = parser.parseStatement();
        ASSERT_SRC_ERROR(statement, CodePosition(1, 6), "Unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"2 := a"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<Statement> statement = parser.parseStatement();
        ASSERT_SRC_ERROR(statement, CodePosition(1, 1), "Expected begin of statement. Assignment or RETURN expression!", "2");
    }

    {
        SourceCodeManagement management{"?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<Statement> statement = parser.parseStatement();
        ASSERT_SRC_ERROR(statement, CodePosition(1, 1), "Unexpected character!", "?");
    }
}

TEST(Parser, testCompoundStatement) {
    {
        SourceCodeManagement management{"BEGIN\n"
                                        "  a := 2;\n"
                                        "  RETURN a\n"
                                        "END"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<CompoundStatement> statement = parser.parseCompoundStatement();
        ASSERT_TRUE(statement);
        ASSERT_EQ(*statement->reference(), "BEGIN\n  a := 2;\n  RETURN a\nEND");
        ASSERT_TRUE(lexer.endOfStream());
    }

    {
        SourceCodeManagement management{"BEGIN ?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<CompoundStatement> statement = parser.parseCompoundStatement();
        ASSERT_SRC_ERROR(statement, CodePosition(1, 7), "Unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"BEGIN a := 2 RETURN a END"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<CompoundStatement> statement = parser.parseCompoundStatement();
        ASSERT_SRC_ERROR(statement, CodePosition(1, 14), "Expected `;` to terminate statement!", "RETURN");
    }
    {
        SourceCodeManagement management{"BEGIN a := 2, RETURN a END"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<CompoundStatement> statement = parser.parseCompoundStatement();
        ASSERT_SRC_ERROR(statement, CodePosition(1, 13), "Expected `;` to terminate statement!", ",");
    }
    {
        SourceCodeManagement management{"BEGIN a := 2; RETURN *"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<CompoundStatement> statement = parser.parseCompoundStatement();
        ASSERT_SRC_ERROR(statement, CodePosition(1, 22), "Unexpected unary operator!", "*");
    }
    {
        SourceCodeManagement management{"END"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<CompoundStatement> statement = parser.parseCompoundStatement();
        ASSERT_SRC_ERROR(statement, CodePosition(1, 1), "Expected `BEGIN` keyword!", "END");
    }
    {
        SourceCodeManagement management{"BEGIN a := 1 RETURN"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<CompoundStatement> statement = parser.parseCompoundStatement();
        ASSERT_SRC_ERROR(statement, CodePosition(1, 14), "Expected `;` to terminate statement!", "RETURN");
    }
}

TEST(Parser, testParameterDeclarations) {
    {
        SourceCodeManagement management{"PARAM a, b, c;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ParameterDeclarations> declarations = parser.parseParameterDeclarations();
        ASSERT_TRUE(declarations);
        ASSERT_EQ(*declarations->reference(), "PARAM a, b, c;");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"PARAM a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ParameterDeclarations> declarations = parser.parseParameterDeclarations();
        ASSERT_TRUE(declarations);
        ASSERT_EQ(*declarations->reference(), "PARAM a;");
        ASSERT_TRUE(lexer.endOfStream());
    }

    {
        SourceCodeManagement management{"VAR a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ParameterDeclarations> declarations = parser.parseParameterDeclarations();
        ASSERT_SRC_ERROR(declarations, CodePosition(1, 1), "Expected `PARAM` keyword!", "VAR");
    }
    {
        SourceCodeManagement management{"PARAM a VAR asdf;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ParameterDeclarations> declarations = parser.parseParameterDeclarations();
        ASSERT_SRC_ERROR(declarations, CodePosition(1, 9), "Expected `;` to terminate PARAM declarations!", "VAR");
    }
    {
        SourceCodeManagement management{"PARAM 0;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ParameterDeclarations> declarations = parser.parseParameterDeclarations();
        ASSERT_SRC_ERROR(declarations, CodePosition(1, 7), "Expected an identifier!", "0");
    }
    {
        SourceCodeManagement management{"PARAM asdf, 0;\n"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ParameterDeclarations> declarations = parser.parseParameterDeclarations();
        ASSERT_SRC_ERROR(declarations, CodePosition(1, 13), "Expected an identifier!", "0");
    }
    {
        SourceCodeManagement management{"PARAM asdf ?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ParameterDeclarations> declarations = parser.parseParameterDeclarations();
        ASSERT_SRC_ERROR(declarations, CodePosition(1, 12), "Unexpected character!", "?");
    }
}

TEST(Parser, testVariableDeclarations) {
    {
        SourceCodeManagement management{"VAR a, b, c;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<VariableDeclarations> declarations = parser.parseVariableDeclarations();
        ASSERT_TRUE(declarations);
        ASSERT_EQ(*declarations->reference(), "VAR a, b, c;");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"VAR a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<VariableDeclarations> declarations = parser.parseVariableDeclarations();
        ASSERT_TRUE(declarations);
        ASSERT_EQ(*declarations->reference(), "VAR a;");
        ASSERT_TRUE(lexer.endOfStream());
    }

    {
        SourceCodeManagement management{"VAR 0;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<VariableDeclarations> declarations = parser.parseVariableDeclarations();
        ASSERT_SRC_ERROR(declarations, CodePosition(1, 5), "Expected an identifier!", "0");
    }
    {
        SourceCodeManagement management{"CONST a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<VariableDeclarations> declarations = parser.parseVariableDeclarations();
        ASSERT_SRC_ERROR(declarations, CodePosition(1, 1), "Expected `VAR` keyword!", "CONST");
    }
    {
        SourceCodeManagement management{"VAR a CONST asdf;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<VariableDeclarations> declarations = parser.parseVariableDeclarations();
        ASSERT_SRC_ERROR(declarations, CodePosition(1, 7), "Expected `;` to terminate VAR declarations!", "CONST");
    }
}

TEST(Parser, testConstantDecalrations) {
    {
        SourceCodeManagement management{"CONST a = 1, b = 2, c = 3;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ConstantDeclarations> declarations = parser.parseConstantDeclarations();
        ASSERT_TRUE(declarations);
        ASSERT_EQ(*declarations->reference(), "CONST a = 1, b = 2, c = 3;");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"CONST a = 1;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ConstantDeclarations> declarations = parser.parseConstantDeclarations();
        ASSERT_TRUE(declarations);
        ASSERT_EQ(*declarations->reference(), "CONST a = 1;");
        ASSERT_TRUE(lexer.endOfStream());
    }

    {
        SourceCodeManagement management{"CONST a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ConstantDeclarations> declarations = parser.parseConstantDeclarations();
        ASSERT_SRC_ERROR(declarations, CodePosition(1, 8), "Expected `=` operator!", ";");
    }
    {
        SourceCodeManagement management{"PARAM a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ConstantDeclarations> declarations = parser.parseConstantDeclarations();
        ASSERT_SRC_ERROR(declarations, CodePosition(1, 1), "Expected `CONST` keyword!", "PARAM");
    }
    {
        SourceCodeManagement management{"CONST a = 1 VAR asdf;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ConstantDeclarations> declarations = parser.parseConstantDeclarations();
        ASSERT_SRC_ERROR(declarations, CodePosition(1, 13), "Expected `;` to terminate CONST declarations!", "VAR");
    }
    {
        SourceCodeManagement management{"CONST 0 = a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ConstantDeclarations> declarations = parser.parseConstantDeclarations();
        ASSERT_SRC_ERROR(declarations, CodePosition(1, 7), "Expected an identifier!", "0");
    }
    {
        SourceCodeManagement management{"CONST asdf = a;\n"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ConstantDeclarations> declarations = parser.parseConstantDeclarations();
        ASSERT_SRC_ERROR(declarations, CodePosition(1, 14), "Expected literal!", "a");
    }
    {
        SourceCodeManagement management{"CONST asdf = 0, 0;\n"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ConstantDeclarations> declarations = parser.parseConstantDeclarations();
        ASSERT_SRC_ERROR(declarations, CodePosition(1, 17), "Expected an identifier!", "0");
    }
    {
        SourceCodeManagement management{"CONST asdf = 0 ?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<ConstantDeclarations> declarations = parser.parseConstantDeclarations();
        ASSERT_SRC_ERROR(declarations, CodePosition(1, 16), "Unexpected character!", "?");
    }
}

TEST(Parser, testParseProgram) {
    {
        SourceCodeManagement management{"PARAM width, height, depth;\n"
                                        "VAR volume, tmp;\n"
                                        "CONST density = 2400, one = 1;\n"
                                        "BEGIN\n"
                                        "\ttmp := density + one - 1;\n"
                                        "\tvolume := +width * height * (depth);\n"
                                        "\tRETURN tmp * volume\n"
                                        "END."};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_TRUE(program.isSuccess());

        ASSERT_PARSE_TREE(*program, "graph {\n"
                                    "  n_1 [label=\"function-definition\",shape=box];\n"
                                    "  n_1 -- n_2;\n"
                                    "  n_2 [label=\"parameter-declarations\",shape=box];\n"
                                    "  n_2 -- n_3;\n"
                                    "  n_3 [label=\"\\\"PARAM\\\"\"];\n"
                                    "  n_2 -- n_4;\n"
                                    "  n_4 [label=\"declarator-list\",shape=box];\n"
                                    "  n_4 -- n_5;\n"
                                    "  n_5 [label=\"\\\"width\\\"\"];\n"
                                    "  n_4 -- n_6;\n"
                                    "  n_6 [label=\"\\\",\\\"\"];\n"
                                    "  n_4 -- n_7;\n"
                                    "  n_7 [label=\"\\\"height\\\"\"];\n"
                                    "  n_4 -- n_8;\n"
                                    "  n_8 [label=\"\\\",\\\"\"];\n"
                                    "  n_4 -- n_9;\n"
                                    "  n_9 [label=\"\\\"depth\\\"\"];\n"
                                    "  n_2 -- n_10;\n"
                                    "  n_10 [label=\"\\\";\\\"\"];\n"
                                    "  n_1 -- n_11;\n"
                                    "  n_11 [label=\"variable-declarations\",shape=box];\n"
                                    "  n_11 -- n_12;\n"
                                    "  n_12 [label=\"\\\"VAR\\\"\"];\n"
                                    "  n_11 -- n_13;\n"
                                    "  n_13 [label=\"declarator-list\",shape=box];\n"
                                    "  n_13 -- n_14;\n"
                                    "  n_14 [label=\"\\\"volume\\\"\"];\n"
                                    "  n_13 -- n_15;\n"
                                    "  n_15 [label=\"\\\",\\\"\"];\n"
                                    "  n_13 -- n_16;\n"
                                    "  n_16 [label=\"\\\"tmp\\\"\"];\n"
                                    "  n_11 -- n_17;\n"
                                    "  n_17 [label=\"\\\";\\\"\"];\n"
                                    "  n_1 -- n_18;\n"
                                    "  n_18 [label=\"constant-declarations\",shape=box];\n"
                                    "  n_18 -- n_19;\n"
                                    "  n_19 [label=\"\\\"CONST\\\"\"];\n"
                                    "  n_18 -- n_20;\n"
                                    "  n_20 [label=\"init-declarator-list\",shape=box];\n"
                                    "  n_20 -- n_21;\n"
                                    "  n_21 [label=\"init-declarator\",shape=box];\n"
                                    "  n_21 -- n_22;\n"
                                    "  n_22 [label=\"\\\"density\\\"\"];\n"
                                    "  n_21 -- n_23;\n"
                                    "  n_23 [label=\"\\\"=\\\"\"];\n"
                                    "  n_21 -- n_24;\n"
                                    "  n_24[label=\"2400\"];\n"
                                    "  n_20 -- n_25;\n"
                                    "  n_25 [label=\"\\\",\\\"\"];\n"
                                    "  n_20 -- n_26;\n"
                                    "  n_26 [label=\"init-declarator\",shape=box];\n"
                                    "  n_26 -- n_27;\n"
                                    "  n_27 [label=\"\\\"one\\\"\"];\n"
                                    "  n_26 -- n_28;\n"
                                    "  n_28 [label=\"\\\"=\\\"\"];\n"
                                    "  n_26 -- n_29;\n"
                                    "  n_29[label=\"1\"];\n"
                                    "  n_18 -- n_30;\n"
                                    "  n_30 [label=\"\\\";\\\"\"];\n"
                                    "  n_1 -- n_31;\n"
                                    "  n_31 [label=\"compound-statement\",shape=box];\n"
                                    "  n_31 -- n_32;\n"
                                    "  n_32 [label=\"\\\"BEGIN\\\"\"];\n"
                                    "  n_31 -- n_33;\n"
                                    "  n_33 [label=\"statement-list\",shape=box];\n"
                                    "  n_33 -- n_34;\n"
                                    "  n_34 [label=\"statement\",shape=box];\n"
                                    "  n_34 -- n_35;\n"
                                    "  n_35 [label=\"assignment-expression\",shape=box];\n"
                                    "  n_35 -- n_36;\n"
                                    "  n_36 [label=\"\\\"tmp\\\"\"];\n"
                                    "  n_35 -- n_37;\n"
                                    "  n_37 [label=\"\\\":=\\\"\"];\n"
                                    "  n_35 -- n_38;\n"
                                    "  n_38 [label=\"additive-expression\",shape=box];\n"
                                    "  n_38 -- n_39;\n"
                                    "  n_39 [label=\"multiplicative-expression\",shape=box];\n"
                                    "  n_39 -- n_40;\n"
                                    "  n_40 [label=\"unary-expression\",shape=box];\n"
                                    "  n_40 -- n_41;\n"
                                    "  n_41 [label=\"primary-expression\",shape=box];\n"
                                    "  n_41 -- n_42;\n"
                                    "  n_42 [label=\"\\\"density\\\"\"];\n"
                                    "  n_38 -- n_43;\n"
                                    "  n_43 [label=\"\\\"+\\\"\"];\n"
                                    "  n_38 -- n_44;\n"
                                    "  n_44 [label=\"additive-expression\",shape=box];\n"
                                    "  n_44 -- n_45;\n"
                                    "  n_45 [label=\"multiplicative-expression\",shape=box];\n"
                                    "  n_45 -- n_46;\n"
                                    "  n_46 [label=\"unary-expression\",shape=box];\n"
                                    "  n_46 -- n_47;\n"
                                    "  n_47 [label=\"primary-expression\",shape=box];\n"
                                    "  n_47 -- n_48;\n"
                                    "  n_48 [label=\"\\\"one\\\"\"];\n"
                                    "  n_44 -- n_49;\n"
                                    "  n_49 [label=\"\\\"-\\\"\"];\n"
                                    "  n_44 -- n_50;\n"
                                    "  n_50 [label=\"additive-expression\",shape=box];\n"
                                    "  n_50 -- n_51;\n"
                                    "  n_51 [label=\"multiplicative-expression\",shape=box];\n"
                                    "  n_51 -- n_52;\n"
                                    "  n_52 [label=\"unary-expression\",shape=box];\n"
                                    "  n_52 -- n_53;\n"
                                    "  n_53 [label=\"primary-expression\",shape=box];\n"
                                    "  n_53 -- n_54;\n"
                                    "  n_54[label=\"1\"];\n"
                                    "  n_33 -- n_55;\n"
                                    "  n_55 [label=\"\\\";\\\"\"];\n"
                                    "  n_33 -- n_56;\n"
                                    "  n_56 [label=\"statement\",shape=box];\n"
                                    "  n_56 -- n_57;\n"
                                    "  n_57 [label=\"assignment-expression\",shape=box];\n"
                                    "  n_57 -- n_58;\n"
                                    "  n_58 [label=\"\\\"volume\\\"\"];\n"
                                    "  n_57 -- n_59;\n"
                                    "  n_59 [label=\"\\\":=\\\"\"];\n"
                                    "  n_57 -- n_60;\n"
                                    "  n_60 [label=\"additive-expression\",shape=box];\n"
                                    "  n_60 -- n_61;\n"
                                    "  n_61 [label=\"multiplicative-expression\",shape=box];\n"
                                    "  n_61 -- n_62;\n"
                                    "  n_62 [label=\"unary-expression\",shape=box];\n"
                                    "  n_62 -- n_63;\n"
                                    "  n_63 [label=\"\\\"+\\\"\"];\n"
                                    "  n_62 -- n_64;\n"
                                    "  n_64 [label=\"primary-expression\",shape=box];\n"
                                    "  n_64 -- n_65;\n"
                                    "  n_65 [label=\"\\\"width\\\"\"];\n"
                                    "  n_61 -- n_66;\n"
                                    "  n_66 [label=\"\\\"*\\\"\"];\n"
                                    "  n_61 -- n_67;\n"
                                    "  n_67 [label=\"multiplicative-expression\",shape=box];\n"
                                    "  n_67 -- n_68;\n"
                                    "  n_68 [label=\"unary-expression\",shape=box];\n"
                                    "  n_68 -- n_69;\n"
                                    "  n_69 [label=\"primary-expression\",shape=box];\n"
                                    "  n_69 -- n_70;\n"
                                    "  n_70 [label=\"\\\"height\\\"\"];\n"
                                    "  n_67 -- n_71;\n"
                                    "  n_71 [label=\"\\\"*\\\"\"];\n"
                                    "  n_67 -- n_72;\n"
                                    "  n_72 [label=\"multiplicative-expression\",shape=box];\n"
                                    "  n_72 -- n_73;\n"
                                    "  n_73 [label=\"unary-expression\",shape=box];\n"
                                    "  n_73 -- n_74;\n"
                                    "  n_74 [label=\"primary-expression\",shape=box];\n"
                                    "  n_74 -- n_75;\n"
                                    "  n_75 [label=\"\\\"(\\\"\"];\n"
                                    "  n_74 -- n_76;\n"
                                    "  n_76 [label=\"additive-expression\",shape=box];\n"
                                    "  n_76 -- n_77;\n"
                                    "  n_77 [label=\"multiplicative-expression\",shape=box];\n"
                                    "  n_77 -- n_78;\n"
                                    "  n_78 [label=\"unary-expression\",shape=box];\n"
                                    "  n_78 -- n_79;\n"
                                    "  n_79 [label=\"primary-expression\",shape=box];\n"
                                    "  n_79 -- n_80;\n"
                                    "  n_80 [label=\"\\\"depth\\\"\"];\n"
                                    "  n_74 -- n_81;\n"
                                    "  n_81 [label=\"\\\")\\\"\"];\n"
                                    "  n_33 -- n_82;\n"
                                    "  n_82 [label=\"\\\";\\\"\"];\n"
                                    "  n_33 -- n_83;\n"
                                    "  n_83 [label=\"statement\",shape=box];\n"
                                    "  n_83 -- n_84;\n"
                                    "  n_84 [label=\"\\\"RETURN\\\"\"];\n"
                                    "  n_83 -- n_85;\n"
                                    "  n_85 [label=\"additive-expression\",shape=box];\n"
                                    "  n_85 -- n_86;\n"
                                    "  n_86 [label=\"multiplicative-expression\",shape=box];\n"
                                    "  n_86 -- n_87;\n"
                                    "  n_87 [label=\"unary-expression\",shape=box];\n"
                                    "  n_87 -- n_88;\n"
                                    "  n_88 [label=\"primary-expression\",shape=box];\n"
                                    "  n_88 -- n_89;\n"
                                    "  n_89 [label=\"\\\"tmp\\\"\"];\n"
                                    "  n_86 -- n_90;\n"
                                    "  n_90 [label=\"\\\"*\\\"\"];\n"
                                    "  n_86 -- n_91;\n"
                                    "  n_91 [label=\"multiplicative-expression\",shape=box];\n"
                                    "  n_91 -- n_92;\n"
                                    "  n_92 [label=\"unary-expression\",shape=box];\n"
                                    "  n_92 -- n_93;\n"
                                    "  n_93 [label=\"primary-expression\",shape=box];\n"
                                    "  n_93 -- n_94;\n"
                                    "  n_94 [label=\"\\\"volume\\\"\"];\n"
                                    "  n_31 -- n_95;\n"
                                    "  n_95 [label=\"\\\"END\\\"\"];\n"
                                    "}\n");
    }
    {
        SourceCodeManagement management{"BEGIN\n"
                                        "  RETURN 0\n"
                                        "END."};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_TRUE(program.isSuccess());

        ASSERT_PARSE_TREE(*program, "graph {\n"
                                    "  n_1 [label=\"function-definition\",shape=box];\n"
                                    "  n_1 -- n_2;\n"
                                    "  n_2 [label=\"compound-statement\",shape=box];\n"
                                    "  n_2 -- n_3;\n"
                                    "  n_3 [label=\"\\\"BEGIN\\\"\"];\n"
                                    "  n_2 -- n_4;\n"
                                    "  n_4 [label=\"statement-list\",shape=box];\n"
                                    "  n_4 -- n_5;\n"
                                    "  n_5 [label=\"statement\",shape=box];\n"
                                    "  n_5 -- n_6;\n"
                                    "  n_6 [label=\"\\\"RETURN\\\"\"];\n"
                                    "  n_5 -- n_7;\n"
                                    "  n_7 [label=\"additive-expression\",shape=box];\n"
                                    "  n_7 -- n_8;\n"
                                    "  n_8 [label=\"multiplicative-expression\",shape=box];\n"
                                    "  n_8 -- n_9;\n"
                                    "  n_9 [label=\"unary-expression\",shape=box];\n"
                                    "  n_9 -- n_10;\n"
                                    "  n_10 [label=\"primary-expression\",shape=box];\n"
                                    "  n_10 -- n_11;\n"
                                    "  n_11[label=\"0\"];\n"
                                    "  n_2 -- n_12;\n"
                                    "  n_12 [label=\"\\\"END\\\"\"];\n"
                                    "}\n");
        ASSERT_EQ(*program->reference(), "BEGIN\n  RETURN 0\nEND.");
    }

    {
        SourceCodeManagement management{"END ."};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 1), "Expected `BEGIN` keyword!", "END");
    }
    {
        SourceCodeManagement management{"BEGIN\n"
                                        "  RETURN 0\n"
                                        "END;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(3, 4), "Expected `.` terminator!", ";");
    }
    {
        SourceCodeManagement management{"BEGIN\n"
                                        "  RETURN 0\n"
                                        "END.\n"
                                        "RETURN 1"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(4, 1), "unexpected character after end of program terminator!", "R");
    }
    {
        SourceCodeManagement management{"BEGIN\n"
                                        "  RETURN 0\n"
                                        "END ?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(3, 5), "Unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 1), "Unexpected character!", "?");
    }

    {
        SourceCodeManagement management{"PARAM a;\n"
                                        "?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(2, 1), "Unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"PARAM a;\n"
                                        "VAR b;\n"
                                        "?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(3, 1), "Unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"PARAM a;\n"
                                        "VAR b;\n"
                                        "CONST c = 0;\n"
                                        "?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(4, 1), "Unexpected character!", "?");
    }


    {
        SourceCodeManagement management{"PARAM 0;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 7), "Expected an identifier!", "0");
    }
    {
        SourceCodeManagement management{"VAR 0;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 5), "Expected an identifier!", "0");
    }
    {
        SourceCodeManagement management{"CONST 0 = 0;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 7), "Expected an identifier!", "0");
    }
}

TEST(Parser, testDeclarationsOrderingAndRedeclarations) {
    {
        SourceCodeManagement management{"PARAM a;\n"
                                        "PARAM a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(2, 1), "Duplicate PARAM declaration!", "PARAM");
    }
    {
        SourceCodeManagement management{"PARAM a;\n"
                                        "VAR a;\n"
                                        "PARAM a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(3, 1), "Duplicate PARAM declaration!", "PARAM");
    }
    {
        SourceCodeManagement management{"PARAM a;\n"
                                        "VAR a;\n"
                                        "CONST a = 0;\n"
                                        "PARAM a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(4, 1), "Duplicate PARAM declaration!", "PARAM");
    }
    {
        SourceCodeManagement management{"PARAM a;\n"
                                        "CONST a = 0;\n"
                                        "PARAM a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(3, 1), "Duplicate PARAM declaration!", "PARAM");
    }
    {
        SourceCodeManagement management{"VAR a;\n"
                                        "PARAM a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(2, 1), "PARAM declaration must appear before VAR declaration!", "PARAM");
    }
    {
        SourceCodeManagement management{"CONST a = 0;\n"
                                        "PARAM a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(2, 1), "PARAM declaration must appear before CONST and VAR declarations!", "PARAM");
    }

    {
        SourceCodeManagement management{"VAR a;\n"
                                        "VAR a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(2, 1), "Duplicate VAR declaration!", "VAR");
    }
    {
        SourceCodeManagement management{"VAR a;\n"
                                        "CONST a = 0;\n"
                                        "VAR a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(3, 1), "Duplicate VAR declaration!", "VAR");
    }
    {
        SourceCodeManagement management{"CONST a = 0;\n"
                                        "VAR a;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(2, 1), "VAR declaration must appear before CONST declaration!", "VAR");
    }

    {
        SourceCodeManagement management{"CONST a = 0;\n"
                                        "CONST a = 0;"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(2, 1), "Duplicate CONST declaration!", "CONST");
    }
}
//---------------------------------------------------------------------------
