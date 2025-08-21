#pragma once

#include <algorithm>
#include <compare>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace mystd {

template<class T, std::size_t N>
struct array {
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

    T elems[N == 0 ? 1 : N];

    constexpr T& at(std::size_t index) {
        if (index >= N) throw std::out_of_range("array");
        return elems[index];
    }

    constexpr const T& at(std::size_t index) const {
        if (index >= N) throw std::out_of_range("array");
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

    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }

    constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

    constexpr bool empty() const noexcept { return N == 0; }
    constexpr std::size_t size() const noexcept { return N; }
    constexpr std::size_t max_size() const noexcept { return N; }
    constexpr void fill(const T& value) { for (T& elem : *this) elem = value; }

    constexpr void swap(array& other) noexcept(std::is_nothrow_swappable_v<T>) { for (std::size_t i = 0; i < N; ++i) std::swap(elems[i], other.elems[i]); }
};

template<class T, class... U>
array(T, U...) -> array<T, 1 + sizeof...(U)>;

template<class T>
constexpr std::size_t tuple_size_v = std::tuple_size<T>::value;

}

template<class T, std::size_t N>
constexpr bool operator==(const mystd::array<T, N>& lhs, const mystd::array<T, N>& rhs) { return std::equal(lhs.begin(), lhs.end(), rhs.begin()); }

template<class T, std::size_t N>
constexpr std::compare_three_way_result_t<T> operator<=>(const mystd::array<T, N>& lhs, const mystd::array<T, N>& rhs) { return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); }

template<std::size_t I, class T, std::size_t N>
constexpr T& get(mystd::array<T, N>& arr) noexcept { return arr[I]; }

template<std::size_t I, class T, std::size_t N>
constexpr T&& get(mystd::array<T, N>&& arr) noexcept { return std::move(arr[I]); }

template<std::size_t I, class T, std::size_t N>
constexpr const T& get(const mystd::array<T, N>& arr) noexcept { return arr[I]; }

template<std::size_t I, class T, std::size_t N>
constexpr const T&& get(const mystd::array<T, N>&& arr) noexcept { return std::move(arr[I]); }

template<class T, size_t N>
constexpr mystd::array<std::remove_cv_t<T>, N> to_array(T(&arr)[N]) { return [&arr]<std::size_t... I>(std::index_sequence<I...>) { return mystd::array<std::remove_cv_t<T>, N>{arr[I]...}; }(std::make_index_sequence<N>{}); }

template<class T, std::size_t N>
constexpr mystd::array<std::remove_cv_t<T>, N> to_array(T(&&arr)[N]) { return [&arr]<std::size_t... I>(std::index_sequence<I...>) { return mystd::array<std::remove_cv_t<T>, N>{std::move(arr[I])...}; }(std::make_index_sequence<N>{}); }


namespace std {

template<class T, std::size_t N>
constexpr void swap(mystd::array<T, N>& lhs, mystd::array<T, N>& rhs) noexcept(noexcept(lhs.swap(rhs))) { lhs.swap(rhs); }

template<class T, std::size_t N>
struct tuple_size<mystd::array<T, N>> : integral_constant<std::size_t, N> {};

template<std::size_t I, class T, std::size_t N>
struct tuple_element<I, mystd::array<T, N>> { using type = T; };

}

