//
// Created by Andreas Bauer on 30.07.22.
//

#ifndef PLJIT_DOTVISITOR_HPP
#define PLJIT_DOTVISITOR_HPP
#include "ParseTreeVisitor.hpp"
#include <string>

//---------------------------------------------------------------------------
namespace pljit::ParseTree {
//---------------------------------------------------------------------------
class DOTVisitor: public ParseTreeVisitor {
    unsigned node_num;

    public:
    DOTVisitor();

    private:
    void printNode(std::string_view name) const;
    void printTerminalNode(std::string_view content) const;
    void printTerminalNode(long long content) const;
    void printEdge(unsigned root_node) const;

    public:
    void visit(const GenericTerminal& node) override;
    void visit(const Identifier& node) override;
    void visit(const Literal& node) override;
    void visit(const PrimaryExpression& node) override;
    void visit(const UnaryExpression& node) override;
    void visit(const MultiplicativeExpression& node) override;
    void visit(const AdditiveExpression& node) override;
    void visit(const AssignmentExpression& node) override;
    void visit(const Statement& node) override;
    void visit(const StatementList& node) override;
    void visit(const CompoundStatement& node) override;
    void visit(const InitDeclarator& node) override;
    void visit(const InitDeclaratorList& node) override;
    void visit(const DeclaratorList& node) override;
    void visit(const ConstantDeclarations& node) override;
    void visit(const VariableDeclarations& node) override;
    void visit(const ParameterDeclarations& node) override;
    void visit(const FunctionDefinition& node) override;
};
//---------------------------------------------------------------------------
} // namespace pljit::ParseTree
//---------------------------------------------------------------------------

#endif //PLJIT_DOTVISITOR_HPP