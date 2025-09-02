#include <cstddef>
#include <type_traits>
#include <memory>
#include <utility>
#include <limits>

namespace mystd {

template <typename Alloc>
struct allocator_traits {
    using allocator_type = Alloc;
    using value_type = typename Alloc::value_type;

private:
    template <typename A, typename = void> struct pointer_helper { using type = value_type*; };
    template <typename A> struct pointer_helper<A, std::void_t<typename A::pointer>> { using type = typename A::pointer; };



public:
    using pointer = typename pointer_helper<Alloc>::type;
    using const_pointer = typename const_pointer_helper<Alloc>::type;
    using void_pointer = std::conditional_t<std::is_void_v<value_type>, pointer, void*>;
    using const_void_pointer = std::conditional_t<std::is_void_v<value_type>, const_pointer, const void*>;
    using size_type = typename Alloc::size_type;
    using difference_type = typename Alloc::difference_type;

    template <typename U>
    using rebind_alloc = typename Alloc::template rebind<U>::other;

    template <typename U>
    using rebind_traits = allocator_traits<rebind_alloc<U>>;

    // --------------------
    // Allocation / Deallocation
    // --------------------
    static pointer allocate(Alloc& a, size_type n) {
        return a.allocate(n);
    }

    static void deallocate(Alloc& a, pointer p, size_type n) {
        a.deallocate(p, n);
    }

    static size_type max_size(const Alloc& a) noexcept {
        if constexpr (requires { a.max_size(); }) {
            return a.max_size();
        } else {
            return std::numeric_limits<size_type>::max() / sizeof(value_type);
        }
    }

    // --------------------
    // Construct / Destroy with fallbacks
    // --------------------
private:
    template <typename A, typename T, typename... Args>
    static auto has_construct(int) -> decltype(std::declval<A>().construct(std::declval<T*>(), std::declval<Args>()...), std::true_type{});
    template <typename, typename, typename...> static std::false_type has_construct(...);

    template <typename A, typename T>
    static auto has_destroy(int) -> decltype(std::declval<A>().destroy(std::declval<T*>()), std::true_type{});
    template <typename, typename> static std::false_type has_destroy(...);

public:
    template <typename T, typename... Args>
    static void construct(Alloc& a, T* p, Args&&... args) {
        if constexpr (decltype(has_construct<Alloc, T, Args...>(0))::value) {
            a.construct(p, std::forward<Args>(args)...);
        } else {
            ::new ((void*)p) T(std::forward<Args>(args)...);  // fallback
        }
    }

    template <typename T>
    static void destroy(Alloc& a, T* p) {
        if constexpr (decltype(has_destroy<Alloc, T>(0))::value) {
            a.destroy(p);
        } else {
            p->~T();  // fallback
        }
    }

    // --------------------
    // Propagation traits (for containers)
    // --------------------
    using propagate_on_container_copy_assignment = typename Alloc::propagate_on_container_copy_assignment;
    using propagate_on_container_move_assignment = typename Alloc::propagate_on_container_move_assignment;
    using propagate_on_container_swap = typename Alloc::propagate_on_container_swap;
    using is_always_equal = typename Alloc::is_always_equal;

    // --------------------
    // select_on_container_copy_construction
    // --------------------
    static Alloc select_on_container_copy_construction(const Alloc& a) {
        if constexpr (requires { a.select_on_container_copy_construction(); }) {
            return a.select_on_container_copy_construction();
        } else {
            return a;
        }
    }
};

};

