//
// Created by Andreas Bauer on 12.08.22.
//

#include "pljit/ast/AST.hpp"
#include "pljit/ast/ASTDOTVisitor.hpp"
#include "pljit/lex/Lexer.hpp"
#include "pljit/parse/Parser.hpp"
#include "test/utils/ast_utils.hpp"
#include "utils/CaptureCOut.hpp"
#include <gtest/gtest.h>

//---------------------------------------------------------------------------
using namespace pljit;
using namespace pljit::code;
using namespace pljit::parse;
using namespace pljit::lex;
using namespace pljit::ast;
//---------------------------------------------------------------------------
TEST(AST, testExampleProgram) {
    SourceCodeManagement management{"PARAM width, height, depth;\n"
                                    "VAR volume;\n"
                                    "CONST density = 2400;\n"
                                    "BEGIN\n"
                                    "  volume := width * height * depth;\n"
                                    "  RETURN density * volume\n"
                                    "END."};
    Result<Function> function = buildAST(management);
    if (function.isFailure()) {
        function.error().printCompilerError();
    }
    ASSERT_TRUE(function.isSuccess());

    DOTVisitor visitor;
    CaptureCOut capture;

    visitor.print(*function);

    EXPECT_EQ(
        capture.str(),
        "graph {\n"
        "  n_1 [label=\"Function\",shape=box];\n"
        "  n_1 -- n_2;\n"
        "  n_2 [label=\"ParamDeclaration\",shape=box];\n"
        "  n_2 -- n_3;\n"
        "  n_3 [label=\"\\\"width\\\"\"];\n"
        "  n_2 -- n_4;\n"
        "  n_4 [label=\"\\\"height\\\"\"];\n"
        "  n_2 -- n_5;\n"
        "  n_5 [label=\"\\\"depth\\\"\"];\n"
        "  n_1 -- n_6;\n"
        "  n_6 [label=\"VarDeclaration\",shape=box];\n"
        "  n_6 -- n_7;\n"
        "  n_7 [label=\"\\\"volume\\\"\"];\n"
        "  n_1 -- n_8;\n"
        "  n_8 [label=\"ConstDeclaration\",shape=box];\n"
        "  n_8 -- n_9;\n"
        "  n_9 [label=\"\\\"density\\\"\"];\n"
        "  n_8 -- n_10;\n"
        "  n_10[label=\"2400\"];\n"
        "  n_1 -- n_11;\n"
        "  n_11 [label=\"AssignmentStatement\",shape=box];\n"
        "  n_11 -- n_12;\n"
        "  n_12 [label=\"\\\"volume\\\"\"];\n"
        "  n_11 -- n_13;\n"
        "  n_13 [label=\"Multiply\",shape=box];\n"
        "  n_13 -- n_14;\n"
        "  n_14 [label=\"\\\"width\\\"\"];\n"
        "  n_13 -- n_15;\n"
        "  n_15 [label=\"Multiply\",shape=box];\n"
        "  n_15 -- n_16;\n"
        "  n_16 [label=\"\\\"height\\\"\"];\n"
        "  n_15 -- n_17;\n"
        "  n_17 [label=\"\\\"depth\\\"\"];\n"
        "  n_1 -- n_18;\n"
        "  n_18 [label=\"ReturnStatement\",shape=box];\n"
        "  n_18 -- n_19;\n"
        "  n_19 [label=\"Multiply\",shape=box];\n"
        "  n_19 -- n_20;\n"
        "  n_20 [label=\"\\\"density\\\"\"];\n"
        "  n_19 -- n_21;\n"
        "  n_21 [label=\"\\\"volume\\\"\"];\n"
        "}\n"
    );

    auto result = function->evaluate({100, 100, 100});
    ASSERT_TRUE(result.return_value());
    ASSERT_EQ(*result.return_value(), 2400000000);
}

TEST(AST, testSmallProgram) {
    SourceCodeManagement management{"PARAM a;\n"
                                    "VAR b;\n"
                                    "BEGIN\n"
                                    "  b := (+a - -a) + a / a;\n"
                                    "  RETURN b\n"
                                    "END."};
    Result<Function> function = buildAST(management);
    if (function.isFailure()) {
        function.error().printCompilerError();
    }
    ASSERT_TRUE(function.isSuccess());

    DOTVisitor visitor;
    CaptureCOut capture;
    visitor.print(*function);
    EXPECT_EQ(
        capture.str(),
        "graph {\n"
        "  n_1 [label=\"Function\",shape=box];\n"
        "  n_1 -- n_2;\n"
        "  n_2 [label=\"ParamDeclaration\",shape=box];\n"
        "  n_2 -- n_3;\n"
        "  n_3 [label=\"\\\"a\\\"\"];\n"
        "  n_1 -- n_4;\n"
        "  n_4 [label=\"VarDeclaration\",shape=box];\n"
        "  n_4 -- n_5;\n"
        "  n_5 [label=\"\\\"b\\\"\"];\n"
        "  n_1 -- n_6;\n"
        "  n_6 [label=\"AssignmentStatement\",shape=box];\n"
        "  n_6 -- n_7;\n"
        "  n_7 [label=\"\\\"b\\\"\"];\n"
        "  n_6 -- n_8;\n"
        "  n_8 [label=\"Add\",shape=box];\n"
        "  n_8 -- n_9;\n"
        "  n_9 [label=\"Subtract\",shape=box];\n"
        "  n_9 -- n_10;\n"
        "  n_10 [label=\"UnaryPlus\",shape=box];\n"
        "  n_10 -- n_11;\n"
        "  n_11 [label=\"\\\"a\\\"\"];\n"
        "  n_9 -- n_12;\n"
        "  n_12 [label=\"UnaryMinus\",shape=box];\n"
        "  n_12 -- n_13;\n"
        "  n_13 [label=\"\\\"a\\\"\"];\n"
        "  n_8 -- n_14;\n"
        "  n_14 [label=\"Divide\",shape=box];\n"
        "  n_14 -- n_15;\n"
        "  n_15 [label=\"\\\"a\\\"\"];\n"
        "  n_14 -- n_16;\n"
        "  n_16 [label=\"\\\"a\\\"\"];\n"
        "  n_1 -- n_17;\n"
        "  n_17 [label=\"ReturnStatement\",shape=box];\n"
        "  n_17 -- n_18;\n"
        "  n_18 [label=\"\\\"b\\\"\"];\n"
        "}\n"
    );

    auto result = function->evaluate({1});
    ASSERT_TRUE(result.return_value());
    ASSERT_EQ(*result.return_value(), 3);
}

TEST(AST, testArgumentCount) {
    {
        SourceCodeManagement management{"PARAM a;\n"
                                        "BEGIN\n"
                                        "  RETURN a\n"
                                        "END."};
        Result<Function> function = buildAST(management);
        ASSERT_TRUE(function.isSuccess());

        EvaluationContext result = function->evaluate({});
        ASSERT_TRUE(result.runtime_error());
        ASSERT_EQ(*result.runtime_error(), "Received to few arguments!");

        result = function->evaluate({1, 1});
        ASSERT_TRUE(result.runtime_error());
        ASSERT_EQ(*result.runtime_error(), "Received to many arguments!");
    }
    {
        SourceCodeManagement management{"BEGIN\n"
                                        "  RETURN 0\n"
                                        "END."};
        Result<Function> function = buildAST(management);
        ASSERT_TRUE(function.isSuccess());

        EvaluationContext result = function->evaluate({1, 1});
        ASSERT_TRUE(result.runtime_error());
        ASSERT_EQ(*result.runtime_error(), "Provided arguments to function with missing PARAM declaration!");
    }
}

TEST(AST, testDivisionByZero) {
    {
        SourceCodeManagement management{"BEGIN\n"
                                        "  RETURN +(-(1 + (1 - (1 * (1 / (1 / 0))))))\n"
                                        "END."};
        Result<Function> function = buildAST(management);
        ASSERT_TRUE(function.isSuccess());

        EvaluationContext result = function->evaluate({});
        ASSERT_TRUE(result.runtime_error());
        ASSERT_EQ(*result.runtime_error(), "Division by zero!");
    }
    {
        SourceCodeManagement management{"VAR a;\n"
                                        "BEGIN\n"
                                        "  a := ((((1 / 0) / 1) * 1) - 1) + 1;\n"
                                        "  RETURN a\n"
                                        "END."};
        Result<Function> function = buildAST(management);
        ASSERT_TRUE(function.isSuccess());

        EvaluationContext result = function->evaluate({});
        ASSERT_TRUE(result.runtime_error());
        ASSERT_EQ(*result.runtime_error(), "Division by zero!");
    }
}

TEST(AST, testMissingReturn) {
    SourceCodeManagement management{"VAR test;\n"
                                    "BEGIN\n"
                                    "  test := 0\n"
                                    "END."};
    Result<Function> function = buildAST(management);
    ASSERT_TRUE(function.isFailure());

    CaptureCOut capture;
    function.error().printCompilerError();

    ASSERT_EQ(
        capture.str(),
        "4:1: error: Reached end of function without a RETURN statement!\n"
        "END.\n"
        "^~~\n"
    );
}

TEST(AST, testUndeclaredIdentifierUsage) {
    {
        SourceCodeManagement management{"VAR test;\n"
                                        "BEGIN\n"
                                        "  testt := 0\n"
                                        "END."};

        Result<Function> function = buildAST(management);
        ASSERT_TRUE(function.isFailure());

        CaptureCOut capture;
        function.error().printCompilerError();

        ASSERT_EQ(
            capture.str(),
            "3:3: error: Using undeclared identifier!\n"
            "  testt := 0\n"
            "  ^~~~~\n"
        );
    }

    {
        SourceCodeManagement management{"VAR test;\n"
                                        "BEGIN\n"
                                        "  test := +testt\n"
                                        "END."};

        Result<Function> function = buildAST(management);
        ASSERT_TRUE(function.isFailure());

        CaptureCOut capture;
        function.error().printCompilerError();

        ASSERT_EQ(
            capture.str(),
            "3:12: error: Using undeclared identifier!\n"
            "  test := +testt\n"
            "           ^~~~~\n"
        );
    }

    {
        SourceCodeManagement management{"VAR test;\n"
                                        "BEGIN\n"
                                        "  test := 2;\n"
                                        "  RETURN tes\n"
                                        "END."};

        Result<Function> function = buildAST(management);
        ASSERT_TRUE(function.isFailure());

        CaptureCOut capture;
        function.error().printCompilerError();

        ASSERT_EQ(
            capture.str(),
            "4:10: error: Using undeclared identifier!\n"
            "  RETURN tes\n"
            "         ^~~\n"
        );
    }
}

TEST(AST, testAssigningConstant) {
    SourceCodeManagement management{"CONST test = 2;\n"
                                    "BEGIN\n"
                                    "  test := 3;\n"
                                    "  RETURN test\n"
                                    "END."};

    Result<Function> function = buildAST(management);
    ASSERT_TRUE(function.isFailure());

    CaptureCOut capture;
    function.error().printCompilerError();

    ASSERT_EQ(
        capture.str(),
        "3:3: error: Can't assign to constant!\n"
        "  test := 3;\n"
        "  ^~~~\n"
    );
}

TEST(AST, testIdentifierRedeclaration) {
    {
        SourceCodeManagement management{"CONST test = 2, test = 3;\n"
                                        "BEGIN\n"
                                        "  RETURN 0\n"
                                        "END."};

        Result<Function> function = buildAST(management);
        ASSERT_TRUE(function.isFailure());

        CaptureCOut capture;
        function.error().printCompilerError();

        ASSERT_EQ(
            capture.str(),
            "1:17: error: Redefinition of identifier!\n"
            "CONST test = 2, test = 3;\n"
            "                ^~~~\n"
            "1:7: note: Original declaration here\n"
            "CONST test = 2, test = 3;\n"
            "      ^~~~\n"
        );
    }

    {
        SourceCodeManagement management{"VAR test;\n"
                                        "CONST test = 2;\n"
                                        "BEGIN\n"
                                        "  RETURN 0\n"
                                        "END."};

        Result<Function> function = buildAST(management);
        ASSERT_TRUE(function.isFailure());

        CaptureCOut capture;
        function.error().printCompilerError();

        ASSERT_EQ(
            capture.str(),
            "2:7: error: Redefinition of identifier!\n"
            "CONST test = 2;\n"
            "      ^~~~\n"
            "1:5: note: Original declaration here\n"
            "VAR test;\n"
            "    ^~~~\n"
        );
    }

    {
        SourceCodeManagement management{"PARAM test;\n"
                                        "VAR test;\n"
                                        "BEGIN\n"
                                        "  RETURN 0\n"
                                        "END."};

        Result<Function> function = buildAST(management);
        ASSERT_TRUE(function.isFailure());

        CaptureCOut capture;
        function.error().printCompilerError();

        ASSERT_EQ(
            capture.str(),
            "2:5: error: Redefinition of identifier!\n"
            "VAR test;\n"
            "    ^~~~\n"
            "1:7: note: Original declaration here\n"
            "PARAM test;\n"
            "      ^~~~\n"
        );
    }
}

TEST(AST, testUninitializedVariableUsage) {
    {
        SourceCodeManagement management{"VAR test;\n"
                                        "BEGIN\n"
                                        "  RETURN test\n"
                                        "END."};

        Result<Function> function = buildAST(management);
        ASSERT_TRUE(function.isFailure());

        CaptureCOut capture;
        function.error().printCompilerError();

        ASSERT_EQ(
            capture.str(),
            "3:10: error: Tried to use uninitialized variable!\n"
            "  RETURN test\n"
            "         ^~~~\n"
        );
    }

    {
        SourceCodeManagement management{"VAR test;\n"
                                        "BEGIN\n"
                                        "  test := test + 2;"
                                        "  RETURN 0\n"
                                        "END."};

        Result<Function> function = buildAST(management);
        ASSERT_TRUE(function.isFailure());

        CaptureCOut capture;
        function.error().printCompilerError();

        ASSERT_EQ(
            capture.str(),
            "3:11: error: Tried to use uninitialized variable!\n"
            "  test := test + 2;  RETURN 0\n"
            "          ^~~~\n"
        );
    }
}
//---------------------------------------------------------------------------

