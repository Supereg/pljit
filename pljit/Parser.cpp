//
// Created by Andreas Bauer on 28.07.22.
//

#include "Parser.hpp"
#include "ParseTreeVisitor.hpp"
#include <charconv>

// TODO namespaces in the implementationf files!

// TODO check variable ordering (bigger to smaller!)

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
namespace ParseTree {
//---------------------------------------------------------------------------
Symbol::Symbol() : src_reference() {}

const SourceCodeReference& Symbol::reference() const {
    return src_reference;
}

Symbol::Symbol(SourceCodeReference src_reference) : src_reference(src_reference) {}
//---------------------------------------------------------------------------
GenericTerminal::GenericTerminal() = default;
GenericTerminal::GenericTerminal(SourceCodeReference src_reference) : Symbol(src_reference) {}

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
Literal::Literal(SourceCodeReference src_reference, long long int literalValue) : Symbol(src_reference), literalValue(literalValue) {}

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
} // namespace ParseTree
//---------------------------------------------------------------------------
using namespace ParseTree;
//---------------------------------------------------------------------------
Parser::Parser(Lexer& lexer) : lexer(&lexer) {}

// TODO are unique_ptr everywhere a better thing to do?
Result<FunctionDefinition> Parser::parse_program() { // TODO whats the cache line size? does it make sense to pass this thing directly?
    FunctionDefinition definition;
    auto maybeError = parseFunctionDefinition(definition);
    if (maybeError.has_value()) {
        return *maybeError;
    }

    return definition;
}

std::optional<SourceCodeError> Parser::parseFunctionDefinition(FunctionDefinition& destination) {
    Result<Token> result;

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(Token::TokenType::KEYWORD, Keyword::PARAM)) {
        if (auto error = parseParameterDeclarations(destination.parameterDeclarations);
            error.has_value()) {
            return error;
        }
    }

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(Token::TokenType::KEYWORD, Keyword::VAR)) {
        if (auto error = parseVariableDeclarations(destination.variableDeclarations);
            error.has_value()) {
            return error;
        }
    }

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(Token::TokenType::KEYWORD, Keyword::CONST)) {
        if (auto error = parseConstantDeclarations(destination.constantDeclarations);
            error.has_value()) {
            return error;
        }
    }

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->getType() != Token::TokenType::KEYWORD) {
        return result->makeError(SourceCodeManagement::ErrorType::ERROR, "Expected `BEGIN` keyword!");
    }

    if (auto error = parseCompoundStatement(destination.compoundStatement);
        error.has_value()) {
        return error;
    }

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    if (!result->is(Token::TokenType::SEPARATOR, Separator::END_OF_PROGRAM)) {
        return result->makeError(SourceCodeManagement::ErrorType::ERROR, "Expected `.` terminator!");
    }

    if (!lexer->endOfStream()) {
        return lexer->cur_position()
            .codeReference()
            .makeError(SourceCodeManagement::ErrorType::ERROR, "unexpected character after end of program terminator!");
    }

    return {};
}

std::optional<SourceCodeError> Parser::parseParameterDeclarations(std::optional<ParameterDeclarations>& destination) {
    ParameterDeclarations declarations;

    if (auto error = parseGenericTerminal(declarations.paramKeyword, Token::TokenType::KEYWORD, Keyword::PARAM, "Expected `PARAM` keyword!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseDeclaratorList(declarations.declaratorList);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(declarations.semicolon, Token::TokenType::SEPARATOR, Separator::SEMICOLON, "Expected `;` separator!");
        error.has_value()) {
        return error;
    }

    destination = declarations;
    return {};
}

std::optional<SourceCodeError> Parser::parseVariableDeclarations(std::optional<VariableDeclarations>& destination) {
    VariableDeclarations declarations;

    if (auto error = parseGenericTerminal(declarations.varKeyword, Token::TokenType::KEYWORD, Keyword::VAR, "Expected `VAR` keyword!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseDeclaratorList(declarations.declaratorList);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(declarations.semicolon, Token::TokenType::SEPARATOR, Separator::SEMICOLON, "Expected `;` separator!");
        error.has_value()) {
        return error;
    }

    destination = declarations;
    return {};
}

std::optional<SourceCodeError> Parser::parseConstantDeclarations(std::optional<ConstantDeclarations>& destination) {
    ConstantDeclarations declarations;

    if (auto error = parseGenericTerminal(declarations.constKeyword, Token::TokenType::KEYWORD, Keyword::CONST, "Expected `CONST` keyword!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseInitDeclaratorList(declarations.initDeclaratorList);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(declarations.semicolon, Token::TokenType::SEPARATOR, Separator::SEMICOLON, "Expected `;` separator!");
        error.has_value()) {
        return error;
    }

    destination = declarations;
    return {};
}

std::optional<SourceCodeError> Parser::parseDeclaratorList(DeclaratorList& destination) {
    if (auto error = parseIdentifier(destination.identifier);
        error.has_value()) {
        return error;
    }

    Result<Token> result;

    while (true) {
        result = lexer->peek_next();
        if (result.failure()) {
            return result.error();
        }

        if (!result->is(Token::TokenType::SEPARATOR, Separator::COMMA)) {
            break;
        }

        // mark token as consumed!
        lexer->consume(*result);

        Identifier identifier;
        if (auto error = parseIdentifier(identifier);
            error.has_value()) {
            return error;
        }

        // TODO implicit constructor a good idea?
        destination.additionalIdentifiers.emplace_back(result->reference(), identifier);
    }

    return {};
}

std::optional<SourceCodeError> Parser::parseInitDeclaratorList(InitDeclaratorList& destination) {
    if (auto error = parseInitDeclarator(destination.initDeclarator);
        error.has_value()) {
        return error;
    }

    Result<Token> result;
    while (true) {
        result = lexer->peek_next();
        if (result.failure()) {
            return result.error();
        }

        if (!result->is(Token::TokenType::SEPARATOR, Separator::COMMA)) {
            break;
        }

        // mark token as consumed!
        lexer->consume(*result);

        InitDeclarator declarator;
        if (auto error = parseInitDeclarator(declarator);
            error.has_value()) {
            return error;
        }

        destination.additionalInitDeclarators.emplace_back(result->reference(), declarator);
    }

    return {};
}

std::optional<SourceCodeError> Parser::parseInitDeclarator(InitDeclarator& destination) {
    if (auto error = parseIdentifier(destination.identifier);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(destination.initOperator, Token::TokenType::OPERATOR, Operator::INIT, "Expected `=` operator!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseLiteral(destination.literal);
        error.has_value()) {
        return error;
    }

    return {};
}

std::optional<SourceCodeError> Parser::parseCompoundStatement(CompoundStatement& destination) {
    if (auto error = parseGenericTerminal(destination.beginKeyword, Token::TokenType::KEYWORD, Keyword::BEGIN, "Expected `BEGIN` keyword!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseStatementList(destination.statementList);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(destination.endKeyword, Token::TokenType::KEYWORD, Keyword::END, "Expected `END` keyword!");
        error.has_value()) {
        return error;
    }

    return {};
}

std::optional<SourceCodeError> Parser::parseStatementList(StatementList& destination) {
    if (auto error = parseStatement(destination.statement);
        error.has_value()) {
        return error;
    }

    Result<Token> result;
    while (true) {
        result = lexer->peek_next();
        if (result.failure()) {
            return result.error();
        }

        if (!result->is(Token::TokenType::SEPARATOR, Separator::SEMICOLON)) {
            break;
        }

        lexer->consume(*result);

        Statement statement;
        if (auto error = parseStatement(statement);
            error.has_value()) {
            return error;
        }

        destination.additionalStatements.emplace_back(result->reference(), std::move(statement));
    }

    return {};
}

std::optional<SourceCodeError> Parser::parseStatement(Statement& destination) {
    Result<Token> result;

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(Token::TokenType::KEYWORD, Keyword::RETURN)) {
        lexer->consume(*result);

        AdditiveExpression expression;

        if (auto error = parseAdditiveExpression(expression);
            error.has_value()) {
            return error;
        }

        destination.type = Statement::Type::RETURN;
        destination.symbols.push_back(std::make_unique<GenericTerminal>(result->reference()));
        destination.symbols.push_back(std::make_unique<AdditiveExpression>(std::move(expression)));
    } else if (result->getType() == Token::TokenType::IDENTIFIER) {
        AssignmentExpression expression;

        if (auto error = parseAssignmentExpression(expression);
            error.has_value()) {
            return error;
        }

        destination.type = Statement::Type::ASSIGNMENT;
        destination.symbols.push_back(std::make_unique<AssignmentExpression>(std::move(expression)));
    } else {
        return result->makeError(SourceCodeManagement::ErrorType::ERROR, "Expected begin of statement. Assignment or RETURN expression!");
    }

    return {};
}

std::optional<SourceCodeError> Parser::parseAssignmentExpression(AssignmentExpression& destination) {
    if (auto error = parseIdentifier(destination.identifier);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(destination.assignmentOperator, Token::TokenType::OPERATOR, Operator::ASSIGNMENT, "Expected `:=` operator!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseAdditiveExpression(destination.additiveExpression);
        error.has_value()) {
        return error;
    }

    return {};
}

std::optional<SourceCodeError> Parser::parseAdditiveExpression(AdditiveExpression& destination) {
    if (auto error = parseMultiplicativeExpression(destination.expression);
        error.has_value()) {
        return error;
    }

    Result<Token> result;

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(Token::TokenType::OPERATOR, Operator::PLUS) || result->is(Token::TokenType::OPERATOR, Operator::MINUS)) {
        lexer->consume(*result);

        AdditiveExpression additiveExpression;

        if (auto error = parseAdditiveExpression(additiveExpression);
            error.has_value()) {
            return error;
        }

        destination.optionalOperand.emplace_back(result->reference(), std::move(additiveExpression));
    }

    return {};
}
std::optional<SourceCodeError> Parser::parseMultiplicativeExpression(MultiplicativeExpression& destination) {
    if (auto error = parseUnaryExpression(destination.expression);
        error.has_value()) {
        return error;
    }

    Result<Token> result;

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(Token::TokenType::OPERATOR, Operator::MULTIPLICATION) || result->is(Token::TokenType::OPERATOR, Operator::DIVISION)) {
        lexer->consume(*result);

        MultiplicativeExpression multiplicativeExpression;

        if (auto error = parseMultiplicativeExpression(multiplicativeExpression);
            error.has_value()) {
            return error;
        }

        destination.optionalOperand.emplace_back(result->reference(), std::move(multiplicativeExpression));
    }

    return {};
}
std::optional<SourceCodeError> Parser::parseUnaryExpression(UnaryExpression& destination) {
    Result<Token> result;

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(Token::TokenType::OPERATOR, Operator::PLUS) || result->is(Token::TokenType::OPERATOR, Operator::MINUS)) {
        lexer->consume(*result);
        destination.unaryOperator = GenericTerminal{result->reference()};
    }

    if (auto error = parsePrimaryExpression(destination.primaryExpression);
        error.has_value()) {
        return error;
    }

    return {};
}

std::optional<SourceCodeError> Parser::parsePrimaryExpression(PrimaryExpression& destination) {
    Result<Token> result;

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->getType() == Token::TokenType::IDENTIFIER) {
        Identifier identifier;

        if (auto error = parseIdentifier(identifier);
            error.has_value()) {
            return error;
        }

        destination.type = PrimaryExpression::Type::IDENTIFIER;
        destination.symbols.push_back(std::make_unique<Identifier>(identifier));
    } else if (result->getType() == Token::TokenType::LITERAL) {
        Literal literal;

        if (auto error = parseLiteral(literal);
            error.has_value()) {
            return error;
        }

        destination.type = PrimaryExpression::Type::LITERAL;
        destination.symbols.push_back(std::make_unique<Literal>(literal));
    } else if (result->is(Token::TokenType::PARENTHESIS, Parenthesis::ROUND_OPEN)) {
        GenericTerminal open;
        AdditiveExpression expression;
        GenericTerminal close;

        if (auto error = parseGenericTerminal(open, Token::TokenType::PARENTHESIS, Parenthesis::ROUND_OPEN, "Expected `(` parenthesis!");
            error.has_value()) {
            return error;
        }

        if (auto error = parseAdditiveExpression(expression);
            error.has_value()) {
            return error;
        }

        if (auto error = parseGenericTerminal(close, Token::TokenType::PARENTHESIS, Parenthesis::ROUND_CLOSE, "Expected matching `)` parenthesis!");
            error.has_value()) {
            return error->withCause(open.reference().makeError(SourceCodeManagement::ErrorType::NOTE, "opening bracket here"));
        }

        destination.type = PrimaryExpression::Type::ADDITIVE_EXPRESSION;
        destination.symbols.push_back(std::make_unique<GenericTerminal>(open));
        destination.symbols.push_back(std::make_unique<AdditiveExpression>(std::move(expression)));
        destination.symbols.push_back(std::make_unique<GenericTerminal>(close));
    } else {
        return result->makeError(SourceCodeManagement::ErrorType::ERROR, "Expected string, literal or bracketed expression!");
    }

    return {};
}

std::optional<SourceCodeError> Parser::parseIdentifier(Identifier& destination) {
    Result<Token> result;

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->getType() != Token::TokenType::IDENTIFIER) {
        return result->makeError(SourceCodeManagement::ErrorType::ERROR, "Expected string!");
    }

    destination.src_reference = result->reference();
    return {};
}

std::optional<SourceCodeError> Parser::parseLiteral(Literal& destination) {
    Result<Token> result;

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->getType() != Token::TokenType::LITERAL) {
        return result->makeError(SourceCodeManagement::ErrorType::ERROR, "Expected literal!");
    }

    std::string_view literal = result->reference().content();
    long long value;

    auto conversion = std::from_chars(literal.data(), literal.data() + literal.size(), value);

    if (conversion.ec != std::errc{}) {
        if (conversion.ec == std::errc::result_out_of_range) {
            return result->makeError(SourceCodeManagement::ErrorType::ERROR, "Integer literal is out of range. Expected singed 64-bit!");
        }
        return result->makeError(SourceCodeManagement::ErrorType::ERROR, "Encountered unexpected error parsing integer literal!");
    }

    if (conversion.ptr != literal.data() + literal.size()) {
        return result->makeError(SourceCodeManagement::ErrorType::ERROR, "Integer literal wasn't fully parsed!");
    }

    // TODO might be bad style; writing into variables?
    destination.src_reference = result->reference();
    destination.literalValue = value;

    return {};
}

std::optional<SourceCodeError> Parser::parseGenericTerminal(
    GenericTerminal& destination,
    Token::TokenType expected_type,
    std::string_view expected_content, // NOLINT(bugprone-easily-swappable-parameters)
    std::string_view potential_error_message) {
    Result<Token> result;

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    if (!result->is(expected_type, expected_content)) {
        return result->makeError(SourceCodeManagement::ErrorType::ERROR, potential_error_message);
    }

    destination.src_reference = result->reference();
    return {};
}
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------