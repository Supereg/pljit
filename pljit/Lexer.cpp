//
// Created by Andreas Bauer on 13.07.22.
//

#include "Lexer.hpp"
#include <cassert>

// TODO AST: no symbols for separators. Still source code references for error printing(?)!
// TODO -fno-rtti (no run time type information)
// TODO optimizations (e.g. with division by zero) -> ensure that the error is still there

// TODO using namespace pljit?

//---------------------------------------------------------------------------
pljit::Lexer::Lexer(const SourceCodeManagement& management) : management(&management), current_position(management.begin()) {}

pljit::Result<pljit::Token> pljit::Lexer::peek_next() {
    auto current_iterator = current_position;
    auto result = consume_next();
    current_position = current_iterator;

    return result;
}

pljit::Result<pljit::Token> pljit::Lexer::consume_next() {
    Token token;

    for(; current_position != management->end(); ++current_position) {
        if (Token::isWhitespace(*current_position)) {
            if (token.isEmpty()) {
                continue; // we skip whitespaces till we find something!
            }

            ++current_position; // consume the whitespace and return the token!

            token.finalize();
            return token;
        } else if (Token::isEndOfProgram(*current_position)) {
            if (!token.isEmpty()) {
                break;
            }

            // consume the character and check if we this is really the end of the source code!
            ++current_position;

            while (current_position != management->end()) {
                // we allow whitespaces after the dot, but nothing else!
                if (!Token::isWhitespace(*current_position)) {
                    SourceCodeError error{
                        SourceCodeManagement::ErrorType::ERROR,
                        "unexpected character after end of program terminator!",
                        current_position.codeReference()
                    };
                    return error;
                }

                ++current_position;
            }

            break; // break the loop; jumps to the logic which handles "end of stream" below
        }

        Token::ExtendResult extend_result = token.extend(current_position);
        switch (extend_result) {
            case Token::ExtendResult::EXTENDED:
                continue;
            case Token::ExtendResult::ERRONEOUS_CHARACTER: {
                SourceCodeError error{
                    SourceCodeManagement::ErrorType::ERROR,
                    "unexpected character",
                    current_position.codeReference()
                };
                return error;
            }
            case Token::ExtendResult::END_OF_TOKEN:
                assert(current_position != management->begin()); // can't be by definition, at least one character was processed.

                // we received a character which doesn't match the current token type
                // we want to parse that character in the next query again (so we don't increment)!

                token.finalize();
                return token;
        }
    }

    // this is called once we reach the end of the source code.

    if (!token.isEmpty()) {
        token.finalize();
    }

    return token; // returning an empty token signals end of stream!
}
//---------------------------------------------------------------------------
pljit::Token::Token() : type(TokenType::EMPTY), source_code() {}

bool pljit::Token::isWhitespace(char character) {
    return character == ' ' || character == '\n' || character == '\t';
}

bool pljit::Token::isSeparator(char character) {
    return character == ',' || character == ';';
}

bool pljit::Token::isAlphanumeric(char character) {
    return std::isalpha(character);
}

bool pljit::Token::isIntegerLiteral(char character) {
    return std::isdigit(character);
}

bool pljit::Token::isParenthesis(char character) {
    return character == '(' || character == ')';
}

bool pljit::Token::isOperator(char character) {
    return character == '+' || character == '-' || character == '*' || character == '/' || character == '=' || character == ':';
}

bool pljit::Token::isEndOfProgram(char character) {
    return character == '.';
}

bool pljit::Token::isKeyword(std::string_view view) {
    return view == "PARAM" || view == "VAR" || view == "CONST" || view == "BEGIN" || view == "END" || view == "RETURN";
}

pljit::Token::TokenType pljit::Token::typeOfCharacter(char character) {
    if (isAlphanumeric(character)) {
        // TokenType::KEYWORD is handled in `Token::finalize`.
        return TokenType::IDENTIFIER;
    } else if (isOperator(character)) {
        return TokenType::OPERATOR;
    } else if (isSeparator(character)) {
        return TokenType::SEPARATOR;
    } else if (isIntegerLiteral(character)) {
        return TokenType::INTEGER_LITERAL;
    } else if (isParenthesis(character)) {
        return TokenType::PARENTHESIS;
    } else {
        return TokenType::EMPTY;
    }
}

bool pljit::Token::isEmpty() const {
    return type == TokenType::EMPTY;
}

pljit::Token::TokenType pljit::Token::getType() const {
    return type;
}

pljit::SourceCodeReference pljit::Token::reference() const {
    assert(type != TokenType::EMPTY && "Can't access the source code reference of an empty token!");
    return source_code;
}

pljit::Token::ExtendResult pljit::Token::extend(pljit::SourceCodeManagement::iterator character) {
    TokenType next_type = typeOfCharacter(*character);
    if (next_type == TokenType::EMPTY) {
        // encountered unknown character type!
        return ExtendResult::ERRONEOUS_CHARACTER;
    }

    if (type == TokenType::EMPTY) {
        type = typeOfCharacter(*character);
        source_code = character.codeReference();
        return ExtendResult::EXTENDED;
    }

    if (next_type != type) {
        return ExtendResult::END_OF_TOKEN;
    }

    // if we reach here, at least one character was added to the token.
    // we need to ensure that we signal END_OF_TOKEN of single character token types
    if (type == TokenType::SEPARATOR || type == TokenType::PARENTHESIS) {
        return ExtendResult::END_OF_TOKEN;
    } else if (type == TokenType::OPERATOR) {
        // operator is special, as we have the `:=` operator of length 2.
        if (source_code.content().size() >= 2) {
            return ExtendResult::END_OF_TOKEN;
        }

        if (!(source_code.content() == ":" && *character == '=')) {
            // we only allow to extend if current operator character is `:` and we want to extend with `=` to form `:=`.
            // While we could build a more abstract solution, capable of handling multiple multi character operators and
            // not dealing with magic constants, we don't (for now). We only need to handle a single character that is special in this way.
            return ExtendResult::END_OF_TOKEN;
        }
    }

    // assert(&(*character) == source_code.content().end() && "Tried to extend Token with more than 1 character difference");
    source_code.extend(1);
    return ExtendResult::EXTENDED;
}
void pljit::Token::finalize() {
    if (type == TokenType::IDENTIFIER && isKeyword(source_code.content())) {
        type = TokenType::KEYWORD;
    }
}
//---------------------------------------------------------------------------
