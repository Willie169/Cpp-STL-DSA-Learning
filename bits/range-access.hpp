#pragma once // range-access.hpp

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <type_traits>

namespace mystd {

template< class C >
constexpr auto begin( const C& c ) -> decltype(c.begin()) { return c.begin(); }

template< class C >
constexpr auto begin( C& c ) -> decltype(c.begin()) { return c.begin(); }

template< class T, std::size_t N >
constexpr T* begin( T (&array)[N] ) noexcept { return array; }

template< class C >
constexpr auto cbegin( const C& c ) noexcept(noexcept(std::begin(c))) -> decltype(std::begin(c)) { return c.begin(); }

template< class C >
constexpr auto end( const C& c ) -> decltype(c.end()) { return c.end(); }

template< class C >
constexpr auto end( C& c ) -> decltype(c.end()) { return c.end(); }

template< class T, std::size_t N >
constexpr T* end( T (&array)[N] ) noexcept { return array + N; }

template< class C >
constexpr auto cend( const C& c ) noexcept(noexcept(std::end(c))) -> decltype(std::end(c)) { return c.end(); }

template< class C >
constexpr auto rbegin( C& c ) -> decltype(c.rbegin()) { return c.rbegin(); }

template< class C >
constexpr auto rbegin( const C& c ) -> decltype(c.rbegin()) { return c.rbegin(); }

template< class T, std::size_t N >
constexpr std::reverse_iterator<T*> rbegin( T (&array)[N] ) { return std::reverse_iterator<T*>(array + N); }

template< class T >
constexpr std::reverse_iterator<const T*> rbegin( std::initializer_list<T> il ) { return std::reverse_iterator<const T*>(il.end()); }

template< class C >
constexpr auto crbegin( const C& c ) -> decltype(std::rbegin(c)) { return c.rbegin(); }

template< class C >
constexpr auto rend( C& c ) -> decltype(c.rend()) { return c.rend(); }

template< class C >
constexpr auto rend( const C& c ) -> decltype(c.rend()) { return c.rend(); }

template< class T, std::size_t N >
constexpr std::reverse_iterator<T*> rend( T (&array)[N] ) { return std::reverse_iterator<T*>(array); }

template< class T >
constexpr std::reverse_iterator<const T*> rend( std::initializer_list<T> il ) { return std::reverse_iterator<const T*>(il.begin()); }

template< class C >
constexpr auto crend( const C& c ) -> decltype(std::rend(c)) { return c.rend(); }

template< class C >
constexpr auto size( const C& c ) -> decltype(c.size()) { return c.size(); }

template< class C >
constexpr auto ssize( const C& c ) -> std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(c.size())>> { return static_cast<std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(c.size())>>>(c.size()); }

template< class T, std::size_t N >
constexpr std::size_t size( const T (&array)[N] ) noexcept { return N; }

template< class T, std::ptrdiff_t N >
constexpr std::ptrdiff_t ssize( const T (&array)[N] ) noexcept { return N; }

template< class C >
constexpr auto empty( const C& c ) -> decltype(c.empty()) { return c.empty(); }

template< class T, std::size_t N >
constexpr bool empty( const T (&array)[N] ) noexcept { return false; }

template< class E >
constexpr bool empty( std::initializer_list<E> il ) noexcept { return il.size() == 0; }

template< class C >
constexpr auto data( C& c ) -> decltype(c.data()) { return c.data(); }

template< class C >
constexpr auto data( const C& c ) -> decltype(c.data()) { return c.data(); }

template< class T, std::size_t N >
constexpr T* data( T (&array)[N] ) noexcept { return array; }

template< class E >
constexpr const E* data( std::initializer_list<E> il ) noexcept { return il.begin(); }

} // namespace mystd
