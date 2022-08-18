//
// Created by Andreas Bauer on 11.08.22.
//

#ifndef PLJIT_LANG_HPP
#define PLJIT_LANG_HPP

#include "pljit/code/SourceCodeManagement.hpp"
#include "pljit/util/Result.hpp"
#include <cassert>
#include <optional>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
struct Keyword {
    static constexpr std::string_view PARAM = "PARAM";
    static constexpr std::string_view VAR = "VAR";
    static constexpr std::string_view CONST = "CONST";
    static constexpr std::string_view BEGIN = "BEGIN";
    static constexpr std::string_view END = "END";
    static constexpr std::string_view RETURN = "RETURN";
};

struct Operator {
    static constexpr std::string_view PLUS = "+";
    static constexpr std::string_view MINUS = "-";
    static constexpr std::string_view MULTIPLICATION = "*";
    static constexpr std::string_view DIVISION = "/";
    static constexpr std::string_view INIT = "=";
    static constexpr std::string_view ASSIGNMENT = ":=";
};

struct Parenthesis {
    static constexpr std::string_view ROUND_OPEN = "(";
    static constexpr std::string_view ROUND_CLOSE = ")";
};

struct Separator {
    static constexpr std::string_view COMMA = ",";
    static constexpr std::string_view SEMICOLON = ";";
    static constexpr std::string_view END_OF_PROGRAM = ".";
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_LANG_HPP
