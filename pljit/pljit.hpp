//
// Created by Andreas Bauer on 12.08.22.
//

#ifndef PLJIT_PLJIT_HPP
#define PLJIT_PLJIT_HPP

#include "./util/Result.hpp"
#include <string>
#include <memory>
#include <initializer_list>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
class Pljit;
class PljitFunction;
//---------------------------------------------------------------------------
class PljitFunctionHandle {
    friend class Pljit;

    PljitFunction* function;

    explicit PljitFunctionHandle(PljitFunction* function);
    public:

    // Copy constructor
    PljitFunctionHandle(const PljitFunctionHandle& other) = default;
    // Move constructor
    PljitFunctionHandle(PljitFunctionHandle&& other) noexcept = default;
    // Copy assignment
    PljitFunctionHandle& operator=(const PljitFunctionHandle& other) = default;
    // Move assignment
    PljitFunctionHandle& operator=(PljitFunctionHandle&& other) noexcept = default;

    // TODO use a different result type for runtime error messages?
    template <typename... T>
    Result<long long> operator()(T... arguments) const;
    Result<long long> operator()(std::initializer_list<long long> argument_list) const;
};

template <typename... T>
Result<long long> PljitFunctionHandle::operator()(T... arguments) const {
    return operator()({arguments...});
}
//---------------------------------------------------------------------------
class Pljit {
    friend class PljitFunctionHandle;

    class ListNode {
        public:
        std::unique_ptr<PljitFunction> function;
        ListNode* next;

        explicit ListNode(std::unique_ptr<PljitFunction> function);
    };

    ListNode* list_head;
    std::allocator<ListNode> allocator;

    public:
    Pljit();
    ~Pljit();

    PljitFunctionHandle registerFunction(std::string&& source_code);
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_PLJIT_HPP
