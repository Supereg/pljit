//
// Created by Andreas Bauer on 21.08.22.
//

#include "ast_utils.hpp"
#include "pljit/lex/Lexer.hpp"
#include "pljit/parse/Parser.hpp"
#include "pljit/ast/ASTBuilder.hpp"

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
Result<ast::Function> buildAST(const code::SourceCodeManagement& management) {
    lex::Lexer lexer{ management };
    parse::Parser parser{ lexer };

    Result<parse::FunctionDefinition> program = parser.parse_program();
    if (program.isFailure()) {
        program.error().printCompilerError();
    }
    assert(program.isSuccess() && "Unexpected parsing error!");

    ast::ASTBuilder builder;
    return builder.analyzeFunction(*program);
}
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------
