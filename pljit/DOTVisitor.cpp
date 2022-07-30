//
// Created by Andreas Bauer on 30.07.22.
//

#include "DOTVisitor.hpp"
#include "Parser.hpp" // TODO parse tree implementation header!
#include <iostream>

//---------------------------------------------------------------------------
namespace pljit::ParseTree {
//---------------------------------------------------------------------------
using std::cout;
using std::endl;
//---------------------------------------------------------------------------
DOTVisitor::DOTVisitor() : ParseTreeVisitor(), node_num(0) {}

void DOTVisitor::printNode(std::string_view name) const {
    cout << "  n_" << node_num << " [label=\""<< name << "\",shape=box];" << endl;
}

void DOTVisitor::printTerminalNode(std::string_view content) const {
    cout << "  n_" << node_num << R"( [label="\")" << content << R"(\""];)" << endl;
}

void DOTVisitor::printTerminalNode(long long content) const {
    cout << "  n_" << node_num << "[label=\"" << content << "\"];" << endl;
}

void DOTVisitor::printEdge(unsigned root_node) const {
    cout << "  n_" << root_node << " -- n_" << (node_num + 1) << ";" << endl;
}

void DOTVisitor::visit(const FunctionDefinition& node) {
    unsigned root = ++node_num;

    cout << "graph {" << endl;
    printNode("function-definition");

    if (node.getParameterDeclarations()) {
        printEdge(root);
        node.getParameterDeclarations()->accept(*this);
    }

    if (node.getVariableDeclarations()) {
        printEdge(root);
        node.getVariableDeclarations()->accept(*this);
    }

    if (node.getConstantDeclarations()) {
        printEdge(root);
        node.getConstantDeclarations()->accept(*this);
    }

    printEdge(root);
    node.getCompoundStatement().accept(*this);

    cout << "}";
}

void DOTVisitor::visit(const ParameterDeclarations& node) {
    unsigned root = ++node_num;

    printNode("parameter-declarations");

    printEdge(root);
    node.getParamKeyword().accept(*this);
    printEdge(root);
    node.getDeclaratorList().accept(*this);
    printEdge(root);
    node.getSemicolon().accept(*this);
}

void DOTVisitor::visit(const VariableDeclarations& node) {
    unsigned root = ++node_num;

    printNode("variable-declarations");

    printEdge(root);
    node.getVarKeyword().accept(*this);
    printEdge(root);
    node.getDeclaratorList().accept(*this);
    printEdge(root);
    node.getSemicolon().accept(*this);
}

void DOTVisitor::visit(const ConstantDeclarations& node) {
    unsigned root = ++node_num;

    printNode("constant-declarations");

    printEdge(root);
    node.getConstKeyword().accept(*this);
    printEdge(root);
    node.getInitDeclaratorList().accept(*this);
    printEdge(root);
    node.getSemicolon().accept(*this);
}

void DOTVisitor::visit(const DeclaratorList& node) {
    unsigned root = ++node_num;

    printNode("declarator-list");

    printEdge(root);
    node.getIdentifier().accept(*this);

    for (auto& tuple: node.getAdditionalIdentifiers()) {
        printEdge(root);
        std::get<GenericTerminal>(tuple).accept(*this);
        printEdge(root);
        std::get<Identifier>(tuple).accept(*this);
    }
}

void DOTVisitor::visit(const InitDeclaratorList& node) {
    unsigned root = ++node_num;

    printNode("init-declarator-list");

    printEdge(root);
    node.getInitDeclarator().accept(*this);

    for (auto tuple: node.getAdditionalInitDeclarators()) {
        printEdge(root);
        std::get<GenericTerminal>(tuple).accept(*this);
        printEdge(root);
        std::get<InitDeclarator>(tuple).accept(*this);
    }
}

void DOTVisitor::visit(const InitDeclarator& node) {
    unsigned root = ++node_num;

    printNode("init-declarator");

    printEdge(root);
    node.getIdentifier().accept(*this);
    printEdge(root);
    node.getInitOperator().accept(*this);
    printEdge(root);
    node.getLiteral().accept(*this);
}

void DOTVisitor::visit(const CompoundStatement& node) {
    unsigned root = ++node_num;

    printNode("compound-statement");

    printEdge(root);
    node.getBeginKeyword().accept(*this);
    printEdge(root);
    node.getStatementList().accept(*this);
    printEdge(root);
    node.getEndKeyword().accept(*this);
}

void DOTVisitor::visit(const StatementList& node) {
    unsigned root = ++node_num;

    printNode("statement-list");

    printEdge(root);
    node.getStatement().accept(*this);

    for (auto& tuple: node.getAdditionalStatements()) {
        printEdge(root);
        std::get<GenericTerminal>(tuple).accept(*this);
        printEdge(root);
        std::get<Statement>(tuple).accept(*this);
    }
}

void DOTVisitor::visit(const Statement& node) {
    unsigned root = ++node_num;

    printNode("statement");

    switch (node.getType()) {
        case Statement::Type::ASSIGNMENT:
            printEdge(root);
            node.asAssignmentExpression().accept(*this);
            break;
        case Statement::Type::RETURN: {
            auto [returnKeyword, additiveExpression] = node.asReturnExpression();
            printEdge(root);
            returnKeyword.accept(*this);
            printEdge(root);
            additiveExpression.accept(*this);
            break;
        }
        case Statement::Type::NONE:
            break;
    }
}

void DOTVisitor::visit(const AssignmentExpression& node) {
    unsigned root = ++node_num;

    printNode("assignment-expression");

    printEdge(root);
    node.getIdentifier().accept(*this);
    printEdge(root);
    node.getAssignmentOperator().accept(*this);
    printEdge(root);
    node.getAdditiveExpression().accept(*this);
}

void DOTVisitor::visit(const AdditiveExpression& node) {
    unsigned root = ++node_num;

    printNode("additive-expression");

    printEdge(root);
    node.getExpression().accept(*this);

    if (node.getOperand()) {
        auto [operatorTerminal, additiveExpression] = *node.getOperand();

        printEdge(root);
        operatorTerminal.accept(*this);
        printEdge(root);
        additiveExpression.accept(*this);
    }
}

void DOTVisitor::visit(const MultiplicativeExpression& node) {
    unsigned root = ++node_num;

    printNode("multiplicative-expression");

    printEdge(root);
    node.getExpression().accept(*this);

    if (node.getOperand()) {
        auto [operatorTerminal, additiveExpression] = *node.getOperand();

        printEdge(root);
        operatorTerminal.accept(*this);
        printEdge(root);
        additiveExpression.accept(*this);
    }
}

void DOTVisitor::visit(const UnaryExpression& node) {
    unsigned root = ++node_num;

    printNode("unary-expression");

    if (node.getUnaryOperator()) {
        printEdge(root);
        node.getUnaryOperator()->accept(*this);
    }

    printEdge(root);
    node.getPrimaryExpression().accept(*this);
}

void DOTVisitor::visit(const PrimaryExpression& node) {
    unsigned root = ++node_num;

    printNode("primary-expression");

    switch (node.getType()) {
        case PrimaryExpression::Type::IDENTIFIER:
            printEdge(root);
            node.asIdentifier().accept(*this);
            break;
        case PrimaryExpression::Type::LITERAL:
            printEdge(root);
            node.asLiteral().accept(*this);
            break;
        case PrimaryExpression::Type::ADDITIVE_EXPRESSION: {
            auto [openParenthesis, additiveExpression, closeParenthesis] = node.asBracketedExpression();
            printEdge(root);
            openParenthesis.accept(*this);
            printEdge(root);
            additiveExpression.accept(*this);
            printEdge(root);
            closeParenthesis.accept(*this);
            break;
        }
        case PrimaryExpression::Type::NONE:
            break;
    }
}

void DOTVisitor::visit(const Literal& node) {
    ++node_num;

    printTerminalNode(node.value());
}

void DOTVisitor::visit(const Identifier& node) {
    ++node_num;

    printTerminalNode(node.value());
}

void DOTVisitor::visit(const GenericTerminal& node) {
    ++node_num;

    printTerminalNode(node.value());
}
//---------------------------------------------------------------------------
} // namespace pljit::ParseTree
//---------------------------------------------------------------------------