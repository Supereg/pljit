//
// Created by Andreas Bauer on 13.07.22.
//

#include "SourceCodeManagement.hpp"
#include <cassert>
#include <iostream>

//---------------------------------------------------------------------------
pljit::SourceCodeManagement::SourceCodeManagement(std::string source_code)
    : source_code(std::move(source_code)),
      source_code_view(this->source_code){
}

pljit::SourceCodeManagement::iterator pljit::SourceCodeManagement::begin() const {
    return { this, source_code_view.begin() };
}

pljit::SourceCodeManagement::iterator pljit::SourceCodeManagement::end() const {
    return { this, source_code_view.end() };
}
std::string_view pljit::SourceCodeManagement::content() const {
    return source_code_view;
}

// TODO check if pair is a viable solution for this?
std::pair<unsigned, unsigned> pljit::SourceCodeManagement::line_number_and_column(const std::string_view::iterator& character) const {
    assert(character >= source_code_view.begin() && character < source_code_view.end() && "Illegal range!");

    std::string_view::iterator iterator = character; // copy the iterator to work on!

    unsigned column = 0;
    unsigned line = 1;

    for (; iterator >= source_code_view.begin(); --iterator) {
        if (*iterator == '\n') {
            // trying to derive the column for a reference pointing to newline is rather weird
            // and undefined behavior and will always report being the 0th column in the next line.
            // That's just we defined it here to keep it simple!
            break;
        }

        ++column;
    }

    for (; iterator >= source_code_view.begin(); --iterator) {
        if (*iterator == '\n') {
            ++line;
        }
    }

    return { line, column };
}
//---------------------------------------------------------------------------
pljit::SourceCodeManagement::iterator::iterator(const pljit::SourceCodeManagement* management, std::string_view::iterator view_iterator)
    : management(management), view_iterator(view_iterator) {}

pljit::SourceCodeManagement::iterator::iterator() : management(nullptr), view_iterator() {}

bool pljit::SourceCodeManagement::iterator::operator==(const pljit::SourceCodeManagement::iterator& other) const {
    return management == other.management && view_iterator == other.view_iterator;
}

pljit::SourceCodeManagement::iterator& pljit::SourceCodeManagement::iterator::operator++() {
    ++view_iterator;
    return *this;
}

pljit::SourceCodeManagement::iterator pljit::SourceCodeManagement::iterator::operator++(int) {
    iterator copy{*this};
    operator++();
    return copy;
}

pljit::SourceCodeManagement::iterator& pljit::SourceCodeManagement::iterator::operator--() {
    --view_iterator;
    return *this;
}

pljit::SourceCodeManagement::iterator pljit::SourceCodeManagement::iterator::operator--(int) {
    iterator copy{*this};
    operator--();
    return copy;
}

pljit::SourceCodeManagement::iterator::reference pljit::SourceCodeManagement::iterator::operator*() const {
    return *view_iterator;
}

pljit::SourceCodeReference pljit::SourceCodeManagement::iterator::codeReference() const {
    std::string_view::iterator begin = &operator*();
    std::string_view::iterator end = begin;
    ++end;

    return { {begin, end}, &(*management->end()) };
}
//---------------------------------------------------------------------------
pljit::SourceCodeReference::SourceCodeReference()
    : string_content(), source_code_end(nullptr) {}
// TODO ensure we handle empty references properly!

pljit::SourceCodeReference::SourceCodeReference(std::string_view string_content, std::string_view::iterator source_code_end)
    : string_content(string_content), source_code_end(source_code_end) {
}

std::string_view pljit::SourceCodeReference::content() const {
    assert(source_code_end != nullptr);
    return string_content;
}

void pljit::SourceCodeReference::extend(int amount) {
    assert(source_code_end != nullptr);
    assert(string_content.end() + amount <= source_code_end);

    string_content = { string_content.data(), string_content.size() + amount };
}
void pljit::SourceCodeReference::print_information(std::string_view name, std::string_view message) const {
    auto [line, column] = management->line_number_and_column(string_content.begin());

    std::cout << line << ':' << column << ": " << name << ": " << message << std::endl;

    // TODO get the content till the first line! (move method handling into the management)

    for (unsigned i = column - 1; i > 0; --i) {
        std::cout << ' ';
    }
    std::cout << '^';
    for (unsigned i = 1; i < string_content.size(); ++i) {
        std::cout << "~";
    }
    std::cout << std::endl;
}
//---------------------------------------------------------------------------
