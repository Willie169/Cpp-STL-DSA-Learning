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
    constexpr T& at(std::size_t index) {
        if (index >= N) throw std::out_of_range("Array");
        return elems[index];
    }

    constexpr const T& at(std::size_t index) const {
        if (index >= N) throw std::out_of_range("Array");
        return elems[index];
    }

    constexpr T& operator[](std::size_t index) { return elems[index]; }
    constexpr const T& operator[](std::size_t index) const { return elems[index]; }

    constexpr T& front() { return elems[0]; }
    constexpr const T& front() const { return elems[0]; }

    constexpr T& back() { return elems[N - 1]; }
    constexpr const T& back() const { return elems[N - 1]; }

    constexpr T* data() noexcept { return elems; }
    constexpr const T* data() const noexcept { return elems; }

    constexpr T* begin() noexcept { return elems; }
    constexpr const T* begin() const noexcept { return elems; }
    constexpr const T* cbegin() const noexcept { return elems; }

    constexpr T* end() noexcept { return elems + N; }
    constexpr const T* end() const noexcept { return elems + N; }
    constexpr const T* cend() const noexcept { return elems + N; }

    constexpr std::reverse_iterator<T*> rbegin() noexcept { return std::reverse_iterator<T*>(end()); }
    constexpr std::reverse_iterator<const T*> rbegin() const noexcept { return std::reverse_iterator<const T*>(end()); }
    constexpr std::reverse_iterator<const T*> crbegin() const noexcept { return std::reverse_iterator<const T*>(end()); }
    
    constexpr std::reverse_iterator<T*> rend() noexcept { return std::reverse_iterator<T*>(begin()); }
    constexpr std::reverse_iterator<const T*> rend() const noexcept { return std::reverse_iterator<const T*>(begin()); }
    constexpr std::reverse_iterator<const T*> crend() const noexcept { return std::reverse_iterator<const T*>(begin()); }

    constexpr bool empty() const noexcept { return N == 0; }
    constexpr std::size_t size() const noexcept { return N; }
    constexpr std::size_t max_size() const noexcept { return N; }

    constexpr void swap(Array& other) noexcept(std::is_nothrow_swappable_v<T>) { for (std::size_t i = 0; i < N; ++i) std::swap(elems[i], other.elems[i]); }
};

