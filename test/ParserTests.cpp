#include "pljit/Lexer.hpp"
#include "pljit/ParseTreeDOTVisitor.hpp"
#include "pljit/Parser.hpp"
#include "pljit/SourceCodeManagement.hpp"
#include "utils/CaptureCOut.hpp"
#include <gtest/gtest.h>
//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
TEST(Parser, testGenericTerminal) {
    SourceCodeManagement management{"BEGIN"};
    Lexer lexer{management};
    Parser parser{lexer};

    ParseTree::GenericTerminal terminal;
    ASSERT_FALSE(parser.parseGenericTerminal(terminal, Token::TokenType::KEYWORD, Keyword::BEGIN, "expected BEGIN").has_value());
    ASSERT_EQ(terminal.value(), "BEGIN");
    ASSERT_TRUE(lexer.endOfStream());
}

TEST(Parser, testLiteral) {
    SourceCodeManagement management{"02731"};
    Lexer lexer{management};
    Parser parser{lexer};

    ParseTree::Literal literal;
    ASSERT_FALSE(parser.parseLiteral(literal).has_value());
    ASSERT_EQ(literal.value(), 2731);
    ASSERT_TRUE(lexer.endOfStream());
}

TEST(Parser, testIdentifier) {
    SourceCodeManagement management{"Program"};
    Lexer lexer{management};
    Parser parser{lexer};

    ParseTree::Identifier identifier;
    ASSERT_FALSE(parser.parseIdentifier(identifier).has_value());
    ASSERT_EQ(identifier.value(), "Program");
    ASSERT_TRUE(lexer.endOfStream());
}

TEST(Parser, testPrimaryExpression) {
    {
        SourceCodeManagement management{"width"};
        Lexer lexer{management};
        Parser parser{lexer};

        ParseTree::PrimaryExpression expression;
        ASSERT_FALSE(parser.parsePrimaryExpression(expression).has_value());
        ASSERT_EQ(expression.asIdentifier().value(), "width");
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"12"};
        Lexer lexer{management};
        Parser parser{lexer};

        ParseTree::PrimaryExpression expression;
        ASSERT_FALSE(parser.parsePrimaryExpression(expression).has_value());
        ASSERT_EQ(expression.asLiteral().value(), 12);
        ASSERT_TRUE(lexer.endOfStream());
    }
    {
        SourceCodeManagement management{"(12)"};
        Lexer lexer{management};
        Parser parser{lexer};

        ParseTree::PrimaryExpression expression;
        ASSERT_FALSE(parser.parsePrimaryExpression(expression).has_value());
        auto [openParenthesis, additiveExpression, closeParenthesis] = expression.asBracketedExpression();
        ASSERT_EQ(openParenthesis.value(), "(");
        // TODO make this nicer to test!
        ASSERT_EQ(additiveExpression.getExpression().getExpression().getPrimaryExpression().asLiteral().value(), 12);
        ASSERT_EQ(closeParenthesis.value(), ")");
        ASSERT_TRUE(lexer.endOfStream());
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
    ParseTree::DOTVisitor visitor;

    CaptureCOut capture;

    program->accept(visitor);

    ASSERT_EQ(
        capture.str(),
        "graph {\n"
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
        "}"
    );
}
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------
