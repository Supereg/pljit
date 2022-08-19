//
// Created by Andreas Bauer on 30.07.22.
//

#ifndef PLJIT_PARSETREEDOTVISITOR_HPP
#define PLJIT_PARSETREEDOTVISITOR_HPP
#include "./ParseTreeVisitor.hpp"
#include "../util/GenericDOTVisitor.hpp"

//---------------------------------------------------------------------------
namespace pljit::parse {
//---------------------------------------------------------------------------
class DOTVisitor: public ParseTreeVisitor, protected GenericDOTVisitor {
    public:
    DOTVisitor();

    template <typename T>
    void print(const T& node);

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
template <typename T>
void DOTVisitor::print(const T& node) {
    reset();
    printGraphHeader();

    node.accept(*this);

    printGraphFooter();
}
//---------------------------------------------------------------------------
} // namespace pljit::parse
//---------------------------------------------------------------------------

#endif //PLJIT_PARSETREEDOTVISITOR_HPP
