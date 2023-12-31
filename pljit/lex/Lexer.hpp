//
// Created by Andreas Bauer on 13.07.22.
//

#ifndef PLJIT_LEXER_HPP
#define PLJIT_LEXER_HPP

#include "../code/SourceCodeManagement.hpp"
#include "../util/Result.hpp"
#include <optional>

//---------------------------------------------------------------------------
namespace pljit::lex {
//---------------------------------------------------------------------------
class Lexer;
//---------------------------------------------------------------------------
class Token {
    friend class Lexer; // access to construction methods.

    public:
    /// Defines the type of the token.
    enum class Type {
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
        /// The given character either doesn't match the given {@class Type} or the Token is already considered complete
        /// (e.g. for single character operators).
        /// The Token should be considered complete and the character should be considered part of a new Token.
        END_OF_TOKEN,
        /// The character given to extend the Token was invalid.
        /// Therefore, the given token was not extended and is considered incomplete.
        INCOMPLETE_TOKEN,
    };

    private:
    Type type;
    code::SourceCodeReference source_code;

    public:
    /// Creates an empty Token. `Type` is set to `EMPTY` and the source code is not accessible.
    Token();

    /**
     * Check if the {@see Type} of this `Token` is of type `Type::EMPTY`.
     * @return Returns `true` if the `Token` is an empty token.
     */
    bool isEmpty() const;
    /**
     * Check if the Token is considered incomplete.
     * This is the case for e.g. multi character operators.
     * @return Returns `true` if the `Token` is considered incomplete.
     */
    bool isIncomplete() const;
    /**
     * @return Returns the type of the Token!
     */
    Type getType() const;

    /**
     * Get access to the source code of the `Token`.
     * @return Returns the {@class SourceCodeReference} of the Token.
     */
    code::SourceCodeReference reference() const;

    /**
     * Shorthand way to create a {@class SourceCodeError} at the current {@class SourceCodeReference}.
     * @param errorType The {@class ErrorType}.
     * @param message The view to the error message.
     * @return The created {@class SourceCodeError} with the given type and message.
     */
    [[nodiscard]] code::SourceCodeError makeError(code::ErrorType errorType, std::string_view message) const;

    /**
     * Shorthand version to check the {@class Type} and Token content of the Token.
     * @param type The {@class Type}.
     * @param content The string representation of the Token content.
     * @return True if both of the passed parameter matches the Token.
     */
    bool is(Type token_type, std::string_view content) const;

    bool operator==(const Token& rhs) const;

    private:
    /**
     * Extends this Token with another character.
     * @param character An SourceIterator pointing to a character which should be added to the Token.
     * @return Returns the result of the extend operation.
     */
    ExtendResult extend(code::SourceIterator character);

    /**
     * Finalizes the Token. This must be called at the end of construction.
     * For example, it checks if a captured IDENTIFIER is a KEYWORD and updates the token type accordingly.
     */
    void finalize();

};
//---------------------------------------------------------------------------
class Lexer {
    const code::SourceCodeManagement* management;
    code::SourceIterator current_position;

    /// Temporary variable to store Result instances that were peeked but not yet consumed!
    std::optional<Result<Token>> next_result;
    bool returnedWithError;

    public:
    explicit Lexer(const code::SourceCodeManagement& management);

    bool endOfStream();

    code::SourceIterator cur_position() const;

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
    /**
     * Internal implementation to derive the next Token.
     */
    Result<Token> next();
};
//---------------------------------------------------------------------------
} // namespace pljit::lex
//---------------------------------------------------------------------------

#endif //PLJIT_LEXER_HPP
