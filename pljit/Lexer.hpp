//
// Created by Andreas Bauer on 13.07.22.
//

#ifndef PLJIT_LEXER_HPP
#define PLJIT_LEXER_HPP

#include "SourceCodeManagement.hpp"
#include <optional>

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
        SEPARATOR,
        OPERATOR, // TODO plus, minus, etc
        INTEGER_LITERAL,
        PARENTHESIS,
    };

    enum class ExtendResult { // TODO docs
        /// The Token was extended with the given character.
        EXTENDED,
        /// The Token was not extended. The character was illegal.
        ERRONEOUS_CHARACTER,
        /// The Token was not extended. The given character doesn't match the given {@class TokenType}.
        /// The Token should be considered complete and the character should be considered part of a new Token.
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
    static bool isEndOfProgram(char character);

    static bool isKeyword(std::string_view view);

    static TokenType typeOfCharacter(char character);

    Token(); // TODO creates an empty token!

    bool isEmpty() const;

    TokenType getType() const;

    SourceCodeReference reference() const;

    /**
     * Extends this Token with another character.
     * @param character An iterator pointing to a character which should be added to the Token.
     * @return Returns the result of the extend operation.
     */
    ExtendResult extend(SourceCodeManagement::iterator character);

    void finalize();

    // TODO OperatorType operatorType() const; ???
    // TODO parenthesis type getter?
};
//---------------------------------------------------------------------------
class Lexer {
    const SourceCodeManagement& management; // TODO reference makes non copyable or movable assignment!?
    SourceCodeManagement::iterator current_position;

    public:
    class LexerResult { // TODO consider renaming? (Double "Lexer")
        std::optional<SourceCodeError> source_error;
        Token lexed_token;

        public:
        LexerResult();
        explicit LexerResult(Token token);
        explicit LexerResult(SourceCodeError error);

        const Token& token() const;
        SourceCodeError error() const;

        bool success() const;
        bool failure() const;
    };

    explicit Lexer(const SourceCodeManagement& management);
    // TODO remove copy?

    LexerResult next();
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_LEXER_HPP
