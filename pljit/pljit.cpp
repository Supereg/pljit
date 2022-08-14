//
// Created by Andreas Bauer on 12.08.22.
//

#include "pljit.hpp"
#include "./ast/ASTBuilder.hpp"
#include "./lex/Lexer.hpp"
#include "./parse/Parser.hpp"

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
Pljit::Pljit() = default;

Pljit::PljitFunctionHandle Pljit::registerFunction(std::string&& source_code) { // NOLINT(readability-convert-member-functions-to-static)
    code::SourceCodeManagement management{ std::move(source_code) };

    std::shared_ptr<PljitFunction> function = std::make_shared<PljitFunction>(std::move(management));

    return PljitFunctionHandle{ std::move(function) };
}
//---------------------------------------------------------------------------
Pljit::PljitFunction::PljitFunction(code::SourceCodeManagement&& source_code) : source_code(std::move(source_code)) {}

Result<long long> Pljit::PljitFunction::evaluate(const std::vector<long long>& arguments) {
    ensure_compiled();

    if (compilation_error) {
        return *compilation_error;
    }

    return function->evaluate(arguments);
}

void Pljit::PljitFunction::ensure_compiled() {
    // atomically check if the function is compiled. No need to acquire any locks for repeated function calls.
    if (function_compiled.load()) {
        return;
    }

    {
        std::unique_lock lock{ compile_mutex, std::try_to_lock };

        // we mutex is already locked, function is currently compiling, therefore we wait for it to be completed!
        if (!lock.owns_lock()) {
            // either returns immediately, as it is already true or will wait till notified!
            function_compiled.wait(false);
            assert(function_compiled.load() == true);
            return;
        }

        lex::Lexer lexer{source_code};
        parse::Parser parser{lexer};
        ast::ASTBuilder builder;

        Result<parse::ParseTree::FunctionDefinition> program = parser.parse_program();
        if (program.failure()) {
            compilation_error = program.error();
        } else {
            Result<ast::Function> func = builder.analyzeFunction(*program);
            if (func.failure()) {
                compilation_error = program.error();
            } else {
                function = func.release();
            }
        }

        // the order we release things here is important.

        // (1) while still being locked, we set the `function_compiled` property
        function_compiled.store(true);
    } // (2) we release the lock

    // (3) we notify all currently waiting threads that the function is now compiled!
    function_compiled.notify_all();
}
//---------------------------------------------------------------------------
Pljit::PljitFunctionHandle::PljitFunctionHandle(std::shared_ptr<PljitFunction> handle) : handle(std::move(handle)) {}

// TODO this doesn't work!
template <typename... T>
Result<long long> Pljit::PljitFunctionHandle::operator()(T... arguments) const {
    return operator()(std::initializer_list<long long>{arguments...});
}

Result<long long> Pljit::PljitFunctionHandle::operator()(std::initializer_list<long long int> argument_list) const {
    std::vector<long long> argument_vector{argument_list};
    return handle->evaluate(argument_vector);
}
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------


