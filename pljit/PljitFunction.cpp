//
// Created by Andreas Bauer on 16.08.22.
//

#include "PljitFunction.hpp"
#include "./ast/ASTBuilder.hpp"
#include "./lex/Lexer.hpp"
#include "./parse/Parser.hpp"
#include <iostream>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
PljitFunction::PljitFunction(std::string&& source_code) : source_code(std::move(source_code)) {}

std::optional<long long> PljitFunction::evaluate(const std::vector<long long>& arguments) {
    ensure_compiled();

    if (compilation_error_val) {
        return {};
    }

    auto context = function->evaluate(arguments);
    if (context.runtime_error()) {
        // specification said it is enough to print the error to std out.
        std::cout << *context.runtime_error() << std::endl;
    }

    return context.return_value();
}

void PljitFunction::ensure_compiled() {
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

        if (function_compiled.load()) {
            // it could happen that the function was not compiled, but another thread compiled it and released
            // the lock before we tried to acquire the lock. Therefore, we release the lock again and return.
            return;
        }

        lex::Lexer lexer{source_code};
        parse::Parser parser{lexer};
        ast::ASTBuilder builder;

        Result<parse::FunctionDefinition> program = parser.parse_program();
        if (!program) {
            compilation_error_val = program.error();
        } else {
            Result<ast::Function> func = builder.analyzeFunction(*program);
            if (!func) {
                compilation_error_val = func.error();
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

std::optional<code::SourceCodeError> PljitFunction::compilation_error() const {
    return compilation_error_val;
}
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------
