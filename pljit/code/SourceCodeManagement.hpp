//
// Created by Andreas Bauer on 13.07.22.
//

#ifndef PLJIT_SOURCECODEMANAGEMENT_H
#define PLJIT_SOURCECODEMANAGEMENT_H

#include "./SourceCode.hpp"
#include <string>
#include <string_view>

//---------------------------------------------------------------------------
namespace pljit::code {
//---------------------------------------------------------------------------
class SourceCodeManagement;
//---------------------------------------------------------------------------
/**
 * Iterator to iterate over source code characters.
 * Provides means to create SourceCodeReferences from the current codePosition.
 */
class SourceIterator {
    friend class SourceCodeManagement; // Access to the constructor.

    const SourceCodeManagement* management;
    std::string_view::iterator view_iterator;

    SourceIterator(const SourceCodeManagement* management, std::string_view::iterator view_iterator);

    public:
    using difference_type = std::ptrdiff_t;
    using value_type = const char;
    using reference = value_type&;
    using iterator_category = std::bidirectional_iterator_tag;

    SourceIterator();

    bool operator==(const SourceIterator& other) const;

    SourceIterator& operator++();
    SourceIterator operator++(int);

    SourceIterator& operator--();
    SourceIterator operator--(int);

    reference operator*() const;

    /**
     * Create a `SourceCodeReference` at the current iterator codePosition.
     * The resulting reference will be of length 1.
     * @return A `SourceCodeReference` containing the current iterator codePosition.
     */
    SourceCodeReference codeReference() const;
};
//---------------------------------------------------------------------------
class SourceCodeManagement {
    std::string source_code;
    std::string_view source_code_view;

    public:
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
    SourceIterator begin() const;
    SourceIterator end() const;

    /**
     * Calculate the Position of a given `SourceCodeReference`.
     * @param reference - The `SourceCodeReference to calculate the codePosition for.
     * @return Returns the `CodePosition` of the first character in the SourceCodeReference!
     */
    CodePosition getPosition(const SourceCodeReference& reference) const;
};
//---------------------------------------------------------------------------
} // namespace pljit::code
//---------------------------------------------------------------------------

#endif //PLJIT_SOURCECODEMANAGEMENT_H
