//
// Created by Andreas Bauer on 28.07.22.
//

#include "Parser.hpp"

using namespace pljit;

Parser::Parser(Lexer& lexer) : lexer(&lexer) {}



Result<ParseTree::FunctionDefinition> Parser::parse() const { // TODO whats the cache line size? does it make sense to pass this thing directly?
    ParseTree::FunctionDefinition definition;
    auto maybeError = parseFunctionDefinition(definition);
    if (maybeError.has_value()) {
        return maybeError.value();
    }

    return { definition }; // TODO why isn't this implicitly constructible?
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

// TODO placement of those below!
ParseTree::FunctionDefinition::FunctionDefinition() : parameterDeclarations(), variableDeclarations(), constantDeclarations(), compoundStatement() {}

ParseTree::CompoundStatement::CompoundStatement() : beginKeyword(), statementList(), endKeyword() {}

ParseTree::GenericTerminal::GenericTerminal() : reference() {}
ParseTree::GenericTerminal::GenericTerminal(SourceCodeReference reference) : reference(reference) {}
ParseTree::StatementList::StatementList() : statement(), additionalStatements(0) {}
ParseTree::Statement::Statement() : type(Type::NONE)/*, symbols(0)*/ {}
