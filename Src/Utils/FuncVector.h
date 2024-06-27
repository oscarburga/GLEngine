#pragma once

#include <vector>
#include <functional>
#include <type_traits>

template<class Callable, class... Args>
class TFuncVectorImpl : public std::vector<Callable>
{
	static_assert(std::is_invocable_v<Callable, Args...>, "Template type must be callable with Args");
	using Vector = std::vector<Callable>;
public:
	// Return ref to self so you can call vector functions after i.e. ExecuteQueue(args).clear();
	TFuncVectorImpl& ExecuteQueue(Args&&... args)
	{
		ExecuteInternal(Vector::begin(), Vector::end(), std::forward(args)...);
		return *this;
	}

	// Return ref to self so you can call vector functions after i.e. ExecuteStack(args).clear();
	TFuncVectorImpl& ExecuteStack(Args&&... args)
	{
		ExecuteInternal(Vector::rbegin(), Vector::rend(), std::forward(args)...);
		return *this;
	}
private:
	template<typename It>
	void ExecuteInternal(It begin, It end, Args&&... args)
	{
		while (begin != end)
		{
			std::apply(*begin, std::forward_as_tuple(args...));
			++begin;
		}
	}

};

template<class... Args>
using TFuncVector = TFuncVectorImpl<std::function<void(Args...)>, Args...>;
using FuncVector = TFuncVector<>;

// template<class... Args>
// using TFpVector = TFuncVectorImpl<void(*)(Args...), Args...>;
