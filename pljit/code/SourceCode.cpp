//
// Created by Andreas Bauer on 18.08.22.
//

#include "SourceCode.hpp"
#include "./SourceCodeManagement.hpp"
#include <cassert>
#include <iostream>

//---------------------------------------------------------------------------
namespace pljit::code {
//---------------------------------------------------------------------------
namespace {
//---------------------------------------------------------------------------
std::string_view errorTypeStringRepresentation(ErrorType errorType) {
    std::string_view name;
    switch (errorType) {
        case ErrorType::ERROR:
            name = "error";
            break;
        case ErrorType::NOTE:
            name = "note";
            break;
    }
    return name;
}
//---------------------------------------------------------------------------
} // namespace
//---------------------------------------------------------------------------
CodePosition::CodePosition(unsigned line_num, unsigned column_num): line_num(line_num), column_num(column_num) {} // NOLINT(bugprone-easily-swappable-parameters)

unsigned CodePosition::column() const {
    return column_num;
}

unsigned CodePosition::line() const {
    return line_num;
}
bool CodePosition::operator==(const CodePosition& rhs) const {
    return column_num == rhs.column_num && line_num == rhs.line_num;
}
//---------------------------------------------------------------------------
SourceCodeReference::SourceCodeReference()
    : management(nullptr), string_content() {}

SourceCodeReference::SourceCodeReference(const SourceCodeManagement* management, std::string_view string_content)
    : management(management), string_content(string_content) {
}

SourceCodeReference::SourceCodeReference(const SourceCodeReference& start, const SourceCodeReference& end)
    : management(start.management), string_content(start->begin(), end->end()) {
    assert(start.management == end.management && "Used SourceCodeReference fro, two different managements!");
    assert(start->begin() < end->begin() && "Confused order constructing SourceCodeReference!");
}

std::string_view SourceCodeReference::operator*() const {
    assert(management != nullptr && "Can't access content of an empty SourceCodeReference!");
    return string_content;
}

const std::string_view* SourceCodeReference::operator->() const {
    assert(management != nullptr && "Can't access content of an empty SourceCodeReference!");
    return &string_content;
}

CodePosition SourceCodeReference::position() const {
    assert(management != nullptr && "Can't access content of an empty SourceCodeReference!");
    return management->getPosition(*this);
}

void SourceCodeReference::extend(int amount) {
    assert(management != nullptr && "Can't access content of an empty SourceCodeReference!");
    assert(string_content.end() + amount <= &(*management->end()));

    string_content = { string_content.data(), string_content.size() + amount };
}

SourceCodeError SourceCodeReference::makeError(ErrorType errorType, std::string_view message) const {
    return { errorType, message, *this };
}

bool SourceCodeReference::operator==(const SourceCodeReference& rhs) const {
    return management == rhs.management &&
        string_content.begin() == rhs.string_content.begin() &&
        string_content.end() == rhs.string_content.end() &&
        string_content == rhs.string_content;
}
//---------------------------------------------------------------------------
SourceCodeError::SourceCodeError(ErrorType errorType, std::string_view errorMessage, SourceCodeReference sourceCodeReference)
    : errorType(errorType), errorMessage(errorMessage), codeReference(sourceCodeReference), codePosition(sourceCodeReference.position()) {}

ErrorType SourceCodeError::type() const {
    return errorType;
}

std::string_view SourceCodeError::message() const {
    return errorMessage;
}

const SourceCodeReference& SourceCodeError::reference() const {
    return codeReference;
}

CodePosition SourceCodeError::position() const {
    return codePosition;
}

const std::list<SourceCodeError>& SourceCodeError::attachedCauses() const {
    return causes;
}

SourceCodeError& SourceCodeError::attachCause(SourceCodeError&& error_cause) {
    causes.emplace_back(error_cause);
    return *this;
}

void SourceCodeError::printCompilerError() const {
    unsigned line = codePosition.line();
    unsigned column = codePosition.column();

    // PRINT ERROR LINE
    std::cout << line << ':' << column << ": ";
    std::cout << errorTypeStringRepresentation(errorType) << ": " << errorMessage << std::endl;

    // PRINT CODE LINE
    std::string_view::iterator message_iterator_begin = codeReference->begin() - (column - 1);
    std::string_view::iterator message_iterator_end = codeReference->begin();

    // TODO maybe support multiline error messages?
    for (; *message_iterator_end != '\n' && message_iterator_end != codeReference.management->content().end(); ++message_iterator_end) {}

    std::string_view code_line{message_iterator_begin, message_iterator_end};
    std::cout << code_line << std::endl;


    // PRINT ERROR INDICATOR
    for (unsigned i = 0; i < (column - 1); ++i) {
        if (*(message_iterator_begin + i) == '\t') {
            std::cout << '\t';
        } else {
            std::cout << ' ';
        }
    }
    std::cout << '^';
    for (unsigned i = 1; i < codeReference->size(); ++i) {
        std::cout << "~";
    }
    std::cout << std::endl;

    // PRINT CAUSES
    for (auto& cause: causes) {
        cause.printCompilerError();
    }
}
//---------------------------------------------------------------------------
} // namespace pljit::code
//---------------------------------------------------------------------------
