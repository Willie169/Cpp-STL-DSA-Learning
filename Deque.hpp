#pragma once

#include <cstddef>

template<class T>
class Deque {
    T** map;
    std::size_t sz, sb, si, eb, ei;
    static const size_t __deque_buf_size =  sizeof(T) < 512 ? size_t(512 / sizeof(T)) : size_t(1);

public:
    constexpr deque() : map(nullptr), sz(0), sb(0), si(0), eb(0), ei(0);

    explicit constexpr deque(std::size_t count) {
        std::size_t blocks_needed = (count + block_size - 1) / block_size; 
