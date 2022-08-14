//
// Created by Andreas Bauer on 14.08.22.
//

#ifndef PLJIT_EVALUATIONCONTEXT_HPP
#define PLJIT_EVALUATIONCONTEXT_HPP

#include "./symbol_id.hpp"
#include <optional>
#include <vector>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
class EvaluationContext {
    std::vector<long long> variables;
    std::optional<long long> return_val;

    // TODO implementation suggest storing error in here?

    public:
    explicit EvaluationContext(std::size_t symbols);

    long long& operator[](symbol_id symbolId);

    std::optional<long long>& return_value();
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_EVALUATIONCONTEXT_HPP
