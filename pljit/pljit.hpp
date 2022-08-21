//
// Created by Andreas Bauer on 12.08.22.
//

#ifndef PLJIT_PLJIT_HPP
#define PLJIT_PLJIT_HPP

#include "./util/Result.hpp"
#include <string>
#include <memory>
#include <initializer_list>
#include <gtest/gtest_prod.h>

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

    /**
     * A call to this function will evaluate the function.
     * The function is compiled before execution if it wasn't compiled yet.
     * @tparam T The parameter pack of `long long`s.
     * @param arguments The variadic arguments passed to the function.
     * @return Returns the value of the function evaluation. The optional might be empty if
     * either a compilation error or a runtime error occurred.
     * You can use the `compilation_error()` getter to get access to the compilation error.
     * Runtime errors are printed to standard out.
     */
    template <typename... T>
    std::optional<long long> operator()(T... arguments) const;
    /**
     * A call to this function will evaluate the function.
     * The function is compiled before execution if it wasn't compiled yet.
     * @param argument_list A list of arguments passed to the function.
     * @return Returns the value of the function evaluation. The optional might be empty if
     * either a compilation error or a runtime error occurred.
     * You can use the `compilation_error()` getter to get access to the compilation error.
     * Runtime errors are printed to standard out.
     */
    std::optional<long long> operator()(std::initializer_list<long long> argument_list) const;

    /**
     * @return Returns the compilation error, if one occurred.
     */
    std::optional<code::SourceCodeError> compilation_error() const;
};

template <typename... T>
std::optional<long long> PljitFunctionHandle::operator()(T... arguments) const {
    return operator()({arguments...});
}
//---------------------------------------------------------------------------
/**
 * Interface for the JIT compiler.
 * It stores are constructed functions.
 */
class Pljit {
    FRIEND_TEST(Pljit, testMultiThreadedRegistration);
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

    /**
     * Registers a new function for the given source code. The code will
     * be compiled just-in-time once required.
     * The lifetime of the returned handle is bound to the lifetime of the Pljit object.
     * @param source_code The source code of the function.
     * @return Returns a easy to copy/move handle to a PljitFunction.
     */
    PljitFunctionHandle registerFunction(std::string&& source_code);
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_PLJIT_HPP
