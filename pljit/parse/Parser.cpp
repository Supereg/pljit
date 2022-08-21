//
// Created by Andreas Bauer on 28.07.22.
//

#include "./Parser.hpp"
#include "../lang.hpp"
#include <charconv>

//---------------------------------------------------------------------------
namespace pljit::parse {
//---------------------------------------------------------------------------
Parser::Parser(lex::Lexer& lexer) : lexer(&lexer) {}

Result<FunctionDefinition> Parser::parse_program() {
    return parseFunctionDefinition();
}

Result<FunctionDefinition> Parser::parseFunctionDefinition() {
    Result<lex::Token> result;

    std::optional<ParameterDeclarations> parameterDeclarations;
    std::optional<VariableDeclarations> variableDeclarations;
    std::optional<ConstantDeclarations> constantDeclarations;

    result = lexer->peek_next();
    if (!result) {
        return result.error();
    }

    if (result->is(lex::Token::Type::KEYWORD, Keyword::PARAM)) {
        Result<ParameterDeclarations> declarations = parseParameterDeclarations();
        if (!declarations) {
            return declarations.error();
        }
        parameterDeclarations = declarations.release();
    }

    result = lexer->peek_next();
    if (!result) {
        return result.error();
    }

    if (result->is(lex::Token::Type::KEYWORD, Keyword::VAR)) {
        Result<VariableDeclarations> declarations = parseVariableDeclarations();
        if (!declarations) {
            return declarations.error();
        }
        variableDeclarations = declarations.release();
    } else if (result->is(lex::Token::Type::KEYWORD, Keyword::PARAM) && parameterDeclarations) {
        return result->makeError(code::ErrorType::ERROR, "Duplicate PARAM declaration!")
            .attachCause(parameterDeclarations->reference().makeError(code::ErrorType::NOTE, "Original declaration here"));
    }

    result = lexer->peek_next();
    if (!result) {
        return result.error();
    }

    if (result->is(lex::Token::Type::KEYWORD, Keyword::CONST)) {
        Result<ConstantDeclarations> declarations = parseConstantDeclarations();
        if (!declarations) {
            return declarations.error();
        }
        constantDeclarations = declarations.release();
    } else if (result->is(lex::Token::Type::KEYWORD, Keyword::PARAM)) {
        if (parameterDeclarations) {
            return result->makeError(code::ErrorType::ERROR, "Duplicate PARAM declaration!")
                .attachCause(parameterDeclarations->reference().makeError(code::ErrorType::NOTE, "Original declaration here"));
        } else {
            return result->makeError(code::ErrorType::ERROR, "PARAM declaration must appear before VAR declaration!");
        }
    } else if (result->is(lex::Token::Type::KEYWORD, Keyword::VAR) && variableDeclarations) {
        return result->makeError(code::ErrorType::ERROR, "Duplicate VAR declaration!")
            .attachCause(variableDeclarations->reference().makeError(code::ErrorType::NOTE, "Original declaration here"));
    }

    result = lexer->peek_next();
    if (!result) {
        return result.error();
    }

    if (result->is(lex::Token::Type::KEYWORD, Keyword::PARAM)) {
        if (parameterDeclarations) {
            return result->makeError(code::ErrorType::ERROR, "Duplicate PARAM declaration!")
                .attachCause(parameterDeclarations->reference().makeError(code::ErrorType::NOTE, "Original declaration here"));
        } else {
            return result->makeError(code::ErrorType::ERROR, "PARAM declaration must appear before CONST and VAR declarations!");
        }
    } else if (result->is(lex::Token::Type::KEYWORD, Keyword::VAR)) {
        if (variableDeclarations) {
            return result->makeError(code::ErrorType::ERROR, "Duplicate VAR declaration!")
                .attachCause(variableDeclarations->reference().makeError(code::ErrorType::NOTE, "Original declaration here"));
        } else {
            return result->makeError(code::ErrorType::ERROR, "VAR declaration must appear before CONST declaration!");
        }
    } else if (result->is(lex::Token::Type::KEYWORD, Keyword::CONST) && constantDeclarations) {
        return result->makeError(code::ErrorType::ERROR, "Duplicate CONST declaration!")
            .attachCause(constantDeclarations->reference().makeError(code::ErrorType::NOTE, "Original declaration here"));
    }

    Result<CompoundStatement> compoundStatement = parseCompoundStatement();
    if (!compoundStatement) {
        return compoundStatement.error();
    }

    result = lexer->consume_next();
    if (!result) {
        return result.error();
    }

    if (!result->is(lex::Token::Type::SEPARATOR, Separator::END_OF_PROGRAM)) {
        return result->makeError(code::ErrorType::ERROR, "Expected `.` terminator!");
    }

    if (!lexer->endOfStream()) {
        return lexer->cur_position()
            .codeReference()
            .makeError(code::ErrorType::ERROR, "unexpected character after end of program terminator!");
    }

    return FunctionDefinition{ parameterDeclarations, variableDeclarations, constantDeclarations, compoundStatement.release(), GenericTerminal{ result->reference() } };
}

Result<ParameterDeclarations> Parser::parseParameterDeclarations() {
    Result<GenericTerminal> paramKeyword = parseGenericTerminal(lex::Token::Type::KEYWORD, Keyword::PARAM, "Expected `PARAM` keyword!");
    if (!paramKeyword) {
        return paramKeyword.error();
    }

    Result<DeclaratorList> declaratorList = parseDeclaratorList();
    if (!declaratorList) {
        return declaratorList.error();
    }

    Result<GenericTerminal> semicolon = parseGenericTerminal(lex::Token::Type::SEPARATOR, Separator::SEMICOLON, "Expected `;` to terminate PARAM declarations!");
    if (!semicolon) {
        return semicolon.error();
    }

    return ParameterDeclarations{ paramKeyword.release(), declaratorList.release(), semicolon.release() };
}

Result<VariableDeclarations> Parser::parseVariableDeclarations() {
    Result<GenericTerminal> varKeyword = parseGenericTerminal(lex::Token::Type::KEYWORD, Keyword::VAR, "Expected `VAR` keyword!");
    if (!varKeyword) {
        return varKeyword.error();
    }

    Result<DeclaratorList> declaratorList = parseDeclaratorList();
    if (!declaratorList) {
        return declaratorList.error();
    }

    Result<GenericTerminal> semicolon = parseGenericTerminal(lex::Token::Type::SEPARATOR, Separator::SEMICOLON, "Expected `;` to terminate VAR declarations!");
    if (!semicolon) {
        return semicolon.error();
    }

    return VariableDeclarations{ varKeyword.release(), declaratorList.release(), semicolon.release() };
}

Result<ConstantDeclarations> Parser::parseConstantDeclarations() {
    Result<GenericTerminal> constKeyword = parseGenericTerminal(lex::Token::Type::KEYWORD, Keyword::CONST, "Expected `CONST` keyword!");
    if (!constKeyword) {
        return constKeyword.error();
    }

    Result<InitDeclaratorList> initDeclaratorList = parseInitDeclaratorList();
    if (!initDeclaratorList) {
        return initDeclaratorList.error();
    }

    Result<GenericTerminal> semicolon = parseGenericTerminal(lex::Token::Type::SEPARATOR, Separator::SEMICOLON, "Expected `;` to terminate CONST declarations!");
    if (!semicolon) {
        return semicolon.error();
    }

    return ConstantDeclarations{ constKeyword.release(), initDeclaratorList.release(), semicolon.release() };
}

Result<DeclaratorList> Parser::parseDeclaratorList() {
    Result<Identifier> identifier = parseIdentifier();
    if (!identifier) {
        return identifier.error();
    }

    DeclaratorList declaratorList{ identifier.release() };

    Result<lex::Token> result;
    while (true) {
        result = lexer->peek_next();
        if (!result) {
            return result.error();
        }

        if (!result->is(lex::Token::Type::SEPARATOR, Separator::COMMA)) {
            break;
        }

        // mark token as consumed!
        lexer->consume(*result);

        Result<Identifier> additionalIdentifier = parseIdentifier();
        if (!additionalIdentifier) {
            return additionalIdentifier.error();
        }

        declaratorList.appendIdentifier(GenericTerminal{ result->reference() }, additionalIdentifier.release());
    }

    return declaratorList;
}

Result<InitDeclaratorList> Parser::parseInitDeclaratorList() {
    Result<InitDeclarator> initDeclarator = parseInitDeclarator();
    if (!initDeclarator) {
        return initDeclarator.error();
    }

    InitDeclaratorList declaratorList{ initDeclarator.release() };

    Result<lex::Token> result;
    while (true) {
        result = lexer->peek_next();
        if (!result) {
            return result.error();
        }

        if (!result->is(lex::Token::Type::SEPARATOR, Separator::COMMA)) {
            break;
        }

        // mark token as consumed!
        lexer->consume(*result);

        Result<InitDeclarator> additionalDeclarator = parseInitDeclarator();
        if (!additionalDeclarator) {
            return additionalDeclarator.error();
        }

        declaratorList.appendInitDeclarator(GenericTerminal{ result->reference() }, additionalDeclarator.release());
    }

    return declaratorList;
}

Result<InitDeclarator> Parser::parseInitDeclarator() {
    Result<Identifier> identifier = parseIdentifier();
    if (!identifier) {
        return identifier.error();
    }

    Result<GenericTerminal> init = parseGenericTerminal(lex::Token::Type::OPERATOR, Operator::INIT, "Expected `=` operator!");
    if (!init) {
        return init.error();
    }

    Result<Literal> literal = parseLiteral();
    if (!literal) {
        return literal.error();
    }

    return InitDeclarator{ identifier.release(), init.release(), literal.release() };
}

Result<CompoundStatement> Parser::parseCompoundStatement() {
    Result<GenericTerminal> begin = parseGenericTerminal(lex::Token::Type::KEYWORD, Keyword::BEGIN, "Expected `BEGIN` keyword!");
    if (!begin) {
        return begin.error();
    }

    Result<StatementList> statementList = parseStatementList();
    if (!statementList) {
        return statementList.error();
    }

    Result<GenericTerminal> end = parseGenericTerminal(lex::Token::Type::KEYWORD, Keyword::END, "Expected `END` keyword!");
    if (!end) {
        return end.error();
    }

    return CompoundStatement{ begin.release(), statementList.release(), end.release() };
}

Result<StatementList> Parser::parseStatementList() {
    Result<Statement> statement = parseStatement();
    if (!statement) {
        return statement.error();
    }

    StatementList statementList{ statement.release() };

    Result<lex::Token> result;
    while (true) {
        result = lexer->peek_next();
        if (!result) {
            return result.error();
        }

        if (result->is(lex::Token::Type::KEYWORD, "END")) {
            // StatementList only ever occurs before END. To provide better error messages, we do this check here.
            break;
        } else if (!result->is(lex::Token::Type::SEPARATOR, Separator::SEMICOLON)) {
            return result->makeError(code::ErrorType::ERROR, "Expected `;` to terminate statement!");
        }

        lexer->consume(*result);

        Result<Statement> additionalStatement = parseStatement();
        if (!additionalStatement) {
            return additionalStatement.error();
        }

        statementList.appendStatement(GenericTerminal{ result->reference() }, additionalStatement.release());
    }

    return statementList;
}

Result<Statement> Parser::parseStatement() {
    Result<lex::Token> result;

    result = lexer->peek_next();
    if (!result) {
        return result.error();
    }

    if (result->is(lex::Token::Type::KEYWORD, Keyword::RETURN)) {
        lexer->consume(*result);

        Result<AdditiveExpression> expression = parseAdditiveExpression();
        if (!expression) {
            return expression.error();
        }

        return Statement{ GenericTerminal(result->reference()), expression.release() };
    } else if (result->getType() == lex::Token::Type::IDENTIFIER) {
        Result<AssignmentExpression> expression = parseAssignmentExpression();
        if (!expression) {
            return expression.error();
        }

        return Statement{ expression.release() };
    } else {
        return result->makeError(code::ErrorType::ERROR, "Expected begin of statement. Assignment or RETURN expression!");
    }
}

Result<AssignmentExpression> Parser::parseAssignmentExpression() {
    Result<Identifier> identifier = parseIdentifier();
    if (!identifier) {
        return identifier.error();
    }

    Result<GenericTerminal> op = parseGenericTerminal(lex::Token::Type::OPERATOR, Operator::ASSIGNMENT, "Expected `:=` operator!");
    if (!op) {
        return op.error();
    }

    Result<AdditiveExpression> additiveExpression = parseAdditiveExpression();
    if (!additiveExpression) {
        return additiveExpression.error();
    }

    return AssignmentExpression{ identifier.release(), op.release(), additiveExpression.release() };
}

Result<AdditiveExpression> Parser::parseAdditiveExpression() {
    Result<MultiplicativeExpression> multiplicativeExpression = parseMultiplicativeExpression();
    if (!multiplicativeExpression) {
        return multiplicativeExpression.error();
    }

    if (lexer->endOfStream()) {
        // edge case when not parsing whole programs!
        return AdditiveExpression{ multiplicativeExpression.release() };
    }

    Result<lex::Token> result = lexer->peek_next();
    if (!result) {
        return result.error();
    }

    if (result->is(lex::Token::Type::OPERATOR, Operator::PLUS) || result->is(lex::Token::Type::OPERATOR, Operator::MINUS)) {
        lexer->consume(*result);

        Result<AdditiveExpression> additiveExpression = parseAdditiveExpression();
        if (!additiveExpression) {
            return additiveExpression.error();
        }

        return AdditiveExpression{ multiplicativeExpression.release(), GenericTerminal{ result->reference() }, additiveExpression.release()};
    } else {
        return AdditiveExpression{ multiplicativeExpression.release() };
    }
}

Result<MultiplicativeExpression> Parser::parseMultiplicativeExpression() {
    Result<UnaryExpression> unaryExpression = parseUnaryExpression();
    if (!unaryExpression) {
        return unaryExpression.error();
    }

    if (lexer->endOfStream()) {
        // edge case when not parsing whole programs!
        return MultiplicativeExpression{ unaryExpression.release() };
    }

    Result<lex::Token> result = lexer->peek_next();
    if (!result) {
        return result.error();
    }

    if (result->is(lex::Token::Type::OPERATOR, Operator::MULTIPLICATION) || result->is(lex::Token::Type::OPERATOR, Operator::DIVISION)) {
        lexer->consume(*result);

        Result<MultiplicativeExpression> multiplicativeExpression = parseMultiplicativeExpression();
        if (!multiplicativeExpression) {
            return multiplicativeExpression.error();
        }

        return MultiplicativeExpression{ unaryExpression.release(), GenericTerminal{result->reference()}, multiplicativeExpression.release() };
    } else {
        return MultiplicativeExpression{ unaryExpression.release() };
    }
}
Result<UnaryExpression> Parser::parseUnaryExpression() {
    Result<lex::Token> result;

    result = lexer->peek_next();
    if (!result) {
        return result.error();
    }

    std::optional<GenericTerminal> unaryOperator;

    if (result->is(lex::Token::Type::OPERATOR, Operator::PLUS) || result->is(lex::Token::Type::OPERATOR, Operator::MINUS)) {
        lexer->consume(*result);
        unaryOperator = GenericTerminal{ result->reference() };
    } else if (result->getType() == lex::Token::Type::OPERATOR) {
        return result->makeError(code::ErrorType::ERROR, "Unexpected unary operator!");
    }

    Result<PrimaryExpression> expression = parsePrimaryExpression();
    if (!expression) {
        return expression.error();
    }

    if (unaryOperator) {
        return UnaryExpression{ *unaryOperator, expression.release() };
    } else {
        return UnaryExpression{ expression.release() };
    }
}

Result<PrimaryExpression> Parser::parsePrimaryExpression() {
    Result<lex::Token> result;

    result = lexer->peek_next();
    if (!result) {
        return result.error();
    }

    if (result->getType() == lex::Token::Type::IDENTIFIER) {
        Result<Identifier> identifier = parseIdentifier();
        if (!identifier) {
            return identifier.error();
        }

        return PrimaryExpression{ identifier.release() };
    } else if (result->getType() == lex::Token::Type::LITERAL) {
        Result<Literal> literal = parseLiteral();
        if (!literal) {
            return literal.error();
        }

        return PrimaryExpression{ literal.release() };
    } else if (result->is(lex::Token::Type::PARENTHESIS, Parenthesis::ROUND_OPEN)) {
        Result<GenericTerminal> open = parseGenericTerminal(lex::Token::Type::PARENTHESIS, Parenthesis::ROUND_OPEN, "Expected `(` parenthesis!");
        if (!open) {
            return open.error();
        }

        Result<AdditiveExpression> expression = parseAdditiveExpression();
        if (!expression) {
            return expression.error();
        }


        Result<GenericTerminal> close = parseGenericTerminal(lex::Token::Type::PARENTHESIS, Parenthesis::ROUND_CLOSE, "Expected matching `)` parenthesis!");
        if (!close) {
            return close.error()
                    .attachCause(open->reference().makeError(code::ErrorType::NOTE, "opening bracket here"));
        }

        return PrimaryExpression{ open.release(), expression.release(), close.release() };
    } else {
        return result->makeError(code::ErrorType::ERROR, "Expected a primary expression!");
    }
}

Result<Identifier> Parser::parseIdentifier() {
    Result<lex::Token> result;

    result = lexer->consume_next();
    if (!result) {
        return result.error();
    }

    if (result->getType() != lex::Token::Type::IDENTIFIER) {
        return result->makeError(code::ErrorType::ERROR, "Expected an identifier!");
    }

    return Identifier{ result->reference() };
}

Result<Literal> Parser::parseLiteral() {
    Result<lex::Token> result;

    result = lexer->consume_next();
    if (!result) {
        return result.error();
    }

    if (result->getType() != lex::Token::Type::LITERAL) {
        return result->makeError(code::ErrorType::ERROR, "Expected literal!");
    }

    std::string_view literal = *result->reference();
    long long value;

    auto conversion = std::from_chars(literal.data(), literal.data() + literal.size(), value);

    if (conversion.ec != std::errc{}) {
        if (conversion.ec == std::errc::result_out_of_range) {
            return result->makeError(code::ErrorType::ERROR, "Integer literal is out of range. Expected singed 64-bit!");
        }
        return result->makeError(code::ErrorType::ERROR, "Encountered unexpected error parsing integer literal!");
    }

    if (conversion.ptr != literal.data() + literal.size()) {
        return result->makeError(code::ErrorType::ERROR, "Integer literal wasn't fully parsed!");
    }

    return Literal{ result->reference(), value };
}

Result<GenericTerminal> Parser::parseGenericTerminal(
    lex::Token::Type expected_type,
    std::string_view expected_content, // NOLINT(bugprone-easily-swappable-parameters)
    std::string_view potential_error_message) {
    Result<lex::Token> result;

    result = lexer->consume_next();
    if (!result) {
        return result.error();
    }

    if (!result->is(expected_type, expected_content)) {
        return result->makeError(code::ErrorType::ERROR, potential_error_message);
    }

    return GenericTerminal{ result->reference() };
}
//---------------------------------------------------------------------------
} // namespace pljit::parse
//---------------------------------------------------------------------------