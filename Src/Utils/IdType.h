#pragma once

#include <concepts>
#include <limits>

template<std::unsigned_integral IntType, typename Tag, IntType NullValue = std::numeric_limits<IntType>::max()>
struct TId
{
	static constexpr IntType NullId = NullValue;
	TId() : Id(NullId) {}
	explicit TId(IntType id) : Id(id) {}

	inline void Reset() { Id = NullId; }
	inline IntType& operator *() { return Id; }
	inline bool IsNull() const { return Id == NullId; }
	inline operator IntType() const { return Id; }
	inline operator bool() const { return !IsNull(); }
	inline TId& operator =(IntType id) { Id = id; return *this; }
	IntType Id;

	bool operator<(const TId& other) const { return Id < other.Id; }
	bool operator==(const TId& other) const { return Id == other.Id; }
	bool operator>(const TId& other) const { return Id > other.Id; }
};
