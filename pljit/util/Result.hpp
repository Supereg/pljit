//
// Created by Andreas Bauer on 29.07.22.
//

#ifndef PLJIT_RESULT_HPP
#define PLJIT_RESULT_HPP

#include "../code/SourceCode.hpp"
#include <cassert>
#include <concepts>
#include <optional>

namespace pljit {
//---------------------------------------------------------------------------
/**
 * Result type describing some result which might fail with an `SourceCodeError`.
 * @tparam T - Some result type.
 */
template <typename T>
class Result {
    std::optional<code::SourceCodeError> source_error;
    T result_content;

    public:
    Result() requires std::default_initializable<T>;
    Result(T result) requires std::move_constructible<T>; // NOLINT(google-explicit-constructor)
    Result(code::SourceCodeError error) requires std::default_initializable<T>; // NOLINT(google-explicit-constructor)

    /**
     * @return Returns the value of the Result (only if a value is present).
     */
    const T& value() const;
    /**
     * @return Releases the value of the Result (only if a value is present).
     */
    T&& release() requires std::movable<T>;
    /**
     * @return Returns the error of the Result (only if an error is present).
     */
    code::SourceCodeError error() const;

    /**
     * Implicit conversion to bool. Returns true if the Result has a value.
     */
    operator bool() const; // NOLINT(google-explicit-constructor)

    bool isSuccess() const;
    bool isFailure() const;

    const T& operator*() const;
    const T* operator->() const;
};
template <typename T>
Result<T>::Result() requires std::default_initializable<T> : source_error(), result_content() {}

template <typename T>
Result<T>::Result(T result) requires std::move_constructible<T> : source_error(), result_content(std::move(result)) {}

template <typename T>
Result<T>::Result(code::SourceCodeError error) requires std::default_initializable<T> : source_error(std::move(error)), result_content() {}

template <typename T>
const T& Result<T>::value() const {
    assert(!source_error.has_value() && "Result: result not present. SourceCode error occurred!");
    return result_content;
}

template <typename T>
T&& Result<T>::release() requires std::movable<T> {
    assert(!source_error.has_value() && "Result: result not present. SourceCode error occurred!");
    return std::move(result_content);
}

template <typename T>
code::SourceCodeError Result<T>::error() const {
    assert(source_error.has_value() && "LexerResult: tried accessing non-existent error!");
    return *source_error;
}

template <typename T>
Result<T>::operator bool() const {
    return !source_error.has_value();
}

template <typename T>
bool Result<T>::isSuccess() const {
    return !source_error.has_value();
}

template <typename T>
bool Result<T>::isFailure() const {
    return source_error.has_value();
}

template <typename T>
const T& Result<T>::operator*() const {
    return value();
}

template <typename T>
const T* Result<T>::operator->() const {
    return &value();
}
//---------------------------------------------------------------------------
} // namespace pljit
#endif //PLJIT_RESULT_HPP
