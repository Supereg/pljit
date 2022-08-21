//
// Created by Andreas Bauer on 30.07.22.
//

#include "./AST.hpp"
#include <utility>

//---------------------------------------------------------------------------
namespace pljit::ast {
//---------------------------------------------------------------------------
Literal::Literal(long long literal_value) : literal_value(literal_value) {}

Node::Type Literal::getType() const {
    return Node::Type::LITERAL;
}

void Literal::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}

std::optional<long long> Literal::evaluate(EvaluationContext&) const {
    return literal_value;
}

long long Literal::value() const {
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

std::optional<long long> Variable::evaluate(EvaluationContext& context) const {
    return context[symbolId];
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

std::unique_ptr<Expression>& UnaryExpression::getChildPtr() {
    return child;
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

std::unique_ptr<Expression>& BinaryExpression::getLeftPtr() {
    return leftChild;
}

std::unique_ptr<Expression>& BinaryExpression::getRightPtr() {
    return rightChild;
}
//---------------------------------------------------------------------------
UnaryPlus::UnaryPlus(std::unique_ptr<Expression> child) : UnaryExpression(std::move(child)) {}

Node::Type UnaryPlus::getType() const {
    return Node::Type::UNARY_PLUS;
}

void UnaryPlus::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}

std::optional<long long> UnaryPlus::evaluate(EvaluationContext& context) const {
    return child->evaluate(context);
}
//---------------------------------------------------------------------------
UnaryMinus::UnaryMinus(std::unique_ptr<Expression> child) : UnaryExpression(std::move(child)) {}

Node::Type UnaryMinus::getType() const {
    return Node::Type::UNARY_MINUS;
}

void UnaryMinus::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}

std::optional<long long> UnaryMinus::evaluate(EvaluationContext& context) const {
    std::optional<long long> value = child->evaluate(context);
    if (!value) {
        return value;
    }
    
    return -(*value);
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

std::optional<long long> Add::evaluate(EvaluationContext& context) const {
    std::optional<long long> lhs = leftChild->evaluate(context);
    if (!lhs) {
        return lhs;
    }

    std::optional<long long> rhs = rightChild->evaluate(context);
    if (!rhs) {
        return rhs;
    }


    return *lhs + *rhs;
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

std::optional<long long> Subtract::evaluate(EvaluationContext& context) const {
    std::optional<long long> lhs = leftChild->evaluate(context);
    if (!lhs) {
        return lhs;
    }

    std::optional<long long> rhs = rightChild->evaluate(context);
    if (!rhs) {
        return rhs;
    }


    return *lhs - *rhs;
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

std::optional<long long> Multiply::evaluate(EvaluationContext& context) const {
    std::optional<long long> lhs = leftChild->evaluate(context);
    if (!lhs) {
        return lhs;
    }

    std::optional<long long> rhs = rightChild->evaluate(context);
    if (!rhs) {
        return rhs;
    }


    return *lhs * *rhs;
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

std::optional<long long> Divide::evaluate(EvaluationContext& context) const {
    std::optional<long long> lhs = leftChild->evaluate(context);
    if (!lhs) {
        return lhs;
    }

    std::optional<long long> rhs = rightChild->evaluate(context);
    if (!rhs) {
        return rhs;
    }

    if (*rhs == 0) {
        // we were informed that we don't need to provide line numbers here!
        context.setRuntimeError("Division by zero!");
        return {};
    }

    return *lhs / *rhs;
}
//---------------------------------------------------------------------------
Statement::Statement(std::unique_ptr<Expression> expression) : expression(std::move(expression)) {}

const Expression& Statement::getExpression() const {
    return *expression;
}

std::unique_ptr<Expression>& Statement::getExpressionPtr() {
    return expression;
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

void AssignmentStatement::evaluate(EvaluationContext& context) const {
    std::optional<long long> value = expression->evaluate(context);
    if (!value) {
        return;
    }

    context[variable.getSymbolId()] = *value;
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

void ReturnStatement::evaluate(EvaluationContext& context) const {
    std::optional<long long> value = expression->evaluate(context);
    if (!value) {
        return;
    }

    context.return_value() = *value;
}
//---------------------------------------------------------------------------
Declaration::Declaration() : declaredIdentifiers() {}
Declaration::Declaration(std::vector<Variable> declaredIdentifiers) : declaredIdentifiers(std::move(declaredIdentifiers)) {}

const std::vector<Variable>& Declaration::getDeclaredIdentifiers() const {
    return declaredIdentifiers;
}
//---------------------------------------------------------------------------
ParamDeclaration::ParamDeclaration() = default;
ParamDeclaration::ParamDeclaration(std::vector<Variable> declaredIdentifiers)
    : Declaration(std::move(declaredIdentifiers)) {}

Node::Type ParamDeclaration::getType() const {
    return Node::Type::PARAM_DECLARATION;
}

void ParamDeclaration::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}

void ParamDeclaration::evaluate(EvaluationContext& context, std::vector<long long> arguments) const {
    if (arguments.size() > declaredIdentifiers.size()) {
        context.setRuntimeError("Received to many arguments!");
        return;
    } else if (arguments.size() < declaredIdentifiers.size()) {
        context.setRuntimeError("Received to few arguments!");
        return;
    }

    for (std::size_t index = 0; index < arguments.size(); ++index) {
        symbol_id symbol = declaredIdentifiers[index].getSymbolId();
        context[symbol] = arguments[index];
    }
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

void ConstDeclaration::evaluate(EvaluationContext& context) const {
    for (auto& [variable, literal]: getConstDeclarations()) {
        context[variable.getSymbolId()] = literal.value();
    }
}

std::vector<std::tuple<const Variable&, const Literal&>> ConstDeclaration::getConstDeclarations() const {
    std::vector<std::tuple<const Variable&, const Literal&>> vector;
    assert(literalValues.size() == declaredIdentifiers.size() && "Reached inconsistent state for ConstDeclaration!");
    vector.reserve(literalValues.size());

    // The `zip` view in the ranges library is only available with C++23, so this is the best and most efficient way
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
Function::Function() : total_symbols(0) {}
Function::Function(
    std::optional<ParamDeclaration> paramDeclaration,
    std::optional<VarDeclaration> varDeclaration,
    std::optional<ConstDeclaration> constDeclaration,
    std::vector<std::unique_ptr<Statement>> statements,
    size_t totalSymbols)
    : paramDeclaration(std::move(paramDeclaration)), varDeclaration(std::move(varDeclaration)), constDeclaration(std::move(constDeclaration)),
      statements(std::move(statements)),
      total_symbols(totalSymbols) {}

Node::Type Function::getType() const {
    return Node::Type::FUNCTION;
}

void Function::accept(ASTVisitor& visitor) const {
    visitor.visit(*this);
}

EvaluationContext Function::evaluate(const std::vector<long long>& arguments) const {
    EvaluationContext context{ total_symbols };

    if (paramDeclaration) {
        paramDeclaration->evaluate(context, arguments);
    } else if (!arguments.empty()) {
        context.setRuntimeError("Provided arguments to function with missing PARAM declaration!");
    }

    if (context.runtime_error()) {
        return context;
    }

    if (constDeclaration) {
        constDeclaration->evaluate(context);
    }

    for (auto& statement: statements) {
        statement->evaluate(context);

        // With the assumption that the dead code elimination optimization was run, we could omit this check.
        // However, we don't want to build upon this assumption.
        if (context.return_value() || context.runtime_error()) {
            return context;
        }
    }

    // we know for a fact, that ASTBuilder checks for the existence of a RETURN statement!
    assert(false && "Fatal error occurred. Illegal AST. No return statement was provided!");
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

std::vector<std::unique_ptr<Statement>>& Function::getStatements() {
    return statements;
}

std::size_t Function::symbol_count() const {
    return total_symbols;
}
//---------------------------------------------------------------------------
} // namespace pljit::ast
//---------------------------------------------------------------------------