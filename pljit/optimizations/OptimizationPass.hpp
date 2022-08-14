//
// Created by Andreas Bauer on 14.08.22.
//

#ifndef PLJIT_OPTIMIZATIONPASS_HPP
#define PLJIT_OPTIMIZATIONPASS_HPP

//---------------------------------------------------------------------------
namespace pljit::ast {
//---------------------------------------------------------------------------
class Function;
//---------------------------------------------------------------------------
namespace optimize {
//---------------------------------------------------------------------------
class OptimizationPass {
    public:
    OptimizationPass() = default;
    virtual ~OptimizationPass() = default;

    virtual void optimize(Function& function) = 0;
};
//---------------------------------------------------------------------------
} // namespace optimize
//---------------------------------------------------------------------------
} // namespace pljit::ast
//---------------------------------------------------------------------------

#endif //PLJIT_OPTIMIZATIONPASS_HPP
