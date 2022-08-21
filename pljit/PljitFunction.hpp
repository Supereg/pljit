//
// Created by Andreas Bauer on 16.08.22.
//

#ifndef PLJIT_PLJITFUNCTION_HPP
#define PLJIT_PLJITFUNCTION_HPP

#include "./code/SourceCodeManagement.hpp"
#include "./ast/AST.hpp"
#include <atomic>
#include <mutex>
#include <optional>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
/**
 * A Pljit Function instance. This object holds the source code and the compiled AST.
 */
class PljitFunction {
    /// Source code of the function.
    code::SourceCodeManagement source_code;

    /// Atomic bool which makes it easy and fast to check if the function was already compiled.
    std::atomic<bool> function_compiled;
    /// A mutex to ensure mutual exclusion when compiling the function.
    std::mutex compile_mutex;

    /// The compiled AST. Present if compiled and no compilation error occurred.
    std::optional<ast::Function> function;
    /// A potential compilation error. Present if compiled and a compilation error occurred.
    std::optional<code::SourceCodeError> compilation_error_val;

    public:
    explicit PljitFunction(std::string&& source_code);

    // We can't safely copy or move without encountering any potential synchronization issues.
    PljitFunction(const PljitFunction& other) = delete;
    PljitFunction(PljitFunction&& other) = delete;

    /**
     * A call to this function will evaluate the function.
     * The function is compiled before execution if it wasn't compiled yet.
     * @param arguments The vector of arguments passed to the compiled function.
     * @return Returns the value of the function evaluation. The optional might be empty if
     * either a compilation error or a runtime error occurred.
     * You can use the `compilation_error()` getter to get access to the compilation error.
     * Runtime errors are printed to standard out.
     */
    std::optional<long long> evaluate(const std::vector<long long>& arguments);

    /**
     * A call to this method will ensure that the function is compiled.
     */
    void ensure_compiled();

    /**
     * @return Returns the compilation error, if one occurred.
     */
    std::optional<code::SourceCodeError> compilation_error() const;
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_PLJITFUNCTION_HPP
