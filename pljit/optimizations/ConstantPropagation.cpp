//
// Created by Andreas Bauer on 14.08.22.
//

#include "ConstantPropagation.hpp"
#include "../ast/AST.hpp"
#include <vector>

//---------------------------------------------------------------------------
namespace pljit::ast::optimize {
//---------------------------------------------------------------------------
ConstantPropagation::ConstTableLookup::Entry::Entry() : constant(false), current_val(0) {}

bool ConstantPropagation::ConstTableLookup::Entry::isConstant() const {
    return constant;
}

long long int ConstantPropagation::ConstTableLookup::Entry::getCurrentVal() const {
    return current_val;
}

void ConstantPropagation::ConstTableLookup::Entry::updateToConstant(long long int current_val_update) {
    constant = true;
    current_val = current_val_update;
}

void ConstantPropagation::ConstTableLookup::Entry::updateToVariable() {
    constant = false;
    current_val = 0;
}

//---------------------------------------------------------------------------
ConstantPropagation::ConstantPropagation() : constTableLookup(0) {}
ConstantPropagation::ConstTableLookup::ConstTableLookup(std::size_t symbol_count) : constant_table_lookup(symbol_count) {}

ConstantPropagation::ConstTableLookup::Entry& ConstantPropagation::ConstTableLookup::operator[](symbol_id symbolId) {
    assert(symbolId > 0 && symbolId <= constant_table_lookup.size() && "Encountered illegal symbol id!");
    return constant_table_lookup[symbolId - 1];
}

//---------------------------------------------------------------------------
void ConstantPropagation::optimize(Function& function) {
    constTableLookup = ConstTableLookup{ function.symbol_count() };

    if (function.getConstDeclaration()) {
        for (auto& [variable, literal]: function.getConstDeclaration()->getConstDeclarations()) {
            constTableLookup[variable.getSymbolId()].updateToConstant(literal.value());
        }
    }

    for (auto& statement: function.getStatements()) {
        optimize(statement);
    }
}

void ConstantPropagation::optimize(std::unique_ptr<Statement>& statement) {
    optimize(statement->getExpressionPtr());

    if (statement->getType() == Node::Type::ASSIGNMENT_STATEMENT) {
        auto& assignment = static_cast<AssignmentStatement&>(*statement);

        if (statement->getExpression().getType() == Node::Type::LITERAL) {
            long long value = static_cast<const Literal&>(statement->getExpression()).value();
            constTableLookup[assignment.getVariable().getSymbolId()].updateToConstant(value);
        } else {
            constTableLookup[assignment.getVariable().getSymbolId()].updateToVariable();
        }
    }
}

void ConstantPropagation::optimize(std::unique_ptr<Expression>& expression) {
    // type might be one of the following:
    // LITERAL, VARIABLE, UNARY_PLUS, UNARY_MINUS, ADD, SUBTRACT, MULTIPLY, DIVIDE,
    auto type = expression->getType();

    if (type == Node::Type::VARIABLE) {
        auto& variable = static_cast<Variable&>(*expression);
        auto& entry = constTableLookup[variable.getSymbolId()];

        if (entry.isConstant()) {
            expression = std::make_unique<Literal>(entry.getCurrentVal());
        }
    } else if (type == Node::Type::UNARY_PLUS || type == Node::Type::UNARY_MINUS) {
        auto& unaryExpression = static_cast<UnaryExpression&>(*expression);

        optimize(unaryExpression.getChildPtr());

        if (unaryExpression.getChild().getType() == Node::Type::LITERAL) {
            long long value = static_cast<const Literal&>(unaryExpression.getChild()).value();

            if (type == Node::Type::UNARY_PLUS) {
                expression = std::make_unique<Literal>(value);
            } else {
                expression = std::make_unique<Literal>(-value);
            }
        }
    } else if (type == Node::Type::ADD || type == Node::Type::SUBTRACT
               || type == Node::Type::MULTIPLY || type == Node::Type::DIVIDE) {
        auto& binaryExpression = static_cast<BinaryExpression&>(*expression);

        optimize(binaryExpression.getLeftPtr());
        optimize(binaryExpression.getRightPtr());

        if (binaryExpression.getLeft().getType() == Node::Type::LITERAL
            && binaryExpression.getRight().getType() == Node::Type::LITERAL) {
            long long lhs_value = static_cast<const Literal&>(binaryExpression.getLeft()).value();
            long long rhs_value = static_cast<const Literal&>(binaryExpression.getRight()).value();

            if (type == Node::Type::ADD) {
                expression = std::make_unique<Literal>(lhs_value + rhs_value);
            } else if (type == Node::Type::SUBTRACT) {
                expression = std::make_unique<Literal>(lhs_value - rhs_value);
            } else if (type == Node::Type::MULTIPLY) {
                expression = std::make_unique<Literal>(lhs_value * rhs_value);
            } else if (rhs_value != 0) { // only optimized divide if we don't generate an error
                expression = std::make_unique<Literal>(lhs_value / rhs_value);
            }
        }
    }
}
//---------------------------------------------------------------------------
} // namespace pljit::ast::optimize
//---------------------------------------------------------------------------
