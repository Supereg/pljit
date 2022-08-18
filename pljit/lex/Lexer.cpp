//
// Created by Andreas Bauer on 13.07.22.
//

#include "./Lexer.hpp"
#include "../lang.hpp"
#include <cassert>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
using namespace code;
//---------------------------------------------------------------------------
namespace lex {
//---------------------------------------------------------------------------
Token::Token() : type(Type::EMPTY), source_code() {}

bool Token::isWhitespace(char character) {
    return character == ' ' || character == '\n' || character == '\t';
}

bool Token::isSeparator(char character) {
    return character == ',' || character == ';' || character == '.';
}

bool Token::isAlphanumeric(char character) {
    return std::isalpha(character);
}

bool Token::isIntegerLiteral(char character) {
    return std::isdigit(character);
}

bool Token::isParenthesis(char character) {
    return character == '(' || character == ')';
}

bool Token::isOperator(char character) {
    return character == '+' || character == '-' || character == '*' || character == '/' || character == '=' || character == ':';
}

bool Token::isKeyword(std::string_view view) {
    return view == Keyword::PARAM || view == Keyword::VAR || view == Keyword::CONST || view == Keyword::BEGIN || view == Keyword::END || view == Keyword::RETURN;
}

Token::Type Token::typeOfCharacter(char character) {
    if (isAlphanumeric(character)) {
        // Type::KEYWORD is handled in `Token::finalize`.
        return Type::IDENTIFIER;
    } else if (isOperator(character)) {
        return Type::OPERATOR;
    } else if (isSeparator(character)) {
        return Type::SEPARATOR;
    } else if (isIntegerLiteral(character)) {
        return Type::LITERAL;
    } else if (isParenthesis(character)) {
        return Type::PARENTHESIS;
    } else {
        return Type::EMPTY;
    }
}

bool Token::isEmpty() const {
    return type == Type::EMPTY;
}

Token::Type Token::getType() const {
    return type;
}

SourceCodeReference Token::reference() const {
    assert(type != Type::EMPTY && "Can't access the source code reference of an empty token!");
    return source_code;
}

SourceCodeError Token::makeError(ErrorType errorType, std::string_view message) const {
    return reference().makeError(errorType, message);
}

std::string_view Token::content() const {
    return *reference();
}

bool Token::is(Token::Type token_type, std::string_view content) const {
    return type == token_type && *source_code == content;
}

Token::ExtendResult Token::extend(SourceIterator character) {
    Type next_type = typeOfCharacter(*character);
    if (next_type == Type::EMPTY) {
        // encountered unknown character type!
        return ExtendResult::ERRONEOUS_CHARACTER;
    }

    if (type == Type::EMPTY) {
        type = typeOfCharacter(*character);
        source_code = character.codeReference();
        return ExtendResult::EXTENDED;
    }

    if (next_type != type) {
        return ExtendResult::END_OF_TOKEN;
    }

    // if we reach here, at least one character was added to the token.
    // we need to ensure that we signal END_OF_TOKEN of single character token types
    if (type == Type::SEPARATOR || type == Type::PARENTHESIS) {
        return ExtendResult::END_OF_TOKEN;
    } else if (type == Type::OPERATOR) {
        // operator is special, as we have the `:=` operator of length 2.
        if (source_code->size() >= 2) {
            return ExtendResult::END_OF_TOKEN;
        }

        if (!(*source_code == ":" && *character == '=')) {
            // we only allow to extend if current operator character is `:` and we want to extend with `=` to form `:=`.
            // While we could build a more abstract solution, capable of handling multiple multi character operators and
            // not dealing with magic constants, we don't (for now). We only need to handle a single character that is special in this way.
            return ExtendResult::END_OF_TOKEN;
        }
    }

    source_code.extend(1);
    return ExtendResult::EXTENDED;
}

void Token::finalize() {
    if (type == Type::IDENTIFIER && isKeyword(*source_code)) {
        type = Type::KEYWORD;
    }
}

bool Token::operator==(const Token& rhs) const {
    return type == rhs.type &&
        source_code == rhs.source_code;
}
//---------------------------------------------------------------------------
Lexer::Lexer(const SourceCodeManagement& management)
    : management(&management), current_position(management.begin()), returnedWithError(false) {}

bool Lexer::endOfStream() {
    // might be the case that there are more than one whitespaces after the `.` terminator.
    // We want to report endOfStream as true, even when there are only whitespaces remaining.
    while (current_position != management->end() && Token::isWhitespace(*current_position)) {
        ++current_position;
    }

    return current_position == management->end();
}

SourceIterator Lexer::cur_position() const {
    return current_position;
}

Result<Token> Lexer::peek_next() {
    if (next_result.has_value()) {
        return *next_result;
    }

    next_result = next();
    // we mark `returnedWithError` only when calling `consume_next` or `consume`.
    return *next_result;
}

Result<Token> Lexer::consume_next() {
    if (next_result.has_value()) {
        Result<Token> result = *next_result;
        next_result.reset();
        return result;
    }

    Result<Token> result = next();
    if (result.failure()) {
        returnedWithError = true;
    }
    return result;
}

void Lexer::consume(const Token& result) {
    assert(next_result.has_value() && "Tried to consume Token when nothing was peeked!");
    assert(next_result->success() && "Tried to consume Token in erroneous state!");
    assert(next_result->value() == result && "Tried consuming unexpected Token!");

    if (next_result->failure()) {
        returnedWithError = true;
    }

    next_result.reset();
}

Result<Token> Lexer::next() {
    assert(!returnedWithError && "Can't continue lexicographical analysis after encountering an error!");

    Token token; // creates an `EMPTY` token.

    for (; current_position != management->end(); ++current_position) {
        if (Token::isWhitespace(*current_position)) {
            if (token.isEmpty()) {
                continue; // we skip whitespaces till we find something!
            }

            ++current_position; // consume the whitespace and return the token!

            token.finalize();
            return token;
        }

        Token::ExtendResult extend_result = token.extend(current_position);
        switch (extend_result) {
            case Token::ExtendResult::EXTENDED:
                continue;
            case Token::ExtendResult::ERRONEOUS_CHARACTER: {
                return current_position
                    .codeReference()
                    .makeError(ErrorType::ERROR, "unexpected character!");
            }
            case Token::ExtendResult::END_OF_TOKEN:
                assert(current_position != management->begin()); // can't be by definition, at least one character was processed.

                // we received a character which doesn't match the current token type
                // we want to parse_program that character in the next query again (so we don't increment)!

                token.finalize();
                return token;
        }
    }

    // we reach here either if we break out of the loop or if we reach the end of the source code.

    if (token.isEmpty()) {
        return current_position
            .codeReference()
            .makeError(ErrorType::ERROR, "unexpected end of stream!");
    }

    token.finalize();
    return token;
}
//---------------------------------------------------------------------------
} // namespace lex
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------
