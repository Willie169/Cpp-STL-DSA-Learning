#pragma once

#include <cstddef>

template<class T>
class Deque {
    T** map;
    std::size_t sz, sb, si, eb, ei;
    static const size_t __buf_size = sizeof(T) < 512 ? size_t(512 / sizeof(T)) : size_t(1);

public:
    constexpr deque() : map(nullptr), sz(0), sb(0), si(0), eb(0), ei(0);

    explicit constexpr deque(std::size_t count) {
        sz = (count + __buf_size - 1) / __buf_size + 2;
        map = new T*[sz];
	for (std::size_t i = 0; i < sz; ++i) map[i] = nullptr;
	sb = 1;
	eb = sz - 2;
	for (std::size_t i = sb; i <= eb; ++i) map[i] = new T[__buf_size];
	si = 0;
	ei = count % __buf_size;
	if (ei == 0 && count != 0) ei = __buf_size;
        for (std::size_t i = sb; i <= eb; ++i) {
	    std::size_t s = (i == sb) ? si : 0;
	    std::size_t e = (i == eb) ? ei : __buf_size;
	    for (T& elem : map[i]) new (&elem) T();
	}
    }

    constexpr deque(std::size_t count, const T& value) {
        sz = (count + __buf_size - 1) / __buf_size + 2;
        map = new T*[sz];
        for (std::size_t i = 0; i < sz; ++i) map[i] = nullptr;
        sb = 1;
        eb = sz - 2;
        for (std::size_t i = sb; i <= eb; ++i) map[i] = new T[__buf_size];
        si = 0;
        ei = count % __buf_size;
        if (ei == 0 && count != 0) ei = __buf_size;
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_size;
            for (T& elem : map[i]) new (&elem) T(value);
        }
    }
};
