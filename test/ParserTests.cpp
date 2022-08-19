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
    SourceCodeManagement management{"BEGIN"};
    Lexer lexer{management};
    Parser parser{lexer};

    Result<GenericTerminal> terminal = parser.parseGenericTerminal(Token::Type::KEYWORD, Keyword::BEGIN, "expected BEGIN");
    ASSERT_TRUE(terminal);
    ASSERT_EQ(terminal->value(), "BEGIN");
    ASSERT_TRUE(lexer.endOfStream());
}

TEST(Parser, testLiteral) {
    SourceCodeManagement management{"02731"};
    Lexer lexer{management};
    Parser parser{lexer};

    Result<Literal> literal = parser.parseLiteral();
    ASSERT_TRUE(literal);
    ASSERT_EQ(literal->value(), 2731);
    ASSERT_TRUE(lexer.endOfStream());
}

TEST(Parser, testIdentifier) {
    SourceCodeManagement management{"Program"};
    Lexer lexer{management};
    Parser parser{lexer};

    Result<Identifier> identifier = parser.parseIdentifier();
    ASSERT_TRUE(identifier);
    ASSERT_EQ(identifier->value(), "Program");
    ASSERT_TRUE(lexer.endOfStream());
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
}

TEST(Parser, testExampleProgram) {
    SourceCodeManagement management{ "PARAM width, height, depth;\n"
                                      "VAR volume;\n"
                                      "CONST density = 2400;\n"
                                      "BEGIN\n"
                                      "\tvolume := width * height * depth;\n"
                                      "\tRETURN density * volume\n"
                                      "END." };
    Lexer lexer{ management };
    Parser parser{lexer};

    auto program = parser.parse_program();
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
                               "  n_11 -- n_15;\n"
                               "  n_15 [label=\"\\\";\\\"\"];\n"
                               "  n_1 -- n_16;\n"
                               "  n_16 [label=\"constant-declarations\",shape=box];\n"
                               "  n_16 -- n_17;\n"
                               "  n_17 [label=\"\\\"CONST\\\"\"];\n"
                               "  n_16 -- n_18;\n"
                               "  n_18 [label=\"init-declarator-list\",shape=box];\n"
                               "  n_18 -- n_19;\n"
                               "  n_19 [label=\"init-declarator\",shape=box];\n"
                               "  n_19 -- n_20;\n"
                               "  n_20 [label=\"\\\"density\\\"\"];\n"
                               "  n_19 -- n_21;\n"
                               "  n_21 [label=\"\\\"=\\\"\"];\n"
                               "  n_19 -- n_22;\n"
                               "  n_22[label=\"2400\"];\n"
                               "  n_16 -- n_23;\n"
                               "  n_23 [label=\"\\\";\\\"\"];\n"
                               "  n_1 -- n_24;\n"
                               "  n_24 [label=\"compound-statement\",shape=box];\n"
                               "  n_24 -- n_25;\n"
                               "  n_25 [label=\"\\\"BEGIN\\\"\"];\n"
                               "  n_24 -- n_26;\n"
                               "  n_26 [label=\"statement-list\",shape=box];\n"
                               "  n_26 -- n_27;\n"
                               "  n_27 [label=\"statement\",shape=box];\n"
                               "  n_27 -- n_28;\n"
                               "  n_28 [label=\"assignment-expression\",shape=box];\n"
                               "  n_28 -- n_29;\n"
                               "  n_29 [label=\"\\\"volume\\\"\"];\n"
                               "  n_28 -- n_30;\n"
                               "  n_30 [label=\"\\\":=\\\"\"];\n"
                               "  n_28 -- n_31;\n"
                               "  n_31 [label=\"additive-expression\",shape=box];\n"
                               "  n_31 -- n_32;\n"
                               "  n_32 [label=\"multiplicative-expression\",shape=box];\n"
                               "  n_32 -- n_33;\n"
                               "  n_33 [label=\"unary-expression\",shape=box];\n"
                               "  n_33 -- n_34;\n"
                               "  n_34 [label=\"primary-expression\",shape=box];\n"
                               "  n_34 -- n_35;\n"
                               "  n_35 [label=\"\\\"width\\\"\"];\n"
                               "  n_32 -- n_36;\n"
                               "  n_36 [label=\"\\\"*\\\"\"];\n"
                               "  n_32 -- n_37;\n"
                               "  n_37 [label=\"multiplicative-expression\",shape=box];\n"
                               "  n_37 -- n_38;\n"
                               "  n_38 [label=\"unary-expression\",shape=box];\n"
                               "  n_38 -- n_39;\n"
                               "  n_39 [label=\"primary-expression\",shape=box];\n"
                               "  n_39 -- n_40;\n"
                               "  n_40 [label=\"\\\"height\\\"\"];\n"
                               "  n_37 -- n_41;\n"
                               "  n_41 [label=\"\\\"*\\\"\"];\n"
                               "  n_37 -- n_42;\n"
                               "  n_42 [label=\"multiplicative-expression\",shape=box];\n"
                               "  n_42 -- n_43;\n"
                               "  n_43 [label=\"unary-expression\",shape=box];\n"
                               "  n_43 -- n_44;\n"
                               "  n_44 [label=\"primary-expression\",shape=box];\n"
                               "  n_44 -- n_45;\n"
                               "  n_45 [label=\"\\\"depth\\\"\"];\n"
                               "  n_26 -- n_46;\n"
                               "  n_46 [label=\"\\\";\\\"\"];\n"
                               "  n_26 -- n_47;\n"
                               "  n_47 [label=\"statement\",shape=box];\n"
                               "  n_47 -- n_48;\n"
                               "  n_48 [label=\"\\\"RETURN\\\"\"];\n"
                               "  n_47 -- n_49;\n"
                               "  n_49 [label=\"additive-expression\",shape=box];\n"
                               "  n_49 -- n_50;\n"
                               "  n_50 [label=\"multiplicative-expression\",shape=box];\n"
                               "  n_50 -- n_51;\n"
                               "  n_51 [label=\"unary-expression\",shape=box];\n"
                               "  n_51 -- n_52;\n"
                               "  n_52 [label=\"primary-expression\",shape=box];\n"
                               "  n_52 -- n_53;\n"
                               "  n_53 [label=\"\\\"density\\\"\"];\n"
                               "  n_50 -- n_54;\n"
                               "  n_54 [label=\"\\\"*\\\"\"];\n"
                               "  n_50 -- n_55;\n"
                               "  n_55 [label=\"multiplicative-expression\",shape=box];\n"
                               "  n_55 -- n_56;\n"
                               "  n_56 [label=\"unary-expression\",shape=box];\n"
                               "  n_56 -- n_57;\n"
                               "  n_57 [label=\"primary-expression\",shape=box];\n"
                               "  n_57 -- n_58;\n"
                               "  n_58 [label=\"\\\"volume\\\"\"];\n"
                               "  n_24 -- n_59;\n"
                               "  n_59 [label=\"\\\"END\\\"\"];\n"
                               "}\n");
}

TEST(Parser, testParsingErrorFunction) {
    {
        SourceCodeManagement management{"?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 1), "unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"PARAM asdf;\n"
                                        "?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(2, 1), "unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"PARAM asdf;\n"
                                        "VAR asdff;\n"
                                        "?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(3, 1), "unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"PARAM asdf;\n"
                                        "VAR asdff;\n"
                                        "CONST asdfff\n"
                                        "?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(4, 1), "unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"END ."};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 1), "Expected `BEGIN` keyword!", "END");
    }
    {
        SourceCodeManagement management{"CONST asdf = 2;\n"
                                        "PARAM asdff;\n"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(2, 1), "Expected `BEGIN` keyword!", "PARAM");
    }
    {
        SourceCodeManagement management{"BEGIN\n"
                                        "  RETURN 0\n"
                                        "END 1"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(3, 5), "Expected `.` terminator!", "1");
    }
    {
        SourceCodeManagement management{"BEGIN\n"
                                        "  RETURN 0\n"
                                        "END ?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(3, 5), "unexpected character!", "?");
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
}

TEST(Parser, testParsingErrorPARAM) {
    // TODO direct call to test param keyword!
    {
        SourceCodeManagement management{"PARAM ?"};
        Lexer lexer{management};
        Parser parser{lexer};

        (void) parser;
        // TODO Result<FunctionDefinition> program = parser.parseParameterDeclarations();
        // TODO ASSERT_SRC_ERROR(program, CodePosition(1, 7), "unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"PARAM ?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 7), "unexpected character!", "?");
    }
    {
        SourceCodeManagement management{"PARAM asdf\n"
                                        "VAR asdff;\n"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(2, 1), "Expected `;` separator!", "VAR");
    }
    {
        SourceCodeManagement management{"PARAM asdf, 0;\n"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 13), "Expected an identifier!", "0");
    }
    {
        SourceCodeManagement management{"PARAM asdf ?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 12), "unexpected character!", "?");
    }
}

TEST(Parser, testParsingErrorVAR) {
    // TODO direct call!
    {
        SourceCodeManagement management{"VAR asdf, 0;\n"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 11), "Expected an identifier!", "0");
    }
    {
        SourceCodeManagement management{"VAR asdf ?"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 10), "unexpected character!", "?");
    }
}

TEST(Parser, testParsingErrorCONST) {
    // TODO direct call!
    {
        SourceCodeManagement management{"VAR asdf, 0;\n"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 11), "Expected an identifier!", "0");
    }
    {
        SourceCodeManagement management{"CONST 0 = a;\n"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 7), "Expected an identifier!", "0");
    }
    {
        SourceCodeManagement management{"CONST asdf = a;\n"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 14), "Expected literal!", "a");
    }
    {
        SourceCodeManagement management{"CONST asdf = 18446744073709551616;\n"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(1, 14), "Integer literal is out of range. Expected singed 64-bit!", "18446744073709551616");
    }
    {
        SourceCodeManagement management{"CONST a = 1, b = 2\n"};
        Lexer lexer{management};
        Parser parser{lexer};

        Result<FunctionDefinition> program = parser.parse_program();
        ASSERT_SRC_ERROR(program, CodePosition(2, 1), "unexpected end of stream!", "");
    }
}
//---------------------------------------------------------------------------
