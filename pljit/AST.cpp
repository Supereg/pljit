//
// Created by Andreas Bauer on 30.07.22.
//

#include "AST.hpp"

#include <utility>

//---------------------------------------------------------------------------
namespace pljit::ast {
//---------------------------------------------------------------------------
Literal::Literal(long long int literal_value) : literal_value(literal_value) {}

Node::Type Literal::getType() const {
    return Node::Type::LITERAL;
}
//---------------------------------------------------------------------------
Variable::Variable() : symbolId(0) {}
Variable::Variable(symbol_id symbolId) : symbolId(symbolId) {}

Node::Type Variable::getType() const {
    return Node::Type::VARIABLE;
}
//---------------------------------------------------------------------------
UnaryExpression::UnaryExpression(std::unique_ptr<Expression> child) : child(std::move(child)) {}

const Expression& UnaryExpression::getChild() const {
    return *child;
}
//---------------------------------------------------------------------------
BinaryExpression::BinaryExpression(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild)
    : leftChild(std::move(leftChild)), rightChild(std::move(rightChild)) {}

const Expression& BinaryExpression::getLeft() const {
    return *leftChild;
}

const Expression& BinaryExpression::getRight() const {
    return *rightChild;
}
//---------------------------------------------------------------------------
UnaryPlus::UnaryPlus(std::unique_ptr<Expression> child) : UnaryExpression(std::move(child)) {}

Node::Type UnaryPlus::getType() const {
    return Node::Type::UNARY_PLUS;
}
//---------------------------------------------------------------------------
UnaryMinus::UnaryMinus(std::unique_ptr<Expression> child) : UnaryExpression(std::move(child)) {}

Node::Type UnaryMinus::getType() const {
    return Node::Type::UNARY_MINUS;
}
//---------------------------------------------------------------------------
Add::Add(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild)
    : BinaryExpression(std::move(leftChild), std::move(rightChild)) {}

Node::Type Add::getType() const {
    return Node::Type::ADD;
}
//---------------------------------------------------------------------------
Subtract::Subtract(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild)
    : BinaryExpression(std::move(leftChild), std::move(rightChild)) {}

Node::Type Subtract::getType() const {
    return Node::Type::SUBTRACT;
}
//---------------------------------------------------------------------------
Multiply::Multiply(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild)
    : BinaryExpression(std::move(leftChild), std::move(rightChild)) {}

Node::Type Multiply::getType() const {
    return Node::Type::MULTIPLY;
}
//---------------------------------------------------------------------------
Divide::Divide(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild)
    : BinaryExpression(std::move(leftChild), std::move(rightChild)) {}

Node::Type Divide::getType() const {
    return Node::Type::DIVIDE;
}
//---------------------------------------------------------------------------
Statement::Statement(std::unique_ptr<Expression> expression) : expression(std::move(expression)) {}

const Expression& Statement::getExpression() const {
    return *expression;
}
//---------------------------------------------------------------------------
AssignmentStatement::AssignmentStatement(std::unique_ptr<Expression> expression, Variable variable)
    : Statement(std::move(expression)), variable(std::move(variable)) {}

Node::Type AssignmentStatement::getType() const {
    return Node::Type::ASSIGNMENT_STATEMENT;
}
//---------------------------------------------------------------------------
ReturnStatement::ReturnStatement(std::unique_ptr<Expression> expression) : Statement(std::move(expression)) {}

Node::Type ReturnStatement::getType() const {
    return Node::Type::RETURN_STATEMENT;
}
//---------------------------------------------------------------------------
Declaration::Declaration() : declaredIdentifiers() {}
Declaration::Declaration(std::vector<Variable> declaredIdentifiers) : declaredIdentifiers(std::move(declaredIdentifiers)) {}
//---------------------------------------------------------------------------
ParamDeclaration::ParamDeclaration() = default;
ParamDeclaration::ParamDeclaration(std::vector<Variable> declaredIdentifiers) : Declaration(std::move(declaredIdentifiers)) {}

Node::Type ParamDeclaration::getType() const {
    return Node::Type::PARAM_DECLARATION;
}
//---------------------------------------------------------------------------
VarDeclaration::VarDeclaration() = default;
VarDeclaration::VarDeclaration(std::vector<Variable> declaredIdentifiers) : Declaration(std::move(declaredIdentifiers)) {}

Node::Type VarDeclaration::getType() const {
    return Node::Type::VAR_DECLARATION;
}
//---------------------------------------------------------------------------
ConstDeclaration::ConstDeclaration() = default;
ConstDeclaration::ConstDeclaration(std::vector<Variable> declaredIdentifiers, std::vector<Literal> literalValues)
    : Declaration(std::move(declaredIdentifiers)), literalValues(std::move(literalValues)) {}

Node::Type ConstDeclaration::getType() const {
    return Node::Type::CONST_DECLARATION;
}
//---------------------------------------------------------------------------
Function::Function() = default;

Node::Type Function::getType() const {
    return Node::Type::FUNCTION;
}
//---------------------------------------------------------------------------
} // namespace pljit::ast
//---------------------------------------------------------------------------