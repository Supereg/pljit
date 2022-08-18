//
// Created by Andreas Bauer on 12.08.22.
//

#include "pljit.hpp"
#include "./PljitFunction.hpp"
#include <atomic>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
PljitFunctionHandle::PljitFunctionHandle(PljitFunction* function) : function(function) {}

Pljit::~Pljit() {
    ListNode* node = list_head;
    while (node) {
        ListNode* next = node->next;
        node->~ListNode();
        allocator.deallocate(node, 1);
        node = next;
    }
}

Result<long long> PljitFunctionHandle::operator()(std::initializer_list<long long int> argument_list) const {
    std::vector<long long> argument_vector{argument_list};
    return function->evaluate(argument_vector);
}
//---------------------------------------------------------------------------
Pljit::ListNode::ListNode(std::unique_ptr<PljitFunction> function) : function(std::move(function)), next(nullptr) {}
//---------------------------------------------------------------------------
Pljit::Pljit() : list_head(nullptr), allocator() {}

PljitFunctionHandle Pljit::registerFunction(std::string&& source_code) {
    // My original approach was to create a shared_ptr here, which was to my perception the better approach,
    // as it automatically handled reference counting and was able to free its resources independent of the Pljit class
    // (e.g. no danger of dandling pointers when Pljit was accidentally freed; and freeing of resources not used anymore as early as possible!).
    // However, the specification explicitly stated that we shall use the Pljit class to >store< the PljitFunctions.
    // Therefore, I revised my design, and the lifetime of a PljitFunction is now bound to the Pljit class.

    ListNode* node = new (allocator.allocate(1)) ListNode(std::make_unique<PljitFunction>(std::move(source_code)));

    std::atomic_ref head_ref{list_head};

    ListNode* head = head_ref.load();
    do {
        node->next = head;
    } while (!head_ref.compare_exchange_weak(head, node));

    return PljitFunctionHandle{ node->function.get() };
}
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------


