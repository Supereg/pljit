//
// Created by Andreas Bauer on 13.07.22.
//

#include "Lexer.hpp"
#include <cassert>
#include <iostream> // TODO remove

//---------------------------------------------------------------------------
pljit::Lexer::Lexer(const SourceCodeManagement& management) : management(management), current_position(management.begin()) {}

bool pljit::Lexer::next(Token& token) {
    // TODO whitespaces= whitespaces, tabs, newline characters! (they are spearators!)
    // TODO other separators: "," ";"
    // TODO terminator!
    // TODO operators: +, -, *, /, :=, =
    // TODO keywords: "PARAM", "VAR", "CONST", "BEGIN", "END", "RETURN"
    // TODO parentheses: "(", ")"

    assert(token.isEmpty() && "Destination token must be EMPTY!");

    for(; current_position != management.end(); ++current_position) {
        std::cout << "Reading : '" << *current_position << "'" << std::endl;

        if (Token::isWhitespace(*current_position)) {
            if (token.isEmpty()) {
                continue; // we skip whitespaces till we find something!
            }

            ++current_position; // consume the whitespace and return the token!
            return true;
        } else if (Token::isSeparator(*current_position)) {
            if (token.isEmpty()) {
                // TODO report error at code reference!
                return false;
            }

            ++current_position; // we consume the separator and return the token!
            return true;
        }
        // TODO check for the "." END of program terminator! (check if there are characters after that!)

        auto extend_result = token.extend(current_position);
        switch (extend_result) {
            case Token::ExtendResult::EXTENDED:
                continue;
            case Token::ExtendResult::ERRONEOUS_CHARACTER:
                // TODO report error at code reference!
                break;
            case Token::ExtendResult::NON_MATCHING_TYPE:
                assert(current_position != management.begin()); // can't be by definition

                // we received a character which doesn't match the current token type
                // we want to parse that character in the next query again (so we don't increment)!
                return true;
        }
    }

    return {}; // return an empty token to signal end of stream!
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
    // TODO how to handle ":=" ?
    return character == '+' || character == '-' || character == '*' || character == '/' || character == '=';
}

pljit::Token::TokenType pljit::Token::typeOfCharacter(char character) {
    if (isAlphanumeric(character)) {
        return TokenType::IDENTIFIER; // TODO change to keywords later
    } else if (isOperator(character)) {
        return TokenType::OPERATOR;
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

pljit::SourceCodeReference pljit::Token::content() const {
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
        return ExtendResult::NON_MATCHING_TYPE;
    }

    // assert(&(*character) == source_code.content().end() && "Tried to extend Token with more than 1 character difference");
    source_code.extend(1);
    return ExtendResult::EXTENDED;
}
//---------------------------------------------------------------------------
