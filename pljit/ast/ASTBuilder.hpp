//
// Created by Andreas Bauer on 31.07.22.
//

#ifndef PLJIT_ASTBUILDER_HPP
#define PLJIT_ASTBUILDER_HPP
#include "../parse/ParseTree.hpp"
#include "../util/Result.hpp"
#include "pljit/SymbolTable.hpp"
#include <memory>

// TODO search for unused stuff!
// TODO revisit all header includes!

//---------------------------------------------------------------------------
namespace pljit::ast {
//---------------------------------------------------------------------------
class Declaration;
class ParamDeclaration;
class VarDeclaration;
class ConstDeclaration;
class Expression;
class Statement;
class Function;
//---------------------------------------------------------------------------
class ASTBuilder {
    SymbolTable symbolTable;

    public:
    ASTBuilder();

    Result<Function> analyzeFunction(const parse::FunctionDefinition& node);

    Result<ParamDeclaration> analyzeParamDeclaration(const parse::ParameterDeclarations& node);
    Result<VarDeclaration> analyzeVarDeclaration(const parse::VariableDeclarations& node);
    Result<ConstDeclaration> analyzeConstDeclaration(const parse::ConstantDeclarations& node);

    Result<std::unique_ptr<Statement>> analyzeStatement(const parse::Statement& node);

    Result<std::unique_ptr<Expression>> analyzeExpression(const parse::AdditiveExpression& node);
    Result<std::unique_ptr<Expression>> analyzeExpression(const parse::MultiplicativeExpression& node);
    Result<std::unique_ptr<Expression>> analyzeExpression(const parse::UnaryExpression& node);
    Result<std::unique_ptr<Expression>> analyzeExpression(const parse::PrimaryExpression& node);
};
//---------------------------------------------------------------------------
} // namespace pljit::ast
//---------------------------------------------------------------------------

#endif //PLJIT_ASTBUILDER_HPP
