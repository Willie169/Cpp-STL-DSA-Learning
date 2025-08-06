#pragma once

#include <algorithm>
#include <compare>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

template<class T, std::size_t N>
struct Array {
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;


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
    constexpr void fill(const T& value) { for (T& elem : *this) elem = value; }


    constexpr void swap(Array& other) noexcept(std::is_nothrow_swappable_v<T>) { for (std::size_t i = 0; i < N; ++i) std::swap(elems[i], other.elems[i]); }
};


template<class T, std::size_t N>
constexpr bool operator==(const Array<T, N>& lhs, const Array<T, N>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

// Non-member operator<=> (C++20 three-way comparison)
template<class T, std::size_t N>
constexpr std::compare_three_way_result_t<T> operator<=>(const Array<T, N>& lhs, const Array<T, N>& rhs) {
    return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}


template<class T, std::size_t N>
constexpr void swap(Array<T, N>& lhs, Array<T, N>& rhs) noexcept(noexcept(lhs.swap(rhs))) { lhs.swap(rhs); }


template<class T, std::size_t N>
constexpr Array<std::remove_cv_t<T>, N> to_array(T (&arr)[N]) { return [&arr]<std::size_t... I>(std::index_sequence<I...>) { return Array<std::remove_cv_t<T>, N>{arr[I]...}; }(std::make_index_sequence<N>{}); }


template<class T, std::size_t N>
constexpr Array<std::remove_cv_t<T>, N> to_array(T (&&arr)[N]) { return [&arr]<std::size_t... I>(std::index_sequence<I...>) { return Array<std::remove_cv_t<T>, N>{std::move(arr[I])...}; }(std::make_index_sequence<N>{}); }


template<class T, std::size_t N>
struct std::tuple_size<Array<T, N>> : std::integral_constant<std::size_t, N> {};


template<std::size_t I, class T, std::size_t N>
struct std::tuple_element<I, Array<T, N>> {
    static_assert(I < N, "Index out of bounds for Array");
    using type = T;
};


template<std::size_t I, class T, std::size_t N>
constexpr T& get(Array<T, N>& arr) noexcept {
    static_assert(I < N, "Index out of bounds for Array");
    return arr[I];
}


template<std::size_t I, class T, std::size_t N>
constexpr T&& get(Array<T, N>&& arr) noexcept {
    static_assert(I < N, "Index out of bounds for Array");
    return std::move(arr[I]);
}


template<std::size_t I, class T, std::size_t N>
constexpr const T& get(const Array<T, N>& arr) noexcept {
    static_assert(I < N, "Index out of bounds for Array");
    return arr[I];
}


template<std::size_t I, class T, std::size_t N>
constexpr const T&& get(const Array<T, N>&& arr) noexcept {
    static_assert(I < N, "Index out of bounds for Array");
    return std::move(arr[I]);
}
