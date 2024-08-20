#pragma once

namespace util
{
	template <class T>
	struct RefIgnore
	{
		static inline T I{};
	};
}
