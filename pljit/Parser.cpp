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
        return maybeError.value();
    }

    return definition;
}
std::optional<SourceCodeError> Parser::parseFunctionDefinition(ParseTree::FunctionDefinition& destination) const {
    Result<Token> result;
    std::optional<SourceCodeError> error;

    result = lexer->peek_next(); // TODO make EMPTY token an error!
    if (result.failure()) {
        return result.error();
    }


    if (result.value().getType() == Token::TokenType::KEYWORD && result.value().reference().content() == "PARAM") {
        error = parseParameterDeclarations(destination.parameterDeclarations);
        if (error.has_value()) {
            return error;
        }
    }

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result.value().getType() == Token::TokenType::KEYWORD && result.value().reference().content() == "VAR") {
        error = parseVariableDeclarations(destination.variableDeclarations);
        if (error.has_value()) {
            return error;
        }
    }

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result.value().getType() == Token::TokenType::KEYWORD && result.value().reference().content() == "CONST") {
        error = parseConstantDeclarations(destination.constantDeclarations);
        if (error.has_value()) {
            return error;
        }
    }

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result.value().getType() != Token::TokenType::KEYWORD) {
        // TODO error message (make it a member function?)
        return SourceCodeError{
            SourceCodeManagement::ErrorType::ERROR,
            "Expected one of the following keywords `PARAM`, `VAR`, `CONST` or `BEGIN`!",
            result.value().reference()
        };
    }

    parseCompoundStatement(destination.compoundStatement);

    // TODO assert end of program! (assert program terminator!)
    return {};
}

std::optional<SourceCodeError> Parser::parseParameterDeclarations(std::optional<ParseTree::ParameterDeclarations>& destination) const {
    ParseTree::ParameterDeclarations declarations;
    Result<Token> result;

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    // TODO make a method for that pattern?
    if (!(result.value().getType() == Token::TokenType::KEYWORD && result.value().reference().content() == "PARAM")) {
        // TODO we could just assert that!
        return SourceCodeError{
            SourceCodeManagement::ErrorType::ERROR,
            "Expected `PARAM` keyword",
            result.value().reference() // TODO we would also need to check for an EMPTY?
        };
    }

    declarations.paramKeyword = { result.value().reference() };

    parseDeclaratorList(declarations.declaratorList);

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    if (!(result.value().getType() == Token::TokenType::SEPARATOR && result.value().reference().content() == ";")) {
        return SourceCodeError{
            SourceCodeManagement::ErrorType::ERROR,
            "Expected `;` separator",
            result.value().reference() // TODO we would also need to check for an EMPTY?
        };
    }

    declarations.semicolon = { result.value().reference() };

    destination = declarations;
    return {};
}

std::optional<SourceCodeError> Parser::parseVariableDeclarations(std::optional<ParseTree::VariableDeclarations>& destination) const {
    // TODO this is pretty much a code duplication!
    ParseTree::VariableDeclarations declarations;
    Result<Token> result;

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    if (!(result.value().getType() == Token::TokenType::KEYWORD && result.value().reference().content() == "VAR")) {
        // TODO we could just assert that!
        return SourceCodeError{
            SourceCodeManagement::ErrorType::ERROR,
            "Expected `VAR` keyword",
            result.value().reference() // TODO we would also need to check for an EMPTY?
        };
    }

    declarations.varKeyword = { result.value().reference() };

    parseDeclaratorList(declarations.declaratorList);

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    if (!(result.value().getType() == Token::TokenType::SEPARATOR && result.value().reference().content() == ";")) {
        return SourceCodeError{
            SourceCodeManagement::ErrorType::ERROR,
            "Expected `;` separator",
            result.value().reference() // TODO we would also need to check for an EMPTY?
        };
    }

    declarations.semicolon = { result.value().reference() };

    destination = declarations;
    return {};
}

std::optional<SourceCodeError> Parser::parseConstantDeclarations(std::optional<ParseTree::ConstantDeclarations>& destination) const {
    // TODO code duplication;
    ParseTree::ConstantDeclarations declarations;
    Result<Token> result;

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    assert(result.value().getType() == Token::TokenType::KEYWORD && result.value().reference().content() == "CONST");

    // TODO do implicit construction?
    declarations.constKeyword = { result.value().reference() };

    parseInitDeclaratorList(declarations.initDeclaratorList);

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    if (!(result.value().getType() == Token::TokenType::SEPARATOR && result.value().reference().content() == ";")) {
        return SourceCodeError{
            SourceCodeManagement::ErrorType::ERROR,
            "Expected `;` separator",
            result.value().reference() // TODO we would also need to check for an EMPTY?
        };
    }

    declarations.semicolon = { result.value().reference() };

    destination = declarations;
    return {};
}

std::optional<SourceCodeError> Parser::parseDeclaratorList(ParseTree::DeclaratorList& destination) const {
    Result<Token> result;

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    if (result.value().getType() != Token::TokenType::IDENTIFIER) {
        return SourceCodeError{
            SourceCodeManagement::ErrorType::ERROR,
            "Expected identifier",
            result.value().reference()
        };
    }

    while (true) {
        result = lexer->peek_next();
        if (result.failure()) {
            return result.error();
        }

        if (!(result.value().getType() == Token::TokenType::SEPARATOR && result.value().reference().content() == ",")) {
            break;
        }

        // TODO mark already peeked result as consumed!

    }
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
