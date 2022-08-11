//
// Created by Andreas Bauer on 31.07.22.
//

#ifndef PLJIT_ASTBUILDER_HPP
#define PLJIT_ASTBUILDER_HPP
#include "./SymbolTable.hpp"
#include "../parse/ParseTree.hpp"
#include "../util/Result.hpp"
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

    Result<Function> analyzeFunction(const parse::ParseTree::FunctionDefinition& node);

    Result<ParamDeclaration> analyzeParamDeclaration(const parse::ParseTree::ParameterDeclarations& node);
    Result<VarDeclaration> analyzeVarDeclaration(const parse::ParseTree::VariableDeclarations& node);
    Result<ConstDeclaration> analyzeConstDeclaration(const parse::ParseTree::ConstantDeclarations& node);

    Result<std::unique_ptr<Statement>> analyzeStatement(const parse::ParseTree::Statement& node); // TODO return ptr!

    Result<std::unique_ptr<Expression>> analyzeExpression(const parse::ParseTree::AdditiveExpression& node);
    Result<std::unique_ptr<Expression>> analyzeExpression(const parse::ParseTree::MultiplicativeExpression& node);
    Result<std::unique_ptr<Expression>> analyzeExpression(const parse::ParseTree::UnaryExpression& node);
    Result<std::unique_ptr<Expression>> analyzeExpression(const parse::ParseTree::PrimaryExpression& node);
};
//---------------------------------------------------------------------------
} // namespace pljit::ast
//---------------------------------------------------------------------------

#endif //PLJIT_ASTBUILDER_HPP
