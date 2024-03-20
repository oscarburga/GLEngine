#pragma once


template <class T>
struct RefIgnore
{
    static inline T ignored_{};

    constexpr operator T&() const
    {
        return ignored_;
    }

    constexpr T* operator &() const
    {
        return &ignored_;
    }
};

template <class T>
constexpr RefIgnore<T> ref_ignore{};
