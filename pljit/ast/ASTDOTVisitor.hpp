//
// Created by Andreas Bauer on 03.08.22.
//

#ifndef PLJIT_ASTDOTVISITOR_HPP
#define PLJIT_ASTDOTVISITOR_HPP

#include "./ASTVisitor.hpp"
#include "../util/GenericDOTVisitor.hpp"

//---------------------------------------------------------------------------
namespace pljit::ast {
//---------------------------------------------------------------------------
class DOTVisitor: public ASTVisitor, protected GenericDOTVisitor {
    public:
    DOTVisitor();

    template <typename T>
    void print(const T& node);

    void visit(const Function& node) override;
    void visit(const ConstDeclaration& node) override;
    void visit(const VarDeclaration& node) override;
    void visit(const ParamDeclaration& node) override;
    void visit(const ReturnStatement& node) override;
    void visit(const AssignmentStatement& node) override;
    void visit(const Divide& node) override;
    void visit(const Multiply& node) override;
    void visit(const Subtract& node) override;
    void visit(const Add& node) override;
    void visit(const UnaryMinus& node) override;
    void visit(const UnaryPlus& node) override;
    void visit(const Variable& node) override;
    void visit(const Literal& node) override;
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
} // namespace pljit::ast
//---------------------------------------------------------------------------

#endif //PLJIT_ASTDOTVISITOR_HPP
