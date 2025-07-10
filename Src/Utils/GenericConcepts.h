#pragma once

#include <concepts>

template<typename T, typename... U>
concept MatchesAnyType = (... || std::same_as<T, U>);

template<typename Func, typename... ArgTypes>
concept IsCallableWith = (... && std::invocable<Func, ArgTypes>);

template<class T> 
concept NumericType = std::is_arithmetic_v<T>;

template<class T>
concept StringLike = std::is_convertible_v<T, std::string_view>;
