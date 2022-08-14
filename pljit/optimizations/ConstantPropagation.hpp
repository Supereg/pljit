//
// Created by Andreas Bauer on 14.08.22.
//

#ifndef PLJIT_CONSTANTPROPAGATION_HPP
#define PLJIT_CONSTANTPROPAGATION_HPP

#include "./OptimizationPass.hpp"
#include "../symbol_id.hpp"
#include <vector>
#include <memory>

//---------------------------------------------------------------------------
namespace pljit::ast {
//---------------------------------------------------------------------------
class Statement;
class Expression;
//---------------------------------------------------------------------------
namespace optimize {
//---------------------------------------------------------------------------
class ConstantPropagation : public OptimizationPass {
    class ConstTableLookup {
        public:
        class Entry {
            bool constant;
            long long current_val;

            public:
            Entry();

            bool isConstant() const;
            long long int getCurrentVal() const;

            void updateToConstant(long long current_val);
            void updateToVariable();
        };

        private:
        std::vector<Entry> constant_table_lookup;

        public:
        explicit ConstTableLookup(std::size_t symbol_count);

        Entry& operator[](symbol_id symbolId);
    };

    ConstTableLookup constTableLookup;

    public:
    ConstantPropagation();

    void optimize(Function& function) override;

    private:
    void optimize(std::unique_ptr<Statement>& statement);
    void optimize(std::unique_ptr<Expression>& expression);
};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
} // namespace pljit::ast
//---------------------------------------------------------------------------

#endif //PLJIT_CONSTANTPROPAGATION_HPP
