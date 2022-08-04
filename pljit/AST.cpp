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

void Literal::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}
long long int Literal::value() const {
    return literal_value;
}
//---------------------------------------------------------------------------
Variable::Variable(symbol_id symbolId, std::string_view name) : symbolId(symbolId), name(name) {}

Node::Type Variable::getType() const {
    return Node::Type::VARIABLE;
}

void Variable::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}

symbol_id Variable::getSymbolId() const {
    return symbolId;
}

const std::string_view& Variable::getName() const {
    return name;
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

void UnaryPlus::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
UnaryMinus::UnaryMinus(std::unique_ptr<Expression> child) : UnaryExpression(std::move(child)) {}

Node::Type UnaryMinus::getType() const {
    return Node::Type::UNARY_MINUS;
}

void UnaryMinus::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
Add::Add(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild)
    : BinaryExpression(std::move(leftChild), std::move(rightChild)) {}

Node::Type Add::getType() const {
    return Node::Type::ADD;
}

void Add::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
Subtract::Subtract(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild)
    : BinaryExpression(std::move(leftChild), std::move(rightChild)) {}

Node::Type Subtract::getType() const {
    return Node::Type::SUBTRACT;
}

void Subtract::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
Multiply::Multiply(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild)
    : BinaryExpression(std::move(leftChild), std::move(rightChild)) {}

Node::Type Multiply::getType() const {
    return Node::Type::MULTIPLY;
}

void Multiply::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
Divide::Divide(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild)
    : BinaryExpression(std::move(leftChild), std::move(rightChild)) {}

Node::Type Divide::getType() const {
    return Node::Type::DIVIDE;
}

void Divide::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
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

void AssignmentStatement::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}

const Variable& AssignmentStatement::getVariable() const {
    return variable;
}
//---------------------------------------------------------------------------
ReturnStatement::ReturnStatement(std::unique_ptr<Expression> expression) : Statement(std::move(expression)) {}

Node::Type ReturnStatement::getType() const {
    return Node::Type::RETURN_STATEMENT;
}

void ReturnStatement::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
Declaration::Declaration() : declaredIdentifiers() {}
Declaration::Declaration(std::vector<Variable> declaredIdentifiers) : declaredIdentifiers(std::move(declaredIdentifiers)) {}

const std::vector<Variable>& Declaration::getDeclaredIdentifiers() const {
    return declaredIdentifiers;
}
//---------------------------------------------------------------------------
ParamDeclaration::ParamDeclaration() = default;
ParamDeclaration::ParamDeclaration(std::vector<Variable> declaredIdentifiers) : Declaration(std::move(declaredIdentifiers)) {}

Node::Type ParamDeclaration::getType() const {
    return Node::Type::PARAM_DECLARATION;
}

void ParamDeclaration::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
VarDeclaration::VarDeclaration() = default;
VarDeclaration::VarDeclaration(std::vector<Variable> declaredIdentifiers) : Declaration(std::move(declaredIdentifiers)) {}

Node::Type VarDeclaration::getType() const {
    return Node::Type::VAR_DECLARATION;
}

void VarDeclaration::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
ConstDeclaration::ConstDeclaration() = default;
ConstDeclaration::ConstDeclaration(std::vector<Variable> declaredIdentifiers, std::vector<Literal> literalValues)
    : Declaration(std::move(declaredIdentifiers)), literalValues(std::move(literalValues)) {}

Node::Type ConstDeclaration::getType() const {
    return Node::Type::CONST_DECLARATION;
}

void ConstDeclaration::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}

std::vector<std::tuple<const Variable&, const Literal&>> ConstDeclaration::getConstDeclarations() const {
    std::vector<std::tuple<const Variable&, const Literal&>> vector;
    assert(literalValues.size() == declaredIdentifiers.size() && "Reached inconsistent state for ConstDeclaration!");
    vector.reserve(literalValues.size());

    // The `zip` view in the ranges library is only available with C++, so this is the best and most efficient way
    // I could come up with. Splitting the two vectors in the first place is a result of reusing the `Declaration`
    // class. I don't think it is worth caching the result, as it is typically only required once in the program execution.

    auto identifierIterator = declaredIdentifiers.begin();
    auto literalIterator = literalValues.begin();
    for (;
         identifierIterator != declaredIdentifiers.end() && literalIterator != literalValues.end();
         ++identifierIterator, ++literalIterator) {
        vector.emplace_back(std::tie(
            *identifierIterator,
            *literalIterator
        ));
    }

    return vector;
}
//---------------------------------------------------------------------------
Function::Function() = default;

Node::Type Function::getType() const {
    return Node::Type::FUNCTION;
}

void Function::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}

const std::optional<ParamDeclaration>& Function::getParamDeclaration() const {
    return paramDeclaration;
}

const std::optional<VarDeclaration>& Function::getVarDeclaration() const {
    return varDeclaration;
}

const std::optional<ConstDeclaration>& Function::getConstDeclaration() const {
    return constDeclaration;
}

const std::vector<std::unique_ptr<Statement>>& Function::getStatements() const {
    return statements;
}
//---------------------------------------------------------------------------
} // namespace pljit::ast
//---------------------------------------------------------------------------