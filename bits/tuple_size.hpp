#pragma once // tuple_size.hpp

#include <cstddef>
#include <type_traits>

namespace mystd {

template< class T >
struct tuple_size;

template< class T >
struct tuple_size< const T > : std::integral_constant<std::size_t, mystd::tuple_size<T>::value> {};

template< class T >
constexpr std::size_t tuple_size_v = tuple_size<T>::value;

} // namspace mystd
