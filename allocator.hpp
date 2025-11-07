#pragma once // allocator.hpp

#include <cstddef>
#include <limits>
#include <new>
#include <type_traits>
#include <memory>
#include <utility>

namespace mystd {

template< class T >
struct allocator {
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::true_type;

    constexpr allocator() noexcept = default;
    constexpr allocator(const allocator& other) noexcept = default;
    template<class U>
    constexpr allocator(const allocator<U>& other) noexcept {};
    constexpr ~allocator() = default;

    [[nodiscard]] constexpr T* allocate(std::size_t n) {
        if (std::numeric_limits<std::size_t>::max() / sizeof(T) < n) {
            throw std::bad_array_new_length{};
        }
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    constexpr void deallocate(T* p, std::size_t) noexcept {
        ::operator delete(p);
    }
};

template< class T1, class T2 >
constexpr bool operator==(const allocator<T1>& lhs, const allocator<T2>& rhs) noexcept { return true; }

template<typename Alloc, typename T, typename = void>
struct __rebind_helper { };

template<typename Alloc, typename T>
struct __rebind_helper<Alloc, T, std::void_t<typename Alloc::template rebind<T>::other>> {
    using type = typename Alloc::template rebind<T>::other;
};

template<template<class, class...> class Alloc, class U, class... Args, class T>
struct __rebind_helper<Alloc<U, Args...>, T> {
    using type = Alloc<T, Args...>;
};

template <typename Alloc>
struct allocator_traits {
    using allocator_type = Alloc;
    using value_type = typename Alloc::value_type;

private:
    template <typename A, typename = void> struct pointer_helper { using type = value_type*; };
    template <typename A> struct pointer_helper<A, std::void_t<typename A::pointer>> { using type = typename A::pointer; };

public:
    using pointer = typename pointer_helper<Alloc>::type;

private:
    template <typename A, typename = void> struct const_pointer_helper { using type = typename std::pointer_traits<pointer>::template rebind<const value_type>; };
    template <typename A> struct const_pointer_helper<A, std::void_t<typename A::const_pointer>> { using type = typename A::const_pointer; };

public:
    using const_pointer = typename const_pointer_helper<Alloc>::type;

private:
    template <typename A, typename = void> struct void_pointer_helper { using type = typename std::pointer_traits<pointer>::template rebind<void>; };
    template <typename A> struct void_pointer_helper<A, std::void_t<typename A::void_pointer>> { using type = typename A::void_pointer; };

public:
    using void_pointer = typename void_pointer_helper<Alloc>::type;

private:
    template <typename A, typename = void> struct const_void_pointer_helper { using type = typename std::pointer_traits<pointer>::template rebind<const void>; };
    template <typename A> struct const_void_pointer_helper<A, std::void_t<typename A::const_void_pointer>> { using type = typename A::const_void_pointer; };

public:
    using const_void_pointer = typename const_void_pointer_helper<Alloc>::type;

private:
    template <typename A, typename = void> struct difference_type_helper { using type = std::pointer_traits<pointer>::difference_type; };
    template <typename A> struct difference_type_helper<A, std::void_t<typename A::difference_type>> { using type = typename A::difference_type; };

public:
    using difference_type = typename difference_type_helper<Alloc>::type;

private:
    template <typename A, typename = void> struct size_type_helper { using type = std::make_unsigned<difference_type>::type; };
    template <typename A> struct size_type_helper<A, std::void_t<typename A::size_type>> { using type = typename A::size_type; };

public:
    using size_type = typename size_type_helper<Alloc>::type;

private:
    template <typename A, typename = void> struct propagate_on_container_copy_assignment_helper { using type = std::false_type; };
    template <typename A> struct propagate_on_container_copy_assignment_helper<A, std::void_t<typename A::propagate_on_container_copy_assignment>> { using type = typename A::propagate_on_container_copy_assignment; };

public:
    using propagate_on_container_copy_assignment = typename propagate_on_container_copy_assignment_helper<Alloc>::type;

private:
    template <typename A, typename = void> struct propagate_on_container_move_assignment_helper { using type = std::false_type; };
    template <typename A> struct propagate_on_container_move_assignment_helper<A, std::void_t<typename A::propagate_on_container_move_assignment>> { using type = typename A::propagate_on_container_move_assignment; };

public:
    using propagate_on_container_move_assignment = typename propagate_on_container_move_assignment_helper<Alloc>::type;

private:
    template <typename A, typename = void> struct propagate_on_container_swap_helper { using type = std::false_type; };
    template <typename A> struct propagate_on_container_swap_helper<A, std::void_t<typename A::propagate_on_container_swap>> { using type = typename A::propagate_on_container_swap; };

public:
    using propagate_on_container_swap = typename propagate_on_container_swap_helper<Alloc>::type;

private:
    template <typename A, typename = void> struct is_always_equal_helper { using type = std::is_empty<Alloc>::type; };
    template <typename A> struct is_always_equal_helper<A, std::void_t<typename A::is_always_equal>> { using type = typename A::is_always_equal; };

public:
    using is_always_equal = typename is_always_equal_helper<Alloc>::type;

    template <typename U>
    using rebind_alloc = typename __rebind_helper<Alloc, U>::type;

    template <typename U>
    using rebind_traits = allocator_traits<rebind_alloc<U>>;

    static constexpr pointer allocate(Alloc& a, size_type n) {
        return a.allocate(n);
    }

private:
    template <typename A>
    static constexpr auto test(int) -> decltype(
        std::declval<A&>().allocate(std::declval<size_type>(), std::declval<const_void_pointer>()),
        std::true_type{}
    );

    template <typename>
    static constexpr std::false_type test(...);

    static constexpr bool has_allocate_with_hint = decltype(test<Alloc>(0))::value;

public:
    static constexpr pointer allocate(Alloc& a, size_type n, const_void_pointer hint) {
        if constexpr (has_allocate_with_hint) {
            return a.allocate(n, hint);
        } else {
            (void)hint;
            return a.allocate(n);
        }
    }

    static constexpr void deallocate(Alloc& a, pointer p, size_type n) {
        a.deallocate(p, n);
    }

    static constexpr size_type max_size(const Alloc& a) noexcept {
        if constexpr (requires { a.max_size(); }) {
            return a.max_size();
        } else {
            return std::numeric_limits<size_type>::max() / sizeof(value_type);
        }
    }

private:
    template <typename A, typename T, typename... Args>
    static constexpr auto has_construct(int) -> decltype(std::declval<A>().construct(std::declval<T*>(), std::declval<Args>()...), std::true_type{});
    template <typename, typename, typename...> static constexpr std::false_type has_construct(...);

    template <typename A, typename T>
    static constexpr auto has_destroy(int) -> decltype(std::declval<A>().destroy(std::declval<T*>()), std::true_type{});
    template <typename, typename> static constexpr std::false_type has_destroy(...);

public:
    template <typename T, typename... Args>
    static constexpr void construct(Alloc& a, T* p, Args&&... args) {
        if constexpr (decltype(has_construct<Alloc, T, Args...>(0))::value) {
            a.construct(p, std::forward<Args>(args)...);
        } else {
            std::construct_at(p, std::forward<Args>(args)...);
        }
    }

    template <typename T>
    static constexpr void destroy(Alloc& a, T* p) {
        if constexpr (decltype(has_destroy<Alloc, T>(0))::value) {
            a.destroy(p);
        } else {
            std::destroy_at(p);
        }
    }

    static constexpr Alloc select_on_container_copy_construction(const Alloc& a) {
        if constexpr (requires { a.select_on_container_copy_construction(); }) {
            return a.select_on_container_copy_construction();
        } else {
            return a;
        }
    }
};

} // namespace mystd

