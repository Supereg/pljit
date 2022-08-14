//
// Created by Andreas Bauer on 12.08.22.
//

#ifndef PLJIT_PLJIT_HPP
#define PLJIT_PLJIT_HPP

#include "./util/Result.hpp"
#include "./code/SourceCodeManagement.hpp"
#include "./ast/AST.hpp"
#include <initializer_list>
#include <string>
#include <optional>
#include <atomic>
#include <mutex>
#include <memory>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
class Pljit {
    public:

    // TODO make those non-nested types!
    class PljitFunction {
        code::SourceCodeManagement source_code;

        std::atomic<bool> function_compiled;
        std::mutex compile_mutex;

        std::optional<ast::Function> function;
        std::optional<code::SourceCodeError> compilation_error;

        public:
        explicit PljitFunction(code::SourceCodeManagement&& source_code);

        Result<long long> evaluate(const std::vector<long long>& arguments);
        void ensure_compiled();
    };

    // TODO make those non-nested types!
    class PljitFunctionHandle {
        std::shared_ptr<PljitFunction> handle;

        public:
        explicit PljitFunctionHandle(std::shared_ptr<PljitFunction> handle);

        // Copy constructor
        PljitFunctionHandle(const PljitFunctionHandle& other) = default;
        // Move constructor
        PljitFunctionHandle(PljitFunctionHandle&& other) noexcept = default;
        // Copy assignment
        PljitFunctionHandle& operator=(const PljitFunctionHandle& other) = default;
        // Move assignment
        PljitFunctionHandle& operator=(PljitFunctionHandle&& other) noexcept = default;

        template <typename... T>
        Result<long long> operator()(T... arguments) const;
        Result<long long> operator()(std::initializer_list<long long> argument_list) const;
    };

    Pljit();

    // TODO specification tells that stuff should be stored within the JIT class?
    PljitFunctionHandle registerFunction(std::string&& source_code);
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_PLJIT_HPP
