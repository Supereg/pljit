//
// Created by Andreas Bauer on 16.08.22.
//

#include "PljitFunction.hpp"
#include "./ast/ASTBuilder.hpp"
#include "./lex/Lexer.hpp"
#include "./parse/Parser.hpp"

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
PljitFunction::PljitFunction(std::string&& source_code) : source_code(std::move(source_code)) {}

Result<long long> PljitFunction::evaluate(const std::vector<long long>& arguments) {
    ensure_compiled();

    if (compilation_error) {
        return *compilation_error;
    }

    return function->evaluate(arguments);
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
} // namespace pljit
//---------------------------------------------------------------------------
