//
// Created by Andreas Bauer on 14.08.22.
//

#include "EvaluationContext.hpp"
#include <cassert>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
EvaluationContext::EvaluationContext(std::size_t symbols) : variables(symbols), return_val() {}

long long& EvaluationContext::operator[](symbol_id symbolId) {
    assert(symbolId > 0 && symbolId <= variables.size() && "Encountered illegal symbol id!");
    return variables[symbolId - 1];
}

std::optional<long long>& EvaluationContext::return_value() {
    return return_val;
}
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------
