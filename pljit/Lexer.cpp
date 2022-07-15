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
        // TODO remove: std::cout << "Reading : '" << *current_position << "'" << std::endl;

        if (Token::isWhitespace(*current_position)) {
            if (token.isEmpty()) {
                continue; // we skip whitespaces till we find something!
            }

            ++current_position; // consume the whitespace and return the token!

            token.finalize();
            return true;
        } else if (Token::isSeparator(*current_position)) {
            // TODO should a separator be a token (research this in the paper)
            if (token.isEmpty()) {
                // TODO this is only a problem if a separator is before the first token?
                // current_position.codeReference()
                //    .print_error(SourceCodeManagement::ErrorType::ERROR, "unexpected separator");
                continue;
            }

            ++current_position; // we consume the separator and return the token!

            token.finalize();
            return true;
        }
        // TODO check for the "." END of program terminator! (check if there are characters after that!)

        auto extend_result = token.extend(current_position);
        switch (extend_result) {
            case Token::ExtendResult::EXTENDED:
                continue;
            case Token::ExtendResult::ERRONEOUS_CHARACTER:
                // TODO we want to return a proper error (so this thing can be used as a library)
                //   => ability to derive line numbers and column when using it as a library!
                current_position.codeReference()
                    .print_error(SourceCodeManagement::ErrorType::ERROR, "unexpected character");
                return false;
            case Token::ExtendResult::NON_MATCHING_TYPE:
                assert(current_position != management.begin()); // can't be by definition

                // we received a character which doesn't match the current token type
                // we want to parse that character in the next query again (so we don't increment)!

                token.finalize();
                return true;
        }
    }

    assert(token.isEmpty());
    return true; // return an empty token to signal end of stream!
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

bool pljit::Token::isKeyword(std::string_view view) {
    return view == "PARAM" || view == "VAR" || view == "CONST" || view == "BEGIN" || view == "END" || view == "RETURN";
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

pljit::SourceCodeReference pljit::Token::reference() const {
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
void pljit::Token::finalize() {
    if (type == TokenType::IDENTIFIER && isKeyword(source_code.content())) {
        type = TokenType::KEYWORD;
    }
}
//---------------------------------------------------------------------------
