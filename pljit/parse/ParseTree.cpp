//
// Created by Andreas Bauer on 11.08.22.
//

#include "./ParseTree.hpp"
#include "./ParseTreeVisitor.hpp"
#include <cassert>

//---------------------------------------------------------------------------
namespace pljit::parse::ParseTree {
//---------------------------------------------------------------------------
Symbol::Symbol() : src_reference() {}

const code::SourceCodeReference& Symbol::reference() const {
    return src_reference;
}

Symbol::Symbol(code::SourceCodeReference src_reference) : src_reference(src_reference) {}
//---------------------------------------------------------------------------
GenericTerminal::GenericTerminal() = default;
GenericTerminal::GenericTerminal(code::SourceCodeReference src_reference) : Symbol(src_reference) {}

std::string_view GenericTerminal::value() const {
    return src_reference.content();
}

void GenericTerminal::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
Identifier::Identifier() = default;

std::string_view Identifier::value() const {
    return src_reference.content();
}

void Identifier::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
Literal::Literal() : literalValue(0) {}
Literal::Literal(code::SourceCodeReference src_reference, long long int literalValue) : Symbol(src_reference), literalValue(literalValue) {}

long long Literal::value() const {
    return literalValue;
}

std::string_view Literal::string_value() const { // TODO is this used?
    return src_reference.content();
}

void Literal::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
PrimaryExpression::PrimaryExpression() : type(Type::NONE), symbols(0) {}

PrimaryExpression::Type PrimaryExpression::getType() const {
    return type;
}

const Identifier& PrimaryExpression::asIdentifier() const {
    assert(type == Type::IDENTIFIER && "PrimaryExpression isn't an identifier!");
    assert(symbols.size() == 1);
    return static_cast<const Identifier&>(*symbols[0]); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
}

const Literal& PrimaryExpression::asLiteral() const {
    assert(type == Type::LITERAL && "PrimaryExpression isn't a literal!");
    assert(symbols.size() == 1);
    return static_cast<const Literal&>(*symbols[0]); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
}
std::tuple<const GenericTerminal&, const AdditiveExpression&, const GenericTerminal&> PrimaryExpression::asBracketedExpression() const {
    assert(type == Type::ADDITIVE_EXPRESSION && "PrimaryExpression isn't a bracketed additive expression!");
    assert(symbols.size() == 3);
    return std::tie(
        static_cast<const GenericTerminal&>(*symbols.at(0)), // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
        static_cast<const AdditiveExpression&>(*symbols.at(1)), // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
        static_cast<const GenericTerminal&>(*symbols.at(2)) // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    );
}

void PrimaryExpression::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
UnaryExpression::UnaryExpression() : unaryOperator(), primaryExpression() {}

const std::optional<GenericTerminal>& UnaryExpression::getUnaryOperator() const {
    return unaryOperator;
}

const PrimaryExpression& UnaryExpression::getPrimaryExpression() const {
    return primaryExpression;
}

void UnaryExpression::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
MultiplicativeExpression::MultiplicativeExpression() : expression(), optionalOperand(0) {}

const UnaryExpression& MultiplicativeExpression::getExpression() const {
    return expression;
}

std::optional<std::tuple<const GenericTerminal&, const MultiplicativeExpression&>> MultiplicativeExpression::getOperand() const {
    if (optionalOperand.empty()) {
        return {};
    }

    return { optionalOperand.at(0) };
}

void MultiplicativeExpression::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
AdditiveExpression::AdditiveExpression() : expression(), optionalOperand() {}

const MultiplicativeExpression& AdditiveExpression::getExpression() const {
    return expression;
}

std::optional<std::tuple<const GenericTerminal&, const AdditiveExpression&>> AdditiveExpression::getOperand() const {
    if (optionalOperand.empty()) {
        return {};
    }

    // TODO why does this work?
    return { optionalOperand.at(0) };
}

void AdditiveExpression::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
AssignmentExpression::AssignmentExpression() : identifier(), assignmentOperator(), additiveExpression() {}

const Identifier& AssignmentExpression::getIdentifier() const {
    return identifier;
}

const GenericTerminal& AssignmentExpression::getAssignmentOperator() const {
    return assignmentOperator;
}

const AdditiveExpression& AssignmentExpression::getAdditiveExpression() const {
    return additiveExpression;
}

void AssignmentExpression::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
Statement::Statement() : type(Type::NONE), symbols(0) {}

Statement::Type Statement::getType() const {
    return type;
}

const AssignmentExpression& Statement::asAssignmentExpression() const {
    assert(type == Type::ASSIGNMENT && "Statement isn't an assignment!");
    assert(symbols.size() == 1);
    return static_cast<const AssignmentExpression&>(*symbols.at(0)); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
}

std::tuple<const GenericTerminal&, const AdditiveExpression&> Statement::asReturnExpression() const {
    assert(type == Type::RETURN && "Statement isn't a return statement!");
    assert(symbols.size() == 2);
    return std::tie(
        static_cast<const GenericTerminal&>(*symbols.at(0)), // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
        static_cast<const AdditiveExpression&>(*symbols.at(1)) // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    );
}

void Statement::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
StatementList::StatementList() : statement(), additionalStatements(0) {}

const Statement& StatementList::getStatement() const {
    return statement;
}

const std::vector<std::tuple<GenericTerminal, Statement>>& StatementList::getAdditionalStatements() const {
    return additionalStatements;
}

void StatementList::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
CompoundStatement::CompoundStatement() : beginKeyword(), statementList(), endKeyword() {}

const GenericTerminal& CompoundStatement::getBeginKeyword() const {
    return beginKeyword;
}

const StatementList& CompoundStatement::getStatementList() const {
    return statementList;
}

const GenericTerminal& CompoundStatement::getEndKeyword() const {
    return endKeyword;
}

void CompoundStatement::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
InitDeclarator::InitDeclarator() : identifier(), initOperator(), literal() {}

const Identifier& InitDeclarator::getIdentifier() const {
    return identifier;
}

const GenericTerminal& InitDeclarator::getInitOperator() const {
    return initOperator;
}

const Literal& InitDeclarator::getLiteral() const {
    return literal;
}

void InitDeclarator::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
InitDeclaratorList::InitDeclaratorList() : initDeclarator(), additionalInitDeclarators(0) {}

const InitDeclarator& InitDeclaratorList::getInitDeclarator() const {
    return initDeclarator;

}
const std::vector<std::tuple<GenericTerminal, InitDeclarator>>& InitDeclaratorList::getAdditionalInitDeclarators() const {
    return additionalInitDeclarators;
}

void InitDeclaratorList::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
DeclaratorList::DeclaratorList() : identifier(), additionalIdentifiers(0) {}

const Identifier& DeclaratorList::getIdentifier() const {
    return identifier;
}

const std::vector<std::tuple<GenericTerminal, Identifier>>& DeclaratorList::getAdditionalIdentifiers() const {
    return additionalIdentifiers;
}

void DeclaratorList::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
ConstantDeclarations::ConstantDeclarations() : constKeyword(), initDeclaratorList(), semicolon() {}

const GenericTerminal& ConstantDeclarations::getConstKeyword() const {
    return constKeyword;
}

const InitDeclaratorList& ConstantDeclarations::getInitDeclaratorList() const {
    return initDeclaratorList;
}

const GenericTerminal& ConstantDeclarations::getSemicolon() const {
    return semicolon;
}

void ConstantDeclarations::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
VariableDeclarations::VariableDeclarations() : varKeyword(), declaratorList(), semicolon() {}

const GenericTerminal& VariableDeclarations::getVarKeyword() const {
    return varKeyword;
}

const DeclaratorList& VariableDeclarations::getDeclaratorList() const {
    return declaratorList;
}

const GenericTerminal& VariableDeclarations::getSemicolon() const {
    return semicolon;
}

void VariableDeclarations::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
ParameterDeclarations::ParameterDeclarations() : paramKeyword(), declaratorList(), semicolon() {}

const GenericTerminal& ParameterDeclarations::getParamKeyword() const {
    return paramKeyword;
}

const DeclaratorList& ParameterDeclarations::getDeclaratorList() const {
    return declaratorList;
}

const GenericTerminal& ParameterDeclarations::getSemicolon() const {
    return semicolon;
}

void ParameterDeclarations::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
FunctionDefinition::FunctionDefinition() : parameterDeclarations(), variableDeclarations(), constantDeclarations(), compoundStatement() {}

const std::optional<ParameterDeclarations>& FunctionDefinition::getParameterDeclarations() const {
    return parameterDeclarations;
}

const std::optional<VariableDeclarations>& FunctionDefinition::getVariableDeclarations() const {
    return variableDeclarations;
}

const std::optional<ConstantDeclarations>& FunctionDefinition::getConstantDeclarations() const {
    return constantDeclarations;
}

const CompoundStatement& FunctionDefinition::getCompoundStatement() const {
    return compoundStatement;
}

void FunctionDefinition::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
} // namespace pljit::parse::ParseTree
//---------------------------------------------------------------------------
