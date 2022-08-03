//
// Created by Andreas Bauer on 31.07.22.
//

#ifndef PLJIT_ASTBUILDER_HPP
#define PLJIT_ASTBUILDER_HPP
#include "SymbolTable.hpp"
#include "ParseTreeVisitor.hpp" // TODO remove!
#include "Result.hpp"
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

    Result<Function> analyzeFunction(const ParseTree::FunctionDefinition& node);

    Result<ParamDeclaration> analyzeParamDeclaration(const ParseTree::ParameterDeclarations& node);
    Result<VarDeclaration> analyzeVarDeclaration(const ParseTree::VariableDeclarations& node);
    Result<ConstDeclaration> analyzeConstDeclaration(const ParseTree::ConstantDeclarations& node);

    Result<std::unique_ptr<Statement>> analyzeStatement(const ParseTree::Statement& node); // TODO return ptr!

    Result<std::unique_ptr<Expression>> analyzeExpression(const ParseTree::AdditiveExpression& node);
    Result<std::unique_ptr<Expression>> analyzeExpression(const ParseTree::MultiplicativeExpression& node);
    Result<std::unique_ptr<Expression>> analyzeExpression(const ParseTree::UnaryExpression& node);
    Result<std::unique_ptr<Expression>> analyzeExpression(const ParseTree::PrimaryExpression& node);
};
//---------------------------------------------------------------------------
} // namespace pljit::ast
//---------------------------------------------------------------------------

#endif //PLJIT_ASTBUILDER_HPP
