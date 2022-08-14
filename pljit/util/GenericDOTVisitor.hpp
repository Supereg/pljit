//
// Created by Andreas Bauer on 03.08.22.
//

#ifndef PLJIT_GENERICDOTVISITOR_HPP
#define PLJIT_GENERICDOTVISITOR_HPP

#include <string_view>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
class GenericDOTVisitor {
    public:
    unsigned node_num{};

    GenericDOTVisitor();
    virtual ~GenericDOTVisitor();

    void reset();

    void printGraphHeader() const;
    void printNode(std::string_view name) const;
    void printTerminalNode(std::string_view content) const;
    void printTerminalNode(long long content) const;
    void printEdge(unsigned root_node) const;
    void printGraphFooter() const;
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_GENERICDOTVISITOR_HPP
