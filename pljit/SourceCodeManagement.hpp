//
// Created by Andreas Bauer on 13.07.22.
//

#ifndef PLJIT_SOURCECODEMANAGEMENT_H
#define PLJIT_SOURCECODEMANAGEMENT_H
#include <string>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
class SourceCodeReference;
//---------------------------------------------------------------------------
class SourceCodeManagement {
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
        // TODO copy consturcotr!

        bool operator==(const iterator& other) const;

        iterator& operator++();
        iterator operator++(int);

        iterator& operator--();
        iterator operator--(int);

        reference operator*() const;

        // TODO want to derive a SourceCodeReference!
        SourceCodeReference codeReference() const;
    };

    enum class ErrorType {
        NOTE,
        ERROR,
    };

    explicit SourceCodeManagement(std::string&& source_code);
    // TODO destructor?

    SourceCodeManagement(const SourceCodeManagement& other) = delete;

    SourceCodeManagement& operator=(const SourceCodeManagement& other) = delete;

    std::string_view content() const;
    iterator begin() const;
    iterator end() const;

    // TODO may this be private/internal?
    void print_error(ErrorType type, std::string_view message, const SourceCodeReference& reference) const;
};
//---------------------------------------------------------------------------
class SourceCodeReference {
    friend SourceCodeManagement::iterator; // allows access to private constructors!

    const SourceCodeManagement* management; // TODO init in constructors!

    std::string_view string_content;

    SourceCodeReference(const SourceCodeManagement* management, std::string_view string_content);

    public:
    SourceCodeReference();

    std::string_view content() const;

    void extend(int amount);

    // TODO replace ERROR; NOTE; WARN with enum type!
    void print_error(pljit::SourceCodeManagement::ErrorType type, std::string_view message) const;
    // TODO create version which allows to specify single code point?
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_SOURCECODEMANAGEMENT_H
