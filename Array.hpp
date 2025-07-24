#pragma once

#include <cstddef>
#include <stdexcept>
#include <iterator>
#include <algorithm>
#include <utility>

template<class T, std::size_t N>
class Array {
private:
    T elems[N == 0 ? 1 : N];

public:
    constexpr T& operator[](std::size_t index) { return elems[index]; }
    constexpr const T& operator[](std::size_t index) const { return elems[index]; }

    constexpr T& at(std::size_t index) {
        if (index >= N) throw std::out_of_range("Array");
        return elems[index];
    }

    constexpr const T& at(std::size_t index) const {
        if (index >= N) throw std::out_of_range("Array");
        return elems[index];
    }

    constexpr T& front() { return elems[0]; }
    constexpr const T& front() const { return elems[0]; }

    constexpr T& back() { return elems[N - 1]; }
    constexpr const T& back() const { return elems[N - 1]; }

    constexpr void swap(Array& other) noexcept(std::is_nothrow_swappable_v<T>) {
        for (std::size_t i = 0; i < N; ++i)
            std::swap(elems[i], other.elems[i]);
    }

    constexpr T* data() noexcept { return elems; }
    constexpr const T* data() const noexcept { return elems; }

    constexpr T* begin() noexcept { return elems; }
    constexpr const T* begin() const noexcept { return elems; }

    constexpr T* end() noexcept { return elems + N; }
    constexpr const T* end() const noexcept { return elems + N; }

    constexpr auto rbegin() noexcept { return std::reverse_iterator(end()); }
    constexpr auto rbegin() const noexcept { return std::reverse_iterator(end()); }
    
    constexpr auto rend() noexcept { return std::reverse_iterator(begin()); }
    constexpr auto rend() const noexcept { return std::reverse_iterator(begin()); }

    constexpr std::size_t size() const noexcept { return N; }
    constexpr std::size_t max_size() const noexcept { return N; }
    constexpr bool empty() const noexcept { return N == 0; }
};

