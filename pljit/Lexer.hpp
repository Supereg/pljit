//
// Created by Andreas Bauer on 13.07.22.
//

#ifndef PLJIT_LEXER_HPP
#define PLJIT_LEXER_HPP

#include "SourceCodeManagement.hpp"
#include <optional>
#include <cassert>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
class Token {
    public:
    /// Defines the type of the token.
    enum class TokenType {
        /// A token which is empty (doesn't contain any characters).
        /// A `Token` with this type signals end of the program.
        EMPTY,
        /// A keyword. One of `PARAM`, `VAR`, `CONST`, `BEGIN`, `END` or `RETURN`.
        KEYWORD,
        /// Some alphanumeric (lowercase or uppercase) identifier (which is not a keyword).
        IDENTIFIER,
        /// A separator. One of `,` or `;`.
        SEPARATOR,
        /// A operator consisting of a single character. One of `+`, `-`, `*`, `/`, `=` or `:=`.
        OPERATOR,
        /// A integer literal. Any combination of digits.
        INTEGER_LITERAL,
        /// A single parenthesis. One of `(` or `)`.
        PARENTHESIS,
    };

    /// Describes the result of the token `extend` operation.
    enum class ExtendResult {
        /// The Token was extended with the given character.
        EXTENDED,
        /// The Token was not extended. The character was illegal.
        ERRONEOUS_CHARACTER,
        /// The Token was not extended.
        /// The given character either doesn't match the given {@class TokenType} or the Token is already considered complete
        /// (e.g. for single character operators).
        /// The Token should be considered complete and the character should be considered part of a new Token.
        END_OF_TOKEN,
    };

    private:
    TokenType type;
    SourceCodeReference source_code;

    public:

    // TODO docs!

    static bool isWhitespace(char character);
    static bool isSeparator(char character);
    static bool isAlphanumeric(char character);
    static bool isIntegerLiteral(char character);
    static bool isParenthesis(char character);
    static bool isOperator(char character);
    static bool isEndOfProgram(char character);

    static bool isKeyword(std::string_view view);

    static TokenType typeOfCharacter(char character);

    /// Creates an empty Token. `TokenType` is set to `EMPTY` and the source code is not accessible.
    Token();

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
};
//---------------------------------------------------------------------------
template <typename T> requires std::constructible_from<T> && std::move_constructible<T>
class Result { // TODO placement!
    std::optional<pljit::SourceCodeError> source_error;
    T result_content;

    // TODO out of line definition!
    public:
    Result() : source_error(), result_content() {}
    Result(T result) : source_error(), result_content(std::move(result)) {}
    Result(SourceCodeError error) : source_error(error), result_content() {}

    const T& value() const {
        assert(!source_error.has_value() && "Result: result not present. SourceCode error occurred!");
        return result_content;
    }

    SourceCodeError error() const {
        assert(source_error.has_value() && "LexerResult: tried accessing non-existent error!");
        return *source_error;
    }

    // TODO prepend with "is" keyword! Both!
    bool success() const {
        return !source_error.has_value();
    }

    bool failure() const {
        return source_error.has_value();
    }
};
//---------------------------------------------------------------------------
class Lexer {
    const SourceCodeManagement* management;
    SourceCodeManagement::iterator current_position;

    public:
    explicit Lexer(const SourceCodeManagement& management);

    Result<Token> peek_next();
    Result<Token> consume_next();
    // Result<Token> next(); // TODO peek_next vs consume_next!
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_LEXER_HPP
