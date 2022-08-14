//
// Created by Andreas Bauer on 14.08.22.
//

#ifndef PLJIT_DEADCODEELIMINATION_HPP
#define PLJIT_DEADCODEELIMINATION_HPP

#include "./OptimizationPass.hpp"

//---------------------------------------------------------------------------
namespace pljit::ast::optimize {
//---------------------------------------------------------------------------
class DeadCodeElimination: public OptimizationPass {
    public:
    void optimize(Function& function) override;
};
//---------------------------------------------------------------------------
} // namespace pljit::ast::optimize
//---------------------------------------------------------------------------

#endif //PLJIT_DEADCODEELIMINATION_HPP
