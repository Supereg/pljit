//
// Created by Andreas Bauer on 16.08.22.
//

#ifndef PLJIT_PLJITFUNCTION_HPP
#define PLJIT_PLJITFUNCTION_HPP

#include "./code/SourceCodeManagement.hpp"
#include "./ast/AST.hpp" // TODO not required when splitting out FunctionHandle!
#include <atomic>
#include <mutex>
#include <optional>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
class PljitFunction { // TODO move the Function into another header, reduces header requirments!
    code::SourceCodeManagement source_code;

    std::atomic<bool> function_compiled;
    std::mutex compile_mutex;

    std::optional<ast::Function> function; // TODO use unique pointer, then we can reduce header file size!
    std::optional<code::SourceCodeError> compilation_error;

    public:
    explicit PljitFunction(std::string&& source_code);

    // We can't safely copy or move without encountering any potential synchronization issues.
    PljitFunction(const PljitFunction& other) = delete;
    PljitFunction(PljitFunction&& other) = delete;

    Result<long long> evaluate(const std::vector<long long>& arguments);
    void ensure_compiled();
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_PLJITFUNCTION_HPP
