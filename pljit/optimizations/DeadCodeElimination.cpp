//
// Created by Andreas Bauer on 14.08.22.
//

#include "./DeadCodeElimination.hpp"
#include "../ast/AST.hpp"

//---------------------------------------------------------------------------
namespace pljit::ast::optimize {
//---------------------------------------------------------------------------
void DeadCodeElimination::optimize(Function& function) {
    std::vector<std::unique_ptr<Statement>>& statements = function.getStatements();

    auto iterator = statements.begin();
    for (; iterator != statements.end(); ++iterator) {
        if ((*iterator)->getType() == Node::Type::RETURN_STATEMENT) {
            ++iterator;
            break;
        }
    }

    if (iterator != statements.end()) {
        // remove dead code!
        statements.erase(iterator, statements.end());
    }
}
//---------------------------------------------------------------------------
} // namespace pljit::ast::optimize
//---------------------------------------------------------------------------
