//
// Created by Andreas Bauer on 13.07.22.
//

#include "SourceCodeManagement.hpp"
#include <cassert>
#include <iostream>

//---------------------------------------------------------------------------
pljit::SourceCodeManagement::SourceCodeManagement(std::string&& source_code)
    : source_code(source_code),
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

void pljit::SourceCodeManagement::print_error(pljit::SourceCodeManagement::ErrorType type, std::string_view message, const SourceCodeReference& reference) const {
    unsigned column = 0;
    unsigned line = 1;

    {
        std::string_view::iterator iterator = reference.content().begin();
        assert(iterator >= source_code_view.begin() && iterator < source_code_view.end() && "Illegal range!");

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
    }

    std::string_view name; // TODO move switch to method!
    switch (type) {
        case ErrorType::ERROR:
            name = "error";
            break;
        case ErrorType::NOTE:
            name = "note";
            break;
    }

    // PRINT ERROR LINE
    std::cout << line << ':' << (column + 1) << ": " << name << ": " << message << std::endl;

    // PRINT CODE LINE
    std::string_view::iterator message_iterator_begin = reference.content().begin() - column;
    std::string_view::iterator message_iterator_end = reference.content().begin();
    for (; *message_iterator_end != '\n' && message_iterator_end != source_code_view.end(); ++message_iterator_end) {}

    std::string_view code_line{message_iterator_begin, message_iterator_end};
    std::cout << code_line << std::endl;

    // PRINT ERROR INDICATOR
    for (unsigned i = column; i > 0; --i) {
        std::cout << ' ';
    }
    std::cout << '^';
    for (unsigned i = 0; i < reference.content().size() - 1; ++i) {
        std::cout << "~";
    }
    std::cout << std::endl;
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

    return { management, {begin, end}};
}
//---------------------------------------------------------------------------
pljit::SourceCodeReference::SourceCodeReference()
    : management(nullptr), string_content() {}
// TODO ensure we handle empty references properly!

pljit::SourceCodeReference::SourceCodeReference(const pljit::SourceCodeManagement* management, std::string_view string_content)
    : management(management), string_content(string_content) {
}

std::string_view pljit::SourceCodeReference::content() const {
    assert(management != nullptr);
    return string_content;
}

void pljit::SourceCodeReference::extend(int amount) {
    assert(management != nullptr);
    assert(string_content.end() + amount <= &(*management->end()));

    string_content = { string_content.data(), string_content.size() + amount };
}
//---------------------------------------------------------------------------
pljit::SourceCodeError::SourceCodeError(pljit::SourceCodeManagement::ErrorType errorType, std::string_view errorMessage, pljit::SourceCodeReference sourceCodeReference)
    : errorType(errorType), errorMessage(errorMessage), sourceCodeReference(sourceCodeReference) {}

pljit::SourceCodeManagement::ErrorType pljit::SourceCodeError::type() const {
    return errorType;
}

std::string_view pljit::SourceCodeError::message() const {
    return errorMessage;
}

const pljit::SourceCodeReference& pljit::SourceCodeError::reference() const {
    return sourceCodeReference;
}

void pljit::SourceCodeError::printCompilerError() const {
    sourceCodeReference.management->print_error(errorType, errorMessage, sourceCodeReference);
}
//---------------------------------------------------------------------------
