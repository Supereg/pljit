//
// Created by Andreas Bauer on 14.08.22.
//

#include "pljit/ast/AST.hpp"
#include "pljit/ast/ASTBuilder.hpp"
#include "pljit/ast/ASTDOTVisitor.hpp"
#include "pljit/lex/Lexer.hpp"
#include "pljit/parse/Parser.hpp"
#include "pljit/optimizations/DeadCodeElimination.hpp"
#include "pljit/optimizations/ConstantPropagation.hpp"
#include "utils/CaptureCOut.hpp"
#include <gtest/gtest.h>

//---------------------------------------------------------------------------
using namespace pljit;
using namespace pljit::code;
using namespace pljit::parse;
using namespace pljit::lex;
using namespace pljit::ast;
using namespace pljit::ast::optimize;
//---------------------------------------------------------------------------
// TODO duplicated code!
static Result<Function> buildAST(const SourceCodeManagement& management) {
    Lexer lexer{ management };
    Parser parser{ lexer };

    Result<FunctionDefinition> program = parser.parse_program();
    if (program.failure()) {
        program.error().printCompilerError();
    }
    assert(program.success() && "Unexpected parsing error!");

    ASTBuilder builder;
    return builder.analyzeFunction(*program);
}
//---------------------------------------------------------------------------
TEST(ASTOptimization, testDeadCodeElminiation) {
    SourceCodeManagement management{"PARAM a;\n"
                                    "VAR b;\n"
                                    "CONST c = 0;\n"
                                    "BEGIN\n"
                                    "  RETURN 0;\n"
                                    "  b := a + c;\n"
                                    "  RETURN b\n"
                                    "END."};

    Result<Function> result = buildAST(management);
    ASSERT_TRUE(result.success());

    Function function = result.release();

    DeadCodeElimination optimization;
    optimization.optimize(function);

    CaptureCOut capture;
    DOTVisitor visitor;
    function.accept(visitor);

    ASSERT_EQ(
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
        "  n_6 [label=\"ConstDeclaration\",shape=box];\n"
        "  n_6 -- n_7;\n"
        "  n_7 [label=\"\\\"c\\\"\"];\n"
        "  n_6 -- n_8;\n"
        "  n_8[label=\"0\"];\n"
        "  n_1 -- n_9;\n"
        "  n_9 [label=\"ReturnStatement\",shape=box];\n"
        "  n_9 -- n_10;\n"
        "  n_10[label=\"0\"];\n"
        "}\n"
    );
}

// TODO test assignemtn making variable constant again!
TEST(ASTOptimization, testConstantPropagation) {
    SourceCodeManagement management{"PARAM x;\n"
                                    "VAR a,b,f;\n"
                                    "CONST c = 2, d = 3, e = 4;\n"
                                    "BEGIN\n"
                                    "  f := 1+1;\n" // f := 2;
                                    "  a := (3*(c * 2)) + (e / +2) - (d + -e);\n" // a := 15;
                                    "  f := x;\n"
                                    "  b := ((d + x) + (d/0));\n" // ((3+x) + (d/0));
                                    "  RETURN a;\n" // RETURN 15
                                    "  RETURN f\n" // RETURN f
                                    "END."};

    Result<Function> result = buildAST(management);
    if (result.failure()) {
        result.error().printCompilerError();
    }
    ASSERT_TRUE(result.success());

    Function function = result.release();

    ConstantPropagation optimization;
    optimization.optimize(function);

    CaptureCOut capture;
    DOTVisitor visitor;
    function.accept(visitor);

    ASSERT_EQ(
        capture.str(),
        "graph {\n"
        "  n_1 [label=\"Function\",shape=box];\n"
        "  n_1 -- n_2;\n"
        "  n_2 [label=\"ParamDeclaration\",shape=box];\n"
        "  n_2 -- n_3;\n"
        "  n_3 [label=\"\\\"x\\\"\"];\n"
        "  n_1 -- n_4;\n"
        "  n_4 [label=\"VarDeclaration\",shape=box];\n"
        "  n_4 -- n_5;\n"
        "  n_5 [label=\"\\\"a\\\"\"];\n"
        "  n_4 -- n_6;\n"
        "  n_6 [label=\"\\\"b\\\"\"];\n"
        "  n_4 -- n_7;\n"
        "  n_7 [label=\"\\\"f\\\"\"];\n"
        "  n_1 -- n_8;\n"
        "  n_8 [label=\"ConstDeclaration\",shape=box];\n"
        "  n_8 -- n_9;\n"
        "  n_9 [label=\"\\\"c\\\"\"];\n"
        "  n_8 -- n_10;\n"
        "  n_10[label=\"2\"];\n"
        "  n_8 -- n_11;\n"
        "  n_11 [label=\"\\\"d\\\"\"];\n"
        "  n_8 -- n_12;\n"
        "  n_12[label=\"3\"];\n"
        "  n_8 -- n_13;\n"
        "  n_13 [label=\"\\\"e\\\"\"];\n"
        "  n_8 -- n_14;\n"
        "  n_14[label=\"4\"];\n"
        "  n_1 -- n_15;\n"
        "  n_15 [label=\"AssignmentStatement\",shape=box];\n"
        "  n_15 -- n_16;\n"
        "  n_16 [label=\"\\\"f\\\"\"];\n"
        "  n_15 -- n_17;\n"
        "  n_17[label=\"2\"];\n"
        "  n_1 -- n_18;\n"
        "  n_18 [label=\"AssignmentStatement\",shape=box];\n"
        "  n_18 -- n_19;\n"
        "  n_19 [label=\"\\\"a\\\"\"];\n"
        "  n_18 -- n_20;\n"
        "  n_20[label=\"15\"];\n"
        "  n_1 -- n_21;\n"
        "  n_21 [label=\"AssignmentStatement\",shape=box];\n"
        "  n_21 -- n_22;\n"
        "  n_22 [label=\"\\\"f\\\"\"];\n"
        "  n_21 -- n_23;\n"
        "  n_23 [label=\"\\\"x\\\"\"];\n"
        "  n_1 -- n_24;\n"
        "  n_24 [label=\"AssignmentStatement\",shape=box];\n"
        "  n_24 -- n_25;\n"
        "  n_25 [label=\"\\\"b\\\"\"];\n"
        "  n_24 -- n_26;\n"
        "  n_26 [label=\"Add\",shape=box];\n"
        "  n_26 -- n_27;\n"
        "  n_27 [label=\"Add\",shape=box];\n"
        "  n_27 -- n_28;\n"
        "  n_28[label=\"3\"];\n"
        "  n_27 -- n_29;\n"
        "  n_29 [label=\"\\\"x\\\"\"];\n"
        "  n_26 -- n_30;\n"
        "  n_30 [label=\"Divide\",shape=box];\n"
        "  n_30 -- n_31;\n"
        "  n_31[label=\"3\"];\n"
        "  n_30 -- n_32;\n"
        "  n_32[label=\"0\"];\n"
        "  n_1 -- n_33;\n"
        "  n_33 [label=\"ReturnStatement\",shape=box];\n"
        "  n_33 -- n_34;\n"
        "  n_34[label=\"15\"];\n"
        "  n_1 -- n_35;\n"
        "  n_35 [label=\"ReturnStatement\",shape=box];\n"
        "  n_35 -- n_36;\n"
        "  n_36 [label=\"\\\"f\\\"\"];\n"
        "}\n"
    );
}
//---------------------------------------------------------------------------
