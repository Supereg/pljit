//
// Created by Andreas Bauer on 13.07.22.
//

#ifndef PLJIT_LEXER_HPP
#define PLJIT_LEXER_HPP

#include "SourceCodeManagement.hpp"

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
class Token {
    public:
    enum class TokenType { // TODO docs
        /// A token which is empty (doesn't contain any characters).
        EMPTY,
        KEYWORD,
        IDENTIFIER,
        OPERATOR, // TODO plus, minus, etc
        INTEGER_LITERAL,
        PARENTHESIS,
    };

    enum class ExtendResult { // TODO docs
        EXTENDED,
        ERRONEOUS_CHARACTER,
        NON_MATCHING_TYPE,
    };

    private:
    TokenType type;
    SourceCodeReference source_code;

    public:

    static bool isWhitespace(char character);
    static bool isSeparator(char character);
    static bool isAlphanumeric(char character);
    static bool isIntegerLiteral(char character);
    static bool isParenthesis(char character);
    static bool isOperator(char character);
    static bool isKeyword(std::string_view view);
    static TokenType typeOfCharacter(char character);

    Token(); // TODO creates an empty token!

    bool isEmpty() const;

    TokenType getType() const;

    SourceCodeReference reference() const;

    ExtendResult extend(SourceCodeManagement::iterator character);

    void finalize();

    // TODO OperatorType operatorType() const; ???
    // TODO parenthesis type getter?
};
//---------------------------------------------------------------------------
class Lexer {
    const SourceCodeManagement& management; // TODO reference makes non copyable or movable?
    SourceCodeManagement::iterator current_position;

    public:
    explicit Lexer(const SourceCodeManagement& management);

    bool next(Token& token);
    // TODO remove copy?
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_LEXER_HPP
