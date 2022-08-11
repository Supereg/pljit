//
// Created by Andreas Bauer on 13.07.22.
//

#include "SourceCodeManagement.hpp"
#include <cassert>
#include <iostream>

//---------------------------------------------------------------------------
namespace pljit::code {
//---------------------------------------------------------------------------
SourceCodeManagement::SourceCodeManagement(std::string&& source_code)
    : source_code(source_code),
      source_code_view(this->source_code){
}

SourceCodeManagement::iterator SourceCodeManagement::begin() const {
    return { this, source_code_view.begin() };
}

SourceCodeManagement::iterator SourceCodeManagement::end() const {
    return { this, source_code_view.end() };
}

std::string_view SourceCodeManagement::content() const {
    return source_code_view;
}

void SourceCodeManagement::print_error(
    SourceCodeManagement::ErrorType type,
    std::string_view message,
    const SourceCodeReference& reference
) const {
    unsigned column = 0;
    unsigned line = 1;

    {
        std::string_view::iterator iterator = reference.content().begin();
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
SourceCodeManagement::iterator::iterator(const SourceCodeManagement* management, std::string_view::iterator view_iterator)
    : management(management), view_iterator(view_iterator) {}

SourceCodeManagement::iterator::iterator() : management(nullptr), view_iterator() {}

bool SourceCodeManagement::iterator::operator==(const SourceCodeManagement::iterator& other) const {
    return management == other.management && view_iterator == other.view_iterator;
}

SourceCodeManagement::iterator& SourceCodeManagement::iterator::operator++() {
    ++view_iterator;
    return *this;
}

SourceCodeManagement::iterator SourceCodeManagement::iterator::operator++(int) {
    iterator copy{*this};
    operator++();
    return copy;
}

SourceCodeManagement::iterator& SourceCodeManagement::iterator::operator--() {
    --view_iterator;
    return *this;
}

SourceCodeManagement::iterator SourceCodeManagement::iterator::operator--(int) {
    iterator copy{*this};
    operator--();
    return copy;
}

SourceCodeManagement::iterator::reference SourceCodeManagement::iterator::operator*() const {
    return *view_iterator;
}

SourceCodeReference SourceCodeManagement::iterator::codeReference() const {
    std::string_view::iterator begin = &operator*();
    std::string_view::iterator end = begin;
    ++end;

    return { management, {begin, end}};
}
//---------------------------------------------------------------------------
SourceCodeReference::SourceCodeReference()
    : management(nullptr), string_content() {}
// TODO ensure we handle empty references properly!

SourceCodeReference::SourceCodeReference(const SourceCodeManagement* management, std::string_view string_content)
    : management(management), string_content(string_content) {
}

std::string_view SourceCodeReference::content() const {
    assert(management != nullptr);
    return string_content;
}

void SourceCodeReference::extend(int amount) {
    assert(management != nullptr);
    assert(string_content.end() + amount <= &(*management->end()));

    string_content = { string_content.data(), string_content.size() + amount };
}

SourceCodeError SourceCodeReference::makeError(SourceCodeManagement::ErrorType errorType, std::string_view message) const {
    return { errorType, message, *this };
}

bool SourceCodeReference::operator==(const SourceCodeReference& rhs) const {
    return management == rhs.management &&
        string_content == rhs.string_content;
}
//---------------------------------------------------------------------------
SourceCodeError::SourceCodeError(SourceCodeManagement::ErrorType errorType, std::string_view errorMessage, SourceCodeReference sourceCodeReference)
    : errorType(errorType), errorMessage(errorMessage), sourceCodeReference(sourceCodeReference) {}

SourceCodeManagement::ErrorType SourceCodeError::type() const {
    return errorType;
}

std::string_view SourceCodeError::message() const {
    return errorMessage;
}

const SourceCodeReference& SourceCodeError::reference() const {
    return sourceCodeReference;
}

SourceCodeError& SourceCodeError::withCause(SourceCodeError&& error_cause) {
    causes.emplace_back(error_cause);
    return *this;
}

void SourceCodeError::printCompilerError() const {
    sourceCodeReference.management->print_error(errorType, errorMessage, sourceCodeReference);
    for (auto& cause: causes) {
        cause.printCompilerError();
    }
}

bool SourceCodeError::operator==(const SourceCodeError& rhs) const {
    return errorType == rhs.errorType &&
        errorMessage == rhs.errorMessage &&
        sourceCodeReference == rhs.sourceCodeReference;
}
//---------------------------------------------------------------------------
} // namespace pljit::code
//---------------------------------------------------------------------------
