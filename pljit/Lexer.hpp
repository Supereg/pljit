//
// Created by Andreas Bauer on 13.07.22.
//

#ifndef PLJIT_LEXER_HPP
#define PLJIT_LEXER_HPP

#include "SourceCodeManagement.hpp"
#include "Result.hpp"
#include <optional>
#include <cassert>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
struct Keyword {
    static constexpr std::string_view PARAM = "PARAM";
    static constexpr std::string_view VAR = "VAR";
    static constexpr std::string_view CONST = "CONST";
    static constexpr std::string_view BEGIN = "BEGIN";
    static constexpr std::string_view END = "END";
    static constexpr std::string_view RETURN = "RETURN";
};

struct Operator {
    static constexpr std::string_view PLUS = "+";
    static constexpr std::string_view MINUS = "-";
    static constexpr std::string_view MULTIPLICATION = "*";
    static constexpr std::string_view DIVISION = "/";
    static constexpr std::string_view INIT = "=";
    static constexpr std::string_view ASSIGNMENT = ":=";
};

struct Parenthesis {
    static constexpr std::string_view ROUND_OPEN = "(";
    static constexpr std::string_view ROUND_CLOSE = ")";
};

struct Separator {
    static constexpr std::string_view COMMA = ",";
    static constexpr std::string_view SEMICOLON = ";";
    static constexpr std::string_view END_OF_PROGRAM = ".";
};

struct Whitespace {
    static constexpr std::string_view SPACE = " ";
    static constexpr std::string_view TAB = "\t";
    static constexpr std::string_view NEW_LINE = "\n";
};
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
        /// Some alphanumeric (lowercase or uppercase) string (which is not a keyword).
        IDENTIFIER,
        /// A separator. One of `,`, `;` or `.`.
        SEPARATOR,
        /// A operator consisting of a single character. One of `+`, `-`, `*`, `/`, `=` or `:=`.
        OPERATOR,
        /// A integer literal. Any combination of digits.
        LITERAL,
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

    // TODO placement? (lang or src namespace?)
    static bool isWhitespace(char character);
    static bool isSeparator(char character);
    static bool isAlphanumeric(char character);
    static bool isIntegerLiteral(char character);
    static bool isParenthesis(char character);
    static bool isOperator(char character);

    static bool isKeyword(std::string_view view);

    static TokenType typeOfCharacter(char character);

    /// Creates an empty Token. `TokenType` is set to `EMPTY` and the source code is not accessible.
    Token();

    /**
     * Check if the {@see TokenType} of this `Token` is of type `TokenType::EMPTY`.
     * @return Returns `true` if the `Token` is an empty token.
     */
    bool isEmpty() const;

    TokenType getType() const;

    /**
     * Get access to the source code of the `Token`.
     * @return Returns the {@class SourceCodeReference} of the Token.
     */
    SourceCodeReference reference() const;

    /**
     * Shorthand way to create a {@class SourceCodeError} at the current {@class SourceCodeReference}.
     * @param errorType The {@class SourceCodeManagement::ErrorType}.
     * @param message The view to the error message.
     * @return The created {@class SourceCodeError} with the given type and message.
     */
    [[nodiscard]] SourceCodeError makeError(SourceCodeManagement::ErrorType errorType, std::string_view message) const;

    /**
     * Access to the Token's source code content.
     * @return `string_view` of the source code content.
     */
    std::string_view content() const;

    /**
     * Shorthand version to check the {@class TokenType} and Token content of the Token.
     * @param type The {@class TokenType}.
     * @param content The string representation of the Token content.
     * @return True if both of the passed parameter matches the Token.
     */
    bool is(TokenType token_type, std::string_view content) const;

    /**
     * Extends this Token with another character.
     * @param character An iterator pointing to a character which should be added to the Token.
     * @return Returns the result of the extend operation.
     */
    ExtendResult extend(SourceCodeManagement::iterator character);

    void finalize();

    bool operator==(const Token& rhs) const;
};
//---------------------------------------------------------------------------
class Lexer {
    const SourceCodeManagement* management;
    SourceCodeManagement::iterator current_position;

    /// Temporary variable to store Result instances that were peeked but not yet consumed!
    std::optional<Result<Token>> next_result;
    bool returnedWithError;

    public:
    explicit Lexer(const SourceCodeManagement& management);

    bool endOfStream();

    SourceCodeManagement::iterator cur_position() const;

    /**
     * Peeks the next `Token`.
     * Peeking will parse_program the next `Token` without advancing the reader index.
     * Repeatedly calling `peek_next()` will always result in the same `Token`.
     * See {@link consume_next()}.
     * @return The next `Token` without consuming it.
     */
    Result<Token> peek_next();
    /**
     * Consumes the next `Token`.
     * Consuming means, parsing the next `Token` and advancing the reader index
     * to the token coming after the returned result.
     * @return The next `Token` by consuming it.
     */
    Result<Token> consume_next();
    /**
     * Consume can be used to consume the token read previously with {@link peek_next()}.
     * @param result The `
     */
    void consume(const Token& result);

    private:
    Result<Token> next();
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_LEXER_HPP
