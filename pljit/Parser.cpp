//
// Created by Andreas Bauer on 28.07.22.
//

#include "Parser.hpp"

//---------------------------------------------------------------------------
using namespace pljit;
//---------------------------------------------------------------------------
Parser::Parser(Lexer& lexer) : lexer(&lexer) {}

Result<ParseTree::FunctionDefinition> Parser::parse() const { // TODO whats the cache line size? does it make sense to pass this thing directly?
    ParseTree::FunctionDefinition definition;
    auto maybeError = parseFunctionDefinition(definition);
    if (maybeError.has_value()) {
        return *maybeError;
    }

    return definition;
}

std::optional<SourceCodeError> Parser::parseFunctionDefinition(ParseTree::FunctionDefinition& destination) const {
    Result<Token> result;

    result = lexer->peek_next(); // TODO make EMPTY token an error!
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
        return result->makeError(
            SourceCodeManagement::ErrorType::ERROR,
            "Expected one of the following keywords `PARAM`, `VAR`, `CONST` or `BEGIN`!");
    }

    if (auto error = parseCompoundStatement(destination.compoundStatement);
        error.has_value()) {
        return error;
    }

    // TODO assert end of program! (assert program terminator!)
    return {};
}

std::optional<SourceCodeError> Parser::parseParameterDeclarations(std::optional<ParseTree::ParameterDeclarations>& destination) const {
    ParseTree::ParameterDeclarations declarations;

    // TODO we would also need to check for an EMPTY Token?

    if (auto error = parseGenericTerminal(destination->paramKeyword, Token::TokenType::KEYWORD, Keyword::PARAM, "Expected `PARAM` keyword!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseDeclaratorList(declarations.declaratorList);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(destination->semicolon, Token::TokenType::SEPARATOR, Separator::SEMICOLON, "Expected `;` separator!");
        error.has_value()) {
        return error;
    }

    destination = declarations;
    return {};
}

std::optional<SourceCodeError> Parser::parseVariableDeclarations(std::optional<ParseTree::VariableDeclarations>& destination) const {
    // TODO this is pretty much a code duplication!
    ParseTree::VariableDeclarations declarations;

    if (auto error = parseGenericTerminal(destination->varKeyword, Token::TokenType::KEYWORD, Keyword::VAR, "Expected `VAR` keyword!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseDeclaratorList(declarations.declaratorList);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(destination->semicolon, Token::TokenType::SEPARATOR, Separator::SEMICOLON, "Expected `;` separator!");
        error.has_value()) {
        return error;
    }

    destination = declarations;
    return {};
}

std::optional<SourceCodeError> Parser::parseConstantDeclarations(std::optional<ParseTree::ConstantDeclarations>& destination) const {
    // TODO code duplication;
    ParseTree::ConstantDeclarations declarations;

    if (auto error = parseGenericTerminal(destination->constKeyword, Token::TokenType::KEYWORD, Keyword::CONST, "Expected `CONST` keyword!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseInitDeclaratorList(declarations.initDeclaratorList);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(destination->semicolon, Token::TokenType::SEPARATOR, Separator::SEMICOLON, "Expected `;` separator!");
        error.has_value()) {
        return error;
    }

    destination = declarations;
    return {};
}

std::optional<SourceCodeError> Parser::parseDeclaratorList(ParseTree::DeclaratorList& destination) const {
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

        ParseTree::Identifier identifier;
        if (auto error = parseIdentifier(identifier);
            error.has_value()) {
            return error;
        }

        // TODO implicit constructor a good idea?
        destination.additionalIdentifiers.emplace_back(result->reference(), identifier);
    }

    return {};
}

std::optional<SourceCodeError> Parser::parseInitDeclaratorList(ParseTree::InitDeclaratorList& destination) const {
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

        ParseTree::InitDeclarator declarator;
        if (auto error = parseInitDeclarator(declarator);
            error.has_value()) {
            return error;
        }

        destination.additionalInitDeclarators.emplace_back(result->reference(), declarator);
    }
}

std::optional<SourceCodeError> Parser::parseInitDeclarator(ParseTree::InitDeclarator& destination) const {
    if (auto error = parseIdentifier(destination.identifier);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(destination.assignmentOperator, Token::TokenType::OPERATOR, Operator::INIT, "Expected `=` operator!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseLiteral(destination.literal);
        error.has_value()) {
        return error;
    }

    return {};
}

std::optional<SourceCodeError> Parser::parseCompoundStatement(ParseTree::CompoundStatement& destination) const {
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

std::optional<SourceCodeError> Parser::parseStatementList(ParseTree::StatementList& destination) const {
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

        ParseTree::Statement statement;
        if (auto error = parseStatement(statement);
            error.has_value()) {
            return error;
        }

        destination.additionalStatements.emplace_back(result->reference(), std::move(statement));
    }

    return {};
}

std::optional<SourceCodeError> Parser::parseStatement(ParseTree::Statement& destination) const {
    Result<Token> result;

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(Token::TokenType::KEYWORD, Keyword::RETURN)) {

    }
    // TODO parse statment! with vector and ptr!

    return {};
}

std::optional<SourceCodeError> Parser::parseAssignmentExpression(ParseTree::AssignmentExpression& destination) const {
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

std::optional<SourceCodeError> Parser::parseAdditiveExpression(ParseTree::AdditiveExpression& destination) const {
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

        ParseTree::AdditiveExpression additiveExpression;

        if (auto error = parseAdditiveExpression(additiveExpression);
            error.has_value()) {
            return error;
        }

        destination.optionalOperand.emplace_back(result->reference(), std::move(additiveExpression));
    }

    return {};
}
std::optional<SourceCodeError> Parser::parseMultiplicativeExpression(ParseTree::MultiplicativeExpression& destination) const {
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

        ParseTree::MultiplicativeExpression multiplicativeExpression;

        if (auto error = parseMultiplicativeExpression(multiplicativeExpression);
            error.has_value()) {
            return error;
        }

        destination.optionalOperand.emplace_back(result->reference(), std::move(multiplicativeExpression));
    }

    return {};
}
std::optional<SourceCodeError> Parser::parseUnaryExpression(ParseTree::UnaryExpression& destination) const {
    Result<Token> result;

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(Token::TokenType::OPERATOR, Operator::PLUS) || result->is(Token::TokenType::OPERATOR, Operator::MINUS)) {
        destination.unaryOperator = result->reference();
    }

    if (auto error = parsePrimaryExpression(destination.primaryExpression);
        error.has_value()) {
        return error;
    }

    return {};
}

std::optional<SourceCodeError> Parser::parsePrimaryExpression(ParseTree::PrimaryExpression& destination) const {
    return std::optional<SourceCodeError>();
}

std::optional<SourceCodeError> Parser::parseIdentifier(ParseTree::Identifier& destination) const {
    Result<Token> result;

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->getType() != Token::TokenType::IDENTIFIER) {
        return result->makeError(SourceCodeManagement::ErrorType::ERROR, "Expected identifier!");
    }

    destination = result->reference();
    return {};
}
std::optional<SourceCodeError> Parser::parseLiteral(ParseTree::Literal& destination) const {
    return std::optional<SourceCodeError>();
}
std::optional<SourceCodeError> Parser::parseGenericTerminal(
    ParseTree::GenericTerminal& destination,
    Token::TokenType expected_type,
    std::string_view expected_content, // NOLINT(bugprone-easily-swappable-parameters)
    std::string_view potential_error_message
) const {
    Result<Token> result;

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    if (!result->is(expected_type, expected_content)) {
        return result->makeError(SourceCodeManagement::ErrorType::ERROR, potential_error_message);
    }

    destination = result->reference();
    return {};
}
//---------------------------------------------------------------------------

// TODO placement of those below!
ParseTree::GenericTerminal::GenericTerminal() : reference() {}
ParseTree::GenericTerminal::GenericTerminal(SourceCodeReference reference) : reference(reference) {}

ParseTree::Identifier::Identifier() : reference() {}
ParseTree::Identifier::Identifier(SourceCodeReference reference) : reference(reference) {}

ParseTree::Literal::Literal() : reference(), literalValue(0) {}
ParseTree::Literal::Literal(SourceCodeReference reference, long long int literalValue) : reference(reference), literalValue(literalValue) {}

ParseTree::PrimaryExpression::PrimaryExpression() : type(Type::NONE), symbols(0) {}

ParseTree::UnaryExpression::UnaryExpression() : unaryOperator(), primaryExpression() {}

ParseTree::MultiplicativeExpression::MultiplicativeExpression() : expression(), optionalOperand(0) {}

ParseTree::AdditiveExpression::AdditiveExpression() : expression(), optionalOperand() {}

ParseTree::AssignmentExpression::AssignmentExpression() : identifier(), assignmentOperator(), additiveExpression() {}


ParseTree::Statement::Statement() : type(Type::NONE), symbols(0) {}

ParseTree::StatementList::StatementList() : statement(), additionalStatements(0) {}

ParseTree::CompoundStatement::CompoundStatement() : beginKeyword(), statementList(), endKeyword() {}

ParseTree::InitDeclarator::InitDeclarator() : identifier(), assignmentOperator(), literal() {}

ParseTree::InitDeclaratorList::InitDeclaratorList() : initDeclarator(), additionalInitDeclarators(0) {}

ParseTree::DeclaratorList::DeclaratorList() : identifier(), additionalIdentifiers(0) {}

ParseTree::ConstantDeclarations::ConstantDeclarations() : constKeyword(), initDeclaratorList(), semicolon() {}

ParseTree::VariableDeclarations::VariableDeclarations() : varKeyword(), declaratorList(), semicolon() {}

ParseTree::ParameterDeclarations::ParameterDeclarations() : paramKeyword(), declaratorList(), semicolon() {}

ParseTree::FunctionDefinition::FunctionDefinition() : parameterDeclarations(), variableDeclarations(), constantDeclarations(), compoundStatement() {}
