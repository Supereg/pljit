//
// Created by Andreas Bauer on 21.08.22.
//

#ifndef PLJIT_AST_UTILS_HPP
#define PLJIT_AST_UTILS_HPP

#include "pljit/ast/AST.hpp"
#include "pljit/util/Result.hpp"
#include "pljit/code/SourceCodeManagement.hpp"

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
Result<ast::Function> buildAST(const code::SourceCodeManagement& management);
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_AST_UTILS_HPP
