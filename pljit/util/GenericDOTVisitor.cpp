//
// Created by Andreas Bauer on 03.08.22.
//

#include "./GenericDOTVisitor.hpp"
#include <iostream>

//---------------------------------------------------------------------------
namespace pljit { // TODO linkage local?
//---------------------------------------------------------------------------
using std::cout;
using std::endl;
//---------------------------------------------------------------------------
GenericDOTVisitor::GenericDOTVisitor() = default;
GenericDOTVisitor::~GenericDOTVisitor() = default;

void GenericDOTVisitor::printGraphHeader() const { // NOLINT(readability-convert-member-functions-to-static)
    cout << "graph {" << endl;
}

void GenericDOTVisitor::printNode(std::string_view name) const {
    cout << "  n_" << node_num << " [label=\""<< name << "\",shape=box];" << endl;
}

void GenericDOTVisitor::printTerminalNode(std::string_view content) const {
    cout << "  n_" << node_num << R"( [label="\")" << content << R"(\""];)" << endl;
}

void GenericDOTVisitor::printTerminalNode(long long content) const {
    cout << "  n_" << node_num << "[label=\"" << content << "\"];" << endl;
}

void GenericDOTVisitor::printEdge(unsigned root_node) const {
    cout << "  n_" << root_node << " -- n_" << (node_num + 1) << ";" << endl;
}

void GenericDOTVisitor::printGraphFooter() const { // NOLINT(readability-convert-member-functions-to-static)
    cout << "}";
}
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

