//
// Created by Andreas Bauer on 11.08.22.
//

#include "./ParseTree.hpp"
#include "./ParseTreeVisitor.hpp"
#include <cassert>

//---------------------------------------------------------------------------
namespace pljit::parse {
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
    return *src_reference;
}

void GenericTerminal::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
Identifier::Identifier() = default;
Identifier::Identifier(code::SourceCodeReference src_reference) : Symbol(src_reference) {}

std::string_view Identifier::value() const {
    return *src_reference;
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

void Literal::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
PrimaryExpression::PrimaryExpression() : type(Type::NONE), symbols(0) {}
PrimaryExpression::PrimaryExpression(Identifier identifier) : Symbol(identifier.reference()), type(Type::IDENTIFIER), symbols() {
    symbols.reserve(1);
    symbols.push_back(std::make_unique<Identifier>(std::move(identifier)));
}
PrimaryExpression::PrimaryExpression(Literal literal) : Symbol(literal.reference()), type(Type::LITERAL), symbols() {
    symbols.reserve(1);
    symbols.push_back(std::make_unique<Literal>(std::move(literal)));
}
PrimaryExpression::PrimaryExpression(GenericTerminal open, AdditiveExpression additiveExpression, GenericTerminal close)
    : Symbol({ open.reference(), close.reference() }), type(Type::ADDITIVE_EXPRESSION), symbols() {
    symbols.reserve(3);
    symbols.push_back(std::make_unique<GenericTerminal>(std::move(open)));
    symbols.push_back(std::make_unique<AdditiveExpression>(std::move(additiveExpression)));
    symbols.push_back(std::make_unique<GenericTerminal>(std::move(close)));
}

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
UnaryExpression::UnaryExpression(PrimaryExpression primaryExpression) : Symbol(primaryExpression.reference()), primaryExpression(std::move(primaryExpression)) {}
UnaryExpression::UnaryExpression(GenericTerminal unaryOperator, PrimaryExpression primaryExpression)
    : Symbol({ unaryOperator.reference(), primaryExpression.reference() }), unaryOperator(std::move(unaryOperator)), primaryExpression(std::move(primaryExpression)) {}

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
MultiplicativeExpression::MultiplicativeExpression() : expression(), optionalOperand() {}
MultiplicativeExpression::MultiplicativeExpression(UnaryExpression unaryExpression)
    : Symbol(unaryExpression.reference()), expression(std::move(unaryExpression)), optionalOperand() {}
MultiplicativeExpression::MultiplicativeExpression(UnaryExpression unaryExpression, GenericTerminal op, MultiplicativeExpression multiplicativeExpression)
    : Symbol({ unaryExpression.reference(), multiplicativeExpression.reference() }), expression(std::move(unaryExpression)), optionalOperand() {
    optionalOperand.emplace_back(std::move(op), std::move(multiplicativeExpression) );
}

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
AdditiveExpression::AdditiveExpression(MultiplicativeExpression multiplicativeExpression)
    : Symbol(multiplicativeExpression.reference()), expression(std::move(multiplicativeExpression)), optionalOperand() {}
AdditiveExpression::AdditiveExpression(MultiplicativeExpression multiplicativeExpression, GenericTerminal op, AdditiveExpression additiveExpression)
    : Symbol({ multiplicativeExpression.reference(), additiveExpression.reference() }), expression(std::move(multiplicativeExpression)), optionalOperand() {
    optionalOperand.emplace_back(std::move(op), std::move(additiveExpression));
}

const MultiplicativeExpression& AdditiveExpression::getExpression() const {
    return expression;
}

std::optional<std::tuple<const GenericTerminal&, const AdditiveExpression&>> AdditiveExpression::getOperand() const {
    if (optionalOperand.empty()) {
        return {};
    }

    return { optionalOperand.at(0) };
}

void AdditiveExpression::accept(ParseTreeVisitor& visitor) const {
    visitor.visit(*this);
}
//---------------------------------------------------------------------------
AssignmentExpression::AssignmentExpression() : identifier(), assignmentOperator(), additiveExpression() {}
AssignmentExpression::AssignmentExpression(Identifier identifier, GenericTerminal op, AdditiveExpression additiveExpression)
    : Symbol({ identifier.reference(), additiveExpression.reference() }), identifier(std::move(identifier)), assignmentOperator(std::move(op)), additiveExpression(std::move(additiveExpression)) {}

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
Statement::Statement(AssignmentExpression assignmentExpression) : Symbol(assignmentExpression.reference()), type(Type::ASSIGNMENT), symbols() {
    symbols.reserve(1);
    symbols.push_back(std::make_unique<AssignmentExpression>(std::move(assignmentExpression)));
}
Statement::Statement(GenericTerminal returnKeyword, AdditiveExpression additiveExpression)
    : Symbol({ returnKeyword.reference(), additiveExpression.reference() }), type(Type::RETURN), symbols() {
    symbols.reserve(2);
    symbols.push_back(std::make_unique<GenericTerminal>(std::move(returnKeyword)));
    symbols.push_back(std::make_unique<AdditiveExpression>(std::move(additiveExpression)));
}

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
StatementList::StatementList(Statement statement) : Symbol(statement.reference()), statement(std::move(statement)), additionalStatements() {}

void StatementList::appendStatement(GenericTerminal separator, Statement additionalStatement) {
    src_reference = { src_reference, additionalStatement.reference() };
    additionalStatements.emplace_back(std::move(separator), std::move(additionalStatement));
}

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
CompoundStatement::CompoundStatement(GenericTerminal beginKeyword, StatementList statementList, GenericTerminal endKeyword)
    : Symbol({ beginKeyword.reference(), endKeyword.reference()}), beginKeyword(std::move(beginKeyword)), statementList(std::move(statementList)), endKeyword(std::move(endKeyword)) {}

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
InitDeclarator::InitDeclarator(Identifier identifier, GenericTerminal initOperator, Literal literal)
    : Symbol({ identifier.reference(), literal.reference() }), identifier(std::move(identifier)), initOperator(std::move(initOperator)), literal(std::move(literal)) {}

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
InitDeclaratorList::InitDeclaratorList(InitDeclarator initDeclarator) : Symbol(initDeclarator.reference()), initDeclarator(std::move(initDeclarator)), additionalInitDeclarators() {}

void InitDeclaratorList::appendInitDeclarator(GenericTerminal separator, InitDeclarator additionalInitDeclarator) {
    src_reference = { src_reference, additionalInitDeclarator.reference() };
    additionalInitDeclarators.emplace_back(std::move(separator), std::move(additionalInitDeclarator));
}

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
DeclaratorList::DeclaratorList(Identifier identifier) : Symbol(identifier.reference()), identifier(std::move(identifier)), additionalIdentifiers() {}

void DeclaratorList::appendIdentifier(GenericTerminal separator, Identifier additionalIdentifier) {
    src_reference = { src_reference, additionalIdentifier.reference() };
    additionalIdentifiers.emplace_back(std::move(separator), std::move(additionalIdentifier));
}

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
ConstantDeclarations::ConstantDeclarations(GenericTerminal constKeyword, InitDeclaratorList initDeclaratorList, GenericTerminal semicolon)
    : Symbol({ constKeyword.reference(), semicolon.reference() }), constKeyword(std::move(constKeyword)), initDeclaratorList(std::move(initDeclaratorList)), semicolon(std::move(semicolon)) {}

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
VariableDeclarations::VariableDeclarations(GenericTerminal varKeyword, DeclaratorList declaratorList, GenericTerminal semicolon)
    : Symbol({ varKeyword.reference(), semicolon.reference() }), varKeyword(std::move(varKeyword)), declaratorList(std::move(declaratorList)), semicolon(std::move(semicolon)) {}

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
ParameterDeclarations::ParameterDeclarations(GenericTerminal paramKeyword, DeclaratorList declaratorList, GenericTerminal semicolon)
    : Symbol({ paramKeyword.reference(), semicolon.reference() }), paramKeyword(std::move(paramKeyword)), declaratorList(std::move(declaratorList)), semicolon(std::move(semicolon)) {}

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
FunctionDefinition::FunctionDefinition(
    std::optional<ParameterDeclarations> parameterDeclarations,
    std::optional<VariableDeclarations> variableDeclarations,
    std::optional<ConstantDeclarations> constantDeclarations,
    CompoundStatement compoundStatement,
    GenericTerminal terminator)
    : Symbol(), parameterDeclarations(std::move(parameterDeclarations)), variableDeclarations(std::move(variableDeclarations)),
      constantDeclarations(std::move(constantDeclarations)), compoundStatement(std::move(compoundStatement)), terminator(std::move(terminator)) {
    code::SourceCodeReference beginReference;
    if (FunctionDefinition::parameterDeclarations) {
        beginReference = FunctionDefinition::parameterDeclarations->reference();
    } else if (FunctionDefinition::variableDeclarations) {
        beginReference = FunctionDefinition::variableDeclarations->reference();
    } else if (FunctionDefinition::constantDeclarations) {
        beginReference = FunctionDefinition::constantDeclarations->reference();
    } else {
        beginReference = FunctionDefinition::compoundStatement.reference();
    }

    src_reference = { beginReference, FunctionDefinition::terminator.reference() };
}

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
} // namespace pljit::parse
//---------------------------------------------------------------------------
