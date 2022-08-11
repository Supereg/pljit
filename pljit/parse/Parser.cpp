//
// Created by Andreas Bauer on 28.07.22.
//

#include "./Parser.hpp"
#include "pljit/lang.hpp"
#include <charconv>

// TODO namespaces in the implementationf files!

// TODO check variable ordering (bigger to smaller!)

//---------------------------------------------------------------------------
namespace pljit::parse {
//---------------------------------------------------------------------------
using namespace ParseTree;
//---------------------------------------------------------------------------
Parser::Parser(lex::Lexer& lexer) : lexer(&lexer) {}

// TODO are unique_ptr everywhere a better thing to do?
Result<FunctionDefinition> Parser::parse_program() { // TODO whats the cache line size? does it make sense to pass this thing directly?
    FunctionDefinition definition;
    auto maybeError = parseFunctionDefinition(definition);
    if (maybeError.has_value()) {
        return *maybeError;
    }

    return definition;
}

std::optional<code::SourceCodeError> Parser::parseFunctionDefinition(FunctionDefinition& destination) {
    Result<lex::Token> result;

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(lex::Token::Type::KEYWORD, Keyword::PARAM)) {
        if (auto error = parseParameterDeclarations(destination.parameterDeclarations);
            error.has_value()) {
            return error;
        }
    }

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(lex::Token::Type::KEYWORD, Keyword::VAR)) {
        if (auto error = parseVariableDeclarations(destination.variableDeclarations);
            error.has_value()) {
            return error;
        }
    }

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(lex::Token::Type::KEYWORD, Keyword::CONST)) {
        if (auto error = parseConstantDeclarations(destination.constantDeclarations);
            error.has_value()) {
            return error;
        }
    }

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->getType() != lex::Token::Type::KEYWORD) {
        return result->makeError(code::SourceCodeManagement::ErrorType::ERROR, "Expected `BEGIN` keyword!");
    }

    if (auto error = parseCompoundStatement(destination.compoundStatement);
        error.has_value()) {
        return error;
    }

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    if (!result->is(lex::Token::Type::SEPARATOR, Separator::END_OF_PROGRAM)) {
        return result->makeError(code::SourceCodeManagement::ErrorType::ERROR, "Expected `.` terminator!");
    }

    if (!lexer->endOfStream()) {
        return lexer->cur_position()
            .codeReference()
            .makeError(code::SourceCodeManagement::ErrorType::ERROR, "unexpected character after end of program terminator!");
    }

    return {};
}

std::optional<code::SourceCodeError> Parser::parseParameterDeclarations(std::optional<ParameterDeclarations>& destination) {
    ParameterDeclarations declarations;

    if (auto error = parseGenericTerminal(declarations.paramKeyword, lex::Token::Type::KEYWORD, Keyword::PARAM, "Expected `PARAM` keyword!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseDeclaratorList(declarations.declaratorList);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(declarations.semicolon, lex::Token::Type::SEPARATOR, Separator::SEMICOLON, "Expected `;` separator!");
        error.has_value()) {
        return error;
    }

    destination = declarations;
    return {};
}

std::optional<code::SourceCodeError> Parser::parseVariableDeclarations(std::optional<VariableDeclarations>& destination) {
    VariableDeclarations declarations;

    if (auto error = parseGenericTerminal(declarations.varKeyword, lex::Token::Type::KEYWORD, Keyword::VAR, "Expected `VAR` keyword!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseDeclaratorList(declarations.declaratorList);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(declarations.semicolon, lex::Token::Type::SEPARATOR, Separator::SEMICOLON, "Expected `;` separator!");
        error.has_value()) {
        return error;
    }

    destination = declarations;
    return {};
}

std::optional<code::SourceCodeError> Parser::parseConstantDeclarations(std::optional<ConstantDeclarations>& destination) {
    ConstantDeclarations declarations;

    if (auto error = parseGenericTerminal(declarations.constKeyword, lex::Token::Type::KEYWORD, Keyword::CONST, "Expected `CONST` keyword!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseInitDeclaratorList(declarations.initDeclaratorList);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(declarations.semicolon, lex::Token::Type::SEPARATOR, Separator::SEMICOLON, "Expected `;` separator!");
        error.has_value()) {
        return error;
    }

    destination = declarations;
    return {};
}

std::optional<code::SourceCodeError> Parser::parseDeclaratorList(DeclaratorList& destination) {
    if (auto error = parseIdentifier(destination.identifier);
        error.has_value()) {
        return error;
    }

    Result<lex::Token> result;

    while (true) {
        result = lexer->peek_next();
        if (result.failure()) {
            return result.error();
        }

        if (!result->is(lex::Token::Type::SEPARATOR, Separator::COMMA)) {
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

std::optional<code::SourceCodeError> Parser::parseInitDeclaratorList(InitDeclaratorList& destination) {
    if (auto error = parseInitDeclarator(destination.initDeclarator);
        error.has_value()) {
        return error;
    }

    Result<lex::Token> result;
    while (true) {
        result = lexer->peek_next();
        if (result.failure()) {
            return result.error();
        }

        if (!result->is(lex::Token::Type::SEPARATOR, Separator::COMMA)) {
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

std::optional<code::SourceCodeError> Parser::parseInitDeclarator(InitDeclarator& destination) {
    if (auto error = parseIdentifier(destination.identifier);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(destination.initOperator, lex::Token::Type::OPERATOR, Operator::INIT, "Expected `=` operator!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseLiteral(destination.literal);
        error.has_value()) {
        return error;
    }

    return {};
}

std::optional<code::SourceCodeError> Parser::parseCompoundStatement(CompoundStatement& destination) {
    if (auto error = parseGenericTerminal(destination.beginKeyword, lex::Token::Type::KEYWORD, Keyword::BEGIN, "Expected `BEGIN` keyword!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseStatementList(destination.statementList);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(destination.endKeyword, lex::Token::Type::KEYWORD, Keyword::END, "Expected `END` keyword!");
        error.has_value()) {
        return error;
    }

    return {};
}

std::optional<code::SourceCodeError> Parser::parseStatementList(StatementList& destination) {
    if (auto error = parseStatement(destination.statement);
        error.has_value()) {
        return error;
    }

    Result<lex::Token> result;
    while (true) {
        result = lexer->peek_next();
        if (result.failure()) {
            return result.error();
        }

        if (!result->is(lex::Token::Type::SEPARATOR, Separator::SEMICOLON)) {
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

std::optional<code::SourceCodeError> Parser::parseStatement(Statement& destination) {
    Result<lex::Token> result;

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(lex::Token::Type::KEYWORD, Keyword::RETURN)) {
        lexer->consume(*result);

        AdditiveExpression expression;

        if (auto error = parseAdditiveExpression(expression);
            error.has_value()) {
            return error;
        }

        destination.type = Statement::Type::RETURN;
        destination.symbols.push_back(std::make_unique<GenericTerminal>(result->reference()));
        destination.symbols.push_back(std::make_unique<AdditiveExpression>(std::move(expression)));
    } else if (result->getType() == lex::Token::Type::IDENTIFIER) {
        AssignmentExpression expression;

        if (auto error = parseAssignmentExpression(expression);
            error.has_value()) {
            return error;
        }

        destination.type = Statement::Type::ASSIGNMENT;
        destination.symbols.push_back(std::make_unique<AssignmentExpression>(std::move(expression)));
    } else {
        return result->makeError(code::SourceCodeManagement::ErrorType::ERROR, "Expected begin of statement. Assignment or RETURN expression!");
    }

    return {};
}

std::optional<code::SourceCodeError> Parser::parseAssignmentExpression(AssignmentExpression& destination) {
    if (auto error = parseIdentifier(destination.identifier);
        error.has_value()) {
        return error;
    }

    if (auto error = parseGenericTerminal(destination.assignmentOperator, lex::Token::Type::OPERATOR, Operator::ASSIGNMENT, "Expected `:=` operator!");
        error.has_value()) {
        return error;
    }

    if (auto error = parseAdditiveExpression(destination.additiveExpression);
        error.has_value()) {
        return error;
    }

    return {};
}

std::optional<code::SourceCodeError> Parser::parseAdditiveExpression(AdditiveExpression& destination) {
    if (auto error = parseMultiplicativeExpression(destination.expression);
        error.has_value()) {
        return error;
    }

    Result<lex::Token> result;

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(lex::Token::Type::OPERATOR, Operator::PLUS) || result->is(lex::Token::Type::OPERATOR, Operator::MINUS)) {
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
std::optional<code::SourceCodeError> Parser::parseMultiplicativeExpression(MultiplicativeExpression& destination) {
    if (auto error = parseUnaryExpression(destination.expression);
        error.has_value()) {
        return error;
    }

    Result<lex::Token> result;

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(lex::Token::Type::OPERATOR, Operator::MULTIPLICATION) || result->is(lex::Token::Type::OPERATOR, Operator::DIVISION)) {
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
std::optional<code::SourceCodeError> Parser::parseUnaryExpression(UnaryExpression& destination) {
    Result<lex::Token> result;

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->is(lex::Token::Type::OPERATOR, Operator::PLUS) || result->is(lex::Token::Type::OPERATOR, Operator::MINUS)) {
        lexer->consume(*result);
        destination.unaryOperator = GenericTerminal{result->reference()};
    }

    if (auto error = parsePrimaryExpression(destination.primaryExpression);
        error.has_value()) {
        return error;
    }

    return {};
}

std::optional<code::SourceCodeError> Parser::parsePrimaryExpression(PrimaryExpression& destination) {
    Result<lex::Token> result;

    result = lexer->peek_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->getType() == lex::Token::Type::IDENTIFIER) {
        Identifier identifier;

        if (auto error = parseIdentifier(identifier);
            error.has_value()) {
            return error;
        }

        destination.type = PrimaryExpression::Type::IDENTIFIER;
        destination.symbols.push_back(std::make_unique<Identifier>(identifier));
    } else if (result->getType() == lex::Token::Type::LITERAL) {
        Literal literal;

        if (auto error = parseLiteral(literal);
            error.has_value()) {
            return error;
        }

        destination.type = PrimaryExpression::Type::LITERAL;
        destination.symbols.push_back(std::make_unique<Literal>(literal));
    } else if (result->is(lex::Token::Type::PARENTHESIS, Parenthesis::ROUND_OPEN)) {
        GenericTerminal open;
        AdditiveExpression expression;
        GenericTerminal close;

        if (auto error = parseGenericTerminal(open, lex::Token::Type::PARENTHESIS, Parenthesis::ROUND_OPEN, "Expected `(` parenthesis!");
            error.has_value()) {
            return error;
        }

        if (auto error = parseAdditiveExpression(expression);
            error.has_value()) {
            return error;
        }

        if (auto error = parseGenericTerminal(close, lex::Token::Type::PARENTHESIS, Parenthesis::ROUND_CLOSE, "Expected matching `)` parenthesis!");
            error.has_value()) {
            return error->withCause(open.reference().makeError(code::SourceCodeManagement::ErrorType::NOTE, "opening bracket here"));
        }

        destination.type = PrimaryExpression::Type::ADDITIVE_EXPRESSION;
        destination.symbols.push_back(std::make_unique<GenericTerminal>(open));
        destination.symbols.push_back(std::make_unique<AdditiveExpression>(std::move(expression)));
        destination.symbols.push_back(std::make_unique<GenericTerminal>(close));
    } else {
        return result->makeError(code::SourceCodeManagement::ErrorType::ERROR, "Expected string, literal or bracketed expression!");
    }

    return {};
}

std::optional<code::SourceCodeError> Parser::parseIdentifier(Identifier& destination) {
    Result<lex::Token> result;

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->getType() != lex::Token::Type::IDENTIFIER) {
        return result->makeError(code::SourceCodeManagement::ErrorType::ERROR, "Expected string!");
    }

    destination.src_reference = result->reference();
    return {};
}

std::optional<code::SourceCodeError> Parser::parseLiteral(Literal& destination) {
    Result<lex::Token> result;

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    if (result->getType() != lex::Token::Type::LITERAL) {
        return result->makeError(code::SourceCodeManagement::ErrorType::ERROR, "Expected literal!");
    }

    std::string_view literal = result->reference().content();
    long long value;

    auto conversion = std::from_chars(literal.data(), literal.data() + literal.size(), value);

    if (conversion.ec != std::errc{}) {
        if (conversion.ec == std::errc::result_out_of_range) {
            return result->makeError(code::SourceCodeManagement::ErrorType::ERROR, "Integer literal is out of range. Expected singed 64-bit!");
        }
        return result->makeError(code::SourceCodeManagement::ErrorType::ERROR, "Encountered unexpected error parsing integer literal!");
    }

    if (conversion.ptr != literal.data() + literal.size()) {
        return result->makeError(code::SourceCodeManagement::ErrorType::ERROR, "Integer literal wasn't fully parsed!");
    }

    // TODO might be bad style; writing into variables?
    destination.src_reference = result->reference();
    destination.literalValue = value;

    return {};
}

std::optional<code::SourceCodeError> Parser::parseGenericTerminal(
    GenericTerminal& destination,
    lex::Token::Type expected_type,
    std::string_view expected_content, // NOLINT(bugprone-easily-swappable-parameters)
    std::string_view potential_error_message) {
    Result<lex::Token> result;

    result = lexer->consume_next();
    if (result.failure()) {
        return result.error();
    }

    if (!result->is(expected_type, expected_content)) {
        return result->makeError(code::SourceCodeManagement::ErrorType::ERROR, potential_error_message);
    }

    destination.src_reference = result->reference();
    return {};
}
//---------------------------------------------------------------------------
} // namespace pljit::parse
//---------------------------------------------------------------------------