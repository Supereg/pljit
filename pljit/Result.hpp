//
// Created by Andreas Bauer on 29.07.22.
//

#ifndef PLJIT_RESULT_HPP
#define PLJIT_RESULT_HPP

#include "code/SourceCodeManagement.hpp"
#include <cassert>
#include <concepts>
#include <optional>

namespace pljit {
//---------------------------------------------------------------------------
template <typename T>
class Result {
    std::optional<code::SourceCodeError> source_error;
    T result_content;

    public:
    Result() requires std::default_initializable<T>;
    Result(T result) requires std::move_constructible<T>;
    Result(code::SourceCodeError error) requires std::default_initializable<T>;

    const T& value() const;

    T&& release() requires std::movable<T>;

    code::SourceCodeError error() const;

    // TODO prepend with "is" keyword! Both!
    bool success() const;

    bool failure() const;

    bool operator==(const Result& rhs) const requires std::equality_comparable<T>;

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
bool Result<T>::success() const {
    return !source_error.has_value();
}

template <typename T>
bool Result<T>::failure() const {
    return source_error.has_value();
}

template <typename T>
bool Result<T>::operator==(const Result& rhs) const requires std::equality_comparable<T> {
    return source_error == rhs.source_error &&
        result_content == rhs.result_content;
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
