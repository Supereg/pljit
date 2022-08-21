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

std::optional<std::string_view> EvaluationContext::runtime_error() const {
    return runtime_error_message;
}

void EvaluationContext::setRuntimeError(const std::optional<std::string_view>& runtime_error) {
    runtime_error_message = runtime_error;
}
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------
