//
// Created by Andreas Bauer on 18.08.22.
//

#ifndef PLJIT_SOURCECODE_HPP
#define PLJIT_SOURCECODE_HPP

#include <string_view>
#include <list>

//---------------------------------------------------------------------------
namespace pljit::code {
//---------------------------------------------------------------------------
class SourceCodeManagement;

class SourceCodeError;
//---------------------------------------------------------------------------
/**
 * Describes the `SourceCodeError` type.
 */
enum class ErrorType {
    /// An `note:` type error. Used to provide contextual information.
    NOTE,
    /// An `error:` type error. Used to describe fatal, erroneous conditions.
    ERROR,
};
//---------------------------------------------------------------------------
/**
 * Structure to store codePosition information of a source code element.
 * Numbers stored here start with 1, and can't be interpreted as indexes.
 */
class CodePosition {
    unsigned line_num;
    unsigned column_num;

    public:
    CodePosition(unsigned line_num, unsigned column_num);

    /// Returns the line number.
    unsigned line() const;
    /// Returns the column.
    unsigned column() const;

    bool operator==(const CodePosition& rhs) const;
};
//---------------------------------------------------------------------------
/**
 * Describes a reference to some substring within the source code managed within a `SourceCodeManagement` instance.
 */
class SourceCodeReference {
    friend class SourceIterator; // allows access to private constructors!
    friend class SourceCodeError; // allow access to `management` for error printing!

    /// The parent `SourceCodeManagement`
    const SourceCodeManagement* management;
    /// The `string_view` of the source code part this reference points to.
    std::string_view string_content;

    SourceCodeReference(const SourceCodeManagement* management, std::string_view string_content);

    public:
    /// Constructs an empty SourceCodeReference, associated with no `SourceCodeManagement`.
    SourceCodeReference();
    /**
     * Create a new SourceCodeReference from two SourceCodeReferences.
     * @param start The first character of this reference will be the first character of the new reference.
     * @param end The created SourceCodeReference will span till the last character in this SourceCodeReference.
     */
    SourceCodeReference(const SourceCodeReference& start, const SourceCodeReference& end);

    /// Access the underlying string_view.
    std::string_view operator*() const;
    /// Access the underlying string_view.
    const std::string_view* operator->() const;
    /// Get the position within the whole program of the first character of the reference.
    CodePosition position() const;

    /**
     * Extend the SourceCodeReference by a given amount of characters.
     * It must be ensured, that the bounds of the source code string aren't exceeded.
     * @param amount The amount of characters this SourceCodeReference shall be extended.
     */
    void extend(int amount);

    /**
     * Create an `SourceCodeError` for the given contents of this `SourceCodeReference`.
     * @param errorType - The `ErrorType`.
     * @param message - The error message.
     * @return Returns the constructed `SourceCodeError`.
     */
    SourceCodeError makeError(ErrorType errorType, std::string_view message) const;

    /// Compares two SourceCodeReferences. They are considered equal if they represent the same string within the same program text at the same position.
    bool operator==(const SourceCodeReference& rhs) const;
};
//---------------------------------------------------------------------------
class SourceCodeError {
    /// The `ErrorType` used for formatting.
    ErrorType errorType;
    /// The string_view pointing to the error message.
    std::string_view errorMessage;
    /// The source code reference that is the root cause of the error.
    SourceCodeReference codeReference;
    /// Position of where the error occurred.
    CodePosition codePosition;

    /// Optionally, causes or notes attached to the error.
    std::list<SourceCodeError> causes;
    public:

    SourceCodeError(ErrorType errorType, std::string_view errorMessage, SourceCodeReference sourceCodeReference);

    /**
     * @return The `ErrorType` of the source code error.
     */
    ErrorType type() const;
    /**
      * @return The error message of the source code error.
     */
    std::string_view message() const;
    /**
     * @return The `SourceCodeReference` pointing to the root cause.
     */
    const SourceCodeReference& reference() const;
    /**
     * @return The `CodePosition` where the error occurred.
     */
    CodePosition position() const;
    /**
     * @return The SourceCodeError causes attached to this error object.
     */
    const std::list<SourceCodeError>& attachedCauses() const;

    /**
     * Attach an cause to this `SourceCodeError`. Attached
     * @param error_cause - An `SourceCodeError` cause to be attached to this error instance.
     * @return Returns `this` instance of the SourceCodeError, used for chaning.
     */
    SourceCodeError& attachCause(SourceCodeError&& error_cause);

    /**
     * Print the fully formatted error message to standard out.
     */
    void printCompilerError() const;
};
//---------------------------------------------------------------------------
} // namespace pljit::code
//---------------------------------------------------------------------------

#endif //PLJIT_SOURCECODE_HPP
