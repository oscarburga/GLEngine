#pragma once

#include <vector>
#include <type_traits>

template<class Callable>
class [[nodiscard]] Defer final
{
	static_assert(std::is_invocable_v<Callable>, "Template type must be callable");
	Defer() = delete;
	Defer(const Defer&) = delete;
	Defer& operator=(const Defer&) = delete;
	Callable Func;

public:
	Defer(Callable&& Func) : Func(Func) {}
	~Defer() { Func(); }
};
