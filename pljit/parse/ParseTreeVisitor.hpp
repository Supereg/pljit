//
// Created by Andreas Bauer on 30.07.22.
//

#ifndef PLJIT_PARSETREEVISITOR_HPP
#define PLJIT_PARSETREEVISITOR_HPP
//---------------------------------------------------------------------------
namespace pljit::parse::ParseTree {
//---------------------------------------------------------------------------
class GenericTerminal;
class Identifier;
class Literal;
class PrimaryExpression;
class UnaryExpression;
class MultiplicativeExpression;
class AdditiveExpression;
class AssignmentExpression;
class Statement;
class StatementList;
class CompoundStatement;
class InitDeclarator;
class InitDeclaratorList;
class DeclaratorList;
class ConstantDeclarations;
class VariableDeclarations;
class ParameterDeclarations;
class FunctionDefinition;
//---------------------------------------------------------------------------
class ParseTreeVisitor {
    public:
    ParseTreeVisitor() = default;
    virtual ~ParseTreeVisitor() = default;

    virtual void visit(const GenericTerminal& node) = 0;
    virtual void visit(const Identifier& node) = 0;
    virtual void visit(const Literal& node) = 0;
    virtual void visit(const PrimaryExpression& node) = 0;
    virtual void visit(const UnaryExpression& node) = 0;
    virtual void visit(const MultiplicativeExpression& node) = 0;
    virtual void visit(const AdditiveExpression& node) = 0;
    virtual void visit(const AssignmentExpression& node) = 0;
    virtual void visit(const Statement& node) = 0;
    virtual void visit(const StatementList& node) = 0;
    virtual void visit(const CompoundStatement& node) = 0;
    virtual void visit(const InitDeclarator& node) = 0;
    virtual void visit(const InitDeclaratorList& node) = 0;
    virtual void visit(const DeclaratorList& node) = 0;
    virtual void visit(const ConstantDeclarations& node) = 0;
    virtual void visit(const VariableDeclarations& node) = 0;
    virtual void visit(const ParameterDeclarations& node) = 0;
    virtual void visit(const FunctionDefinition& node) = 0;
};
//---------------------------------------------------------------------------
} // namespace pljit::parse::ParseTree
//---------------------------------------------------------------------------
#endif //PLJIT_PARSETREEVISITOR_HPP
