//
// Created by Andreas Bauer on 13.07.22.
//

#include "./SourceCodeManagement.hpp"
#include <cassert>

//---------------------------------------------------------------------------
namespace pljit::code {
//---------------------------------------------------------------------------
SourceIterator::SourceIterator(const SourceCodeManagement* management, std::string_view::iterator view_iterator)
    : management(management), view_iterator(view_iterator) {}

SourceIterator::SourceIterator() : management(nullptr), view_iterator() {}

bool SourceIterator::operator==(const SourceIterator& other) const {
    return management == other.management && view_iterator == other.view_iterator;
}

SourceIterator& SourceIterator::operator++() {
    ++view_iterator;
    return *this;
}

SourceIterator SourceIterator::operator++(int) {
    SourceIterator copy{*this};
    operator++();
    return copy;
}

SourceIterator& SourceIterator::operator--() {
    --view_iterator;
    return *this;
}

SourceIterator SourceIterator::operator--(int) {
    SourceIterator copy{*this};
    operator--();
    return copy;
}

SourceIterator::reference SourceIterator::operator*() const {
    return *view_iterator;
}

SourceCodeReference SourceIterator::codeReference() const {
    std::string_view::iterator begin = &operator*();
    std::string_view::iterator end = begin;
    ++end;

    return { management, {begin, end}};
}
//---------------------------------------------------------------------------
SourceCodeManagement::SourceCodeManagement(std::string&& source_code)
    : source_code(source_code),
      source_code_view(this->source_code){
}

SourceIterator SourceCodeManagement::begin() const {
    return { this, source_code_view.begin() };
}

SourceIterator SourceCodeManagement::end() const {
    return { this, source_code_view.end() };
}

std::string_view SourceCodeManagement::content() const {
    return source_code_view;
}

CodePosition SourceCodeManagement::getPosition(const SourceCodeReference& reference) const {
    unsigned column = 0;
    unsigned line = 1;

    std::string_view::iterator iterator = reference->begin();
    assert(iterator >= source_code_view.begin() && iterator <= source_code_view.end() && "Illegal range!");

    while(--iterator >= source_code_view.begin()) {
        if (*iterator == '\n') {
            break;
        }

        ++column;
    }

    for (; iterator >= source_code_view.begin(); --iterator) {
        if (*iterator == '\n') {
            ++line;
        }
    }
    return { line, column + 1 };
}
//---------------------------------------------------------------------------
} // namespace pljit::code
//---------------------------------------------------------------------------
