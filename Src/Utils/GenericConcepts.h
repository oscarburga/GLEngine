#pragma once

#include <concepts>

template<typename T, typename... U>
concept MatchesAnyType = (... || std::same_as<T, U>);

template<typename Func, typename... ArgTypes>
concept IsCallableWith = (... && std::invocable<Func, ArgTypes>);
