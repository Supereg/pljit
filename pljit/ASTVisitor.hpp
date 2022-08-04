//
// Created by Andreas Bauer on 03.08.22.
//

#ifndef PLJIT_ASTVISITOR_HPP
#define PLJIT_ASTVISITOR_HPP

//---------------------------------------------------------------------------
namespace pljit::ast {
//---------------------------------------------------------------------------
class Literal;
class Variable;
class UnaryPlus;
class UnaryMinus;
class Add;
class Subtract;
class Multiply;
class Divide;
class AssignmentStatement;
class ReturnStatement;
class ParamDeclaration;
class VarDeclaration;
class ConstDeclaration;
class Function;
//---------------------------------------------------------------------------
class ASTVisitor {
    public:
    ASTVisitor() = default;
    virtual ~ASTVisitor() = default;

    virtual void visit(const Function& node) = 0;
    virtual void visit(const ConstDeclaration& node) = 0;
    virtual void visit(const VarDeclaration& node) = 0;
    virtual void visit(const ParamDeclaration& node) = 0;
    virtual void visit(const ReturnStatement& node) = 0;
    virtual void visit(const AssignmentStatement& node) = 0;
    virtual void visit(const Divide& node) = 0;
    virtual void visit(const Multiply& node) = 0;
    virtual void visit(const Subtract& node) = 0;
    virtual void visit(const Add& node) = 0;
    virtual void visit(const UnaryMinus& node) = 0;
    virtual void visit(const UnaryPlus& node) = 0;
    virtual void visit(const Variable& node) = 0;
    virtual void visit(const Literal& node) = 0;
};
//---------------------------------------------------------------------------
} // namespace pljit::ast
//---------------------------------------------------------------------------

#endif //PLJIT_ASTVISITOR_HPP
