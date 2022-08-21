//
// Created by Andreas Bauer on 14.08.22.
//

#ifndef PLJIT_EVALUATIONCONTEXT_HPP
#define PLJIT_EVALUATIONCONTEXT_HPP

#include "./symbol_id.hpp"
#include <optional>
#include <vector>
#include <string_view>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
/**
 * The EvaluationContext is used to store contextual information within the
 * execution of a Function.
 */
class EvaluationContext {
    /// Values of allocated variables.
    std::vector<long long> variables;
    /// The return value of a function if already evaluated.
    std::optional<long long> return_val;

    /// Stores the error message of runtime errors
    std::optional<std::string_view> runtime_error_message;

    public:
    explicit EvaluationContext(std::size_t symbols);

    /**
     * Access the current value of a given variable.
     * @param symbolId The symbol id of a given `Variable`.
     * @return Returns a reference to the current variable value.
     */
    long long& operator[](symbol_id symbolId);

    /**
     * @return Returns the return value if present. The value is present once
     * a Return statement was evaluated.
     */
    std::optional<long long>& return_value();

    /**
     * @return Returns the runtime error message if one occurred during execution.
     */
    std::optional<std::string_view> runtime_error() const;

    void setRuntimeError(const std::optional<std::string_view>& runtime_error);
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_EVALUATIONCONTEXT_HPP
