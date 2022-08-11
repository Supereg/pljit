//
// Created by Andreas Bauer on 03.08.22.
//

#include "./ASTDOTVisitor.hpp"
#include "./AST.hpp"

//---------------------------------------------------------------------------
namespace pljit::ast {
//---------------------------------------------------------------------------
DOTVisitor::DOTVisitor() = default;

void DOTVisitor::visit(const Function& node) {
    unsigned root = ++node_num;

    printGraphHeader();
    printNode("Function");

    if (node.getParamDeclaration()) {
        printEdge(root);
        node.getParamDeclaration()->accept(*this);
    }

    if (node.getVarDeclaration()) {
        printEdge(root);
        node.getVarDeclaration()->accept(*this);
    }

    if (node.getConstDeclaration()) {
        printEdge(root);
        node.getConstDeclaration()->accept(*this);
    }

    for (auto& statement: node.getStatements()) {
        printEdge(root);
        statement->accept(*this);
    }

    printGraphFooter();
}

void DOTVisitor::visit(const ConstDeclaration& node) {
    unsigned root = ++node_num;

    printNode("ConstDeclaration");

    for (auto& [variable, literal]: node.getConstDeclarations()) {
        printEdge(root);
        printTerminalNode(variable.getName());
        printEdge(root);
        printTerminalNode(literal.value());
    }
}

// TODO method ordering!
void DOTVisitor::visit(const VarDeclaration& node) {
    unsigned root = ++node_num;

    printNode("VarDeclaration");

    for (auto& variable: node.getDeclaredIdentifiers()) {
        printEdge(root);
        variable.accept(*this);
    }
}

void DOTVisitor::visit(const ParamDeclaration& node) {
    unsigned root = ++node_num;

    printNode("ParamDeclaration");

    for (auto& variable: node.getDeclaredIdentifiers()) {
        printEdge(root);
        variable.accept(*this);
    }
}

void DOTVisitor::visit(const ReturnStatement& node) {
    unsigned root = ++node_num;

    printNode("ReturnStatement");

    printEdge(root);
    node.getExpression().accept(*this);
}

void DOTVisitor::visit(const AssignmentStatement& node) {
    unsigned root = ++node_num;

    printNode("AssignmentStatement");

    printEdge(root);
    node.getVariable().accept(*this);
    printEdge(root);
    node.getExpression().accept(*this);
}

void DOTVisitor::visit(const Divide& node) {
    unsigned root = ++node_num;

    printNode("Divide");

    printEdge(root);
    node.getLeft().accept(*this);
    printEdge(root);
    node.getRight().accept(*this);
}

void DOTVisitor::visit(const Multiply& node) {
    unsigned root = ++node_num;

    printNode("Multiply");

    printEdge(root);
    node.getLeft().accept(*this);
    printEdge(root);
    node.getRight().accept(*this);
}

void DOTVisitor::visit(const Subtract& node) {
    unsigned root = ++node_num;

    printNode("Subtract");

    printEdge(root);
    node.getLeft().accept(*this);
    printEdge(root);
    node.getRight().accept(*this);
}

void DOTVisitor::visit(const Add& node) {
    unsigned root = ++node_num;

    printNode("Add");

    printEdge(root);
    node.getLeft().accept(*this);
    printEdge(root);
    node.getRight().accept(*this);
}

void DOTVisitor::visit(const UnaryMinus& node) {
    unsigned root = ++node_num;

    printNode("UnaryMinus");

    printEdge(root);
    node.getChild().accept(*this);
}

void DOTVisitor::visit(const UnaryPlus& node) {
    unsigned root = ++node_num;

    printNode("UnaryPlus");

    printEdge(root);
    node.getChild().accept(*this);
}

void DOTVisitor::visit(const Variable& node) {
    ++node_num;

    printTerminalNode(node.getName());
}

void DOTVisitor::visit(const Literal& node) {
    ++node_num;

    printTerminalNode(node.value());
}
//---------------------------------------------------------------------------
} // namespace pljit::ast
//---------------------------------------------------------------------------
