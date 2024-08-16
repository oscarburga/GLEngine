#pragma once

#include <vector>
#include <algorithm>
#include "Utils/ConceptsDef.h"

template<typename... Types>
class MultiTypeContainer
{
public:

	template<MatchesAnyType<Types...> T>
	inline T& Get(size_t index)
	{
		return std::get<std::vector<T>>(Data).at(index);
	}

	template<MatchesAnyType<Types...> T>
	inline T GetCopy(size_t index)
	{
		return std::get<std::vector<T>>(Data).at(index);
	}

	template<MatchesAnyType<Types...> T>
	inline std::vector<T>& Get()
	{
		return std::get<std::vector<T>>(Data);
	}

	template<MatchesAnyType<Types...> T>
	inline std::vector<T> GetCopy()
	{
		return std::get<std::vector<T>>(Data);
	}

	// Unsure yet on the best approach to handling moving vectors out of Data
	// template<MatchesAnyType<Types...> T>
	// T&& GetMove()
	// {
	// 	return std::move(std::get<std::vector<T>>(Data));
	// }

	template<MatchesAnyType<Types...> T>
	inline void push_back(T&& value)
	{
		std::get<std::vector<T>>(Data).push_back(value);
	} 

	template<MatchesAnyType<Types...> T>
	inline T& emplace_back(T&& value)
	{
		return std::get<std::vector<T>>(Data).emplace_back(value);
	}

	template<MatchesAnyType<Types...> T, typename... Args>
	inline T& emplace_back(Args&&... args)
	{
		return std::get<std::vector<T>>(Data).emplace_back(args...);
	}

	template<IsCallableWith<Types&...> F>
	inline void apply(F&& f)
	{
		std::apply([&](auto&... vectors)
		{
			(std::for_each(vectors.begin(), vectors.end(), f), ...);
		}, Data);
	}

	template<IsCallableWith<std::vector<Types>&...> F>
	inline void applyVector(F&& f)
	{
		std::apply([&](auto&... vectors)
		{
			(f(vectors), ...);
		}, Data);
	}

private:
	std::tuple<std::vector<Types>...> Data;
};

#define DeclareConceptContainer(ClassName, Concept) \
template <Concept... Types> class ClassName : public MultiTypeContainer<Types...> {};
