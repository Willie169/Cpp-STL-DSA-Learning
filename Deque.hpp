#pragma once

#include <cstddef>

template<class T>
class Deque {
    T** map;
    std::size_t sz, sb, si, eb, ei;
    constexpr size_t __deque_buf_size(size_t sz) {
        return sz < 512 ? size_t(512 / sz) : size_t(1);
    }

public:
    constexpr deque() : map(nullptr), sz(0), sb(0), si(0), eb(0), ei(0);

    explicit constexpr deque(std::size_t count) : 
