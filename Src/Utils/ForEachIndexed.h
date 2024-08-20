#pragma once


namespace util
{
	template <typename IteratorT, typename IntegerT, typename FunctionT>
	FunctionT for_each_indexed(IteratorT first, IteratorT last, IntegerT initial, FunctionT func)
	{
		for (;first != last; ++first, ++initial)
			func(initial, *first);
		return func;
	}
}
