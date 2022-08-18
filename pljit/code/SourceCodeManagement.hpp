//
// Created by Andreas Bauer on 13.07.22.
//

#ifndef PLJIT_SOURCECODEMANAGEMENT_H
#define PLJIT_SOURCECODEMANAGEMENT_H

#include <string>
#include <string_view>
#include <vector>

//---------------------------------------------------------------------------
namespace pljit::code {
//---------------------------------------------------------------------------
class SourceCodeReference; // TODO move those to individual headers?
class SourceCodeError;
//---------------------------------------------------------------------------
enum class ErrorType {
    NOTE,
    ERROR,
};
//---------------------------------------------------------------------------
class SourceCodeManagement {
    // TODO coupling?
    friend class SourceCodeError; // access to `print_error`

    std::string source_code;
    std::string_view source_code_view;

    public:
    class iterator {
        friend class SourceCodeManagement;

        const SourceCodeManagement* management;
        std::string_view::iterator view_iterator;

        iterator(const SourceCodeManagement* management, std::string_view::iterator view_iterator);

        public:
        using difference_type = std::ptrdiff_t;
        using value_type = const char;
        using reference = value_type&;
        using iterator_category = std::bidirectional_iterator_tag;

        iterator();

        bool operator==(const iterator& other) const;

        iterator& operator++();
        iterator operator++(int);

        iterator& operator--();
        iterator operator--(int);

        reference operator*() const;

        SourceCodeReference codeReference() const;
    };

    explicit SourceCodeManagement(std::string&& source_code);

    /// Delete copy construction. We don't want to allow copying the string.
    SourceCodeManagement(const SourceCodeManagement& other) = delete;
    /// Move constructor.
    SourceCodeManagement(SourceCodeManagement&& other) noexcept = default;

    /// Delete copy assignment. We don't want to allow copying the string.
    SourceCodeManagement& operator=(const SourceCodeManagement& other) = delete;
    /// Move assignment.
    SourceCodeManagement& operator=(SourceCodeManagement&& other) noexcept = default;

    std::string_view content() const;
    iterator begin() const;
    iterator end() const;

    private:
    void print_error(ErrorType type, std::string_view message, const SourceCodeReference& reference) const;
};
//---------------------------------------------------------------------------
class SourceCodeReference {
    friend class SourceCodeManagement::iterator; // allows access to private constructors!
    friend class SourceCodeError; // allow access to `management` for error printing! TODO coupling?

    const SourceCodeManagement* management;
    std::string_view string_content;

    SourceCodeReference(const SourceCodeManagement* management, std::string_view string_content);

    public:
    SourceCodeReference();

    std::string_view content() const;

    void extend(int amount);

    SourceCodeError makeError(ErrorType errorType, std::string_view message) const;

    bool operator==(const SourceCodeReference& rhs) const;
};
//---------------------------------------------------------------------------
class SourceCodeError {
    ErrorType errorType;
    std::string_view errorMessage;
    SourceCodeReference sourceCodeReference;

    // TODO use std::list!
    std::vector<SourceCodeError> causes;
    public:

    SourceCodeError(ErrorType errorType, std::string_view errorMessage, SourceCodeReference sourceCodeReference);

    ErrorType type() const;
    std::string_view message() const;
    const SourceCodeReference& reference() const;

    SourceCodeError& withCause(SourceCodeError&& error_cause);

    void printCompilerError() const;

    bool operator==(const SourceCodeError& rhs) const;
    // TODO API ability to derive line numbers! (also useful for tests!)
};
//---------------------------------------------------------------------------
} // namespace pljit::code
//---------------------------------------------------------------------------

#endif //PLJIT_SOURCECODEMANAGEMENT_H
