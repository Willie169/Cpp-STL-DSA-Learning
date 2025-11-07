#pragma once // algorithm-heap.hpp

#include <concepts>
#include <execution>
#include <functional>
#include <iterator>
#include <type_traits>

namespace mystd {

template< std::random_access_iterator It >
requires std::is_swappable_v<It> && std::is_move_constructible_v<It> && std::is_move_assignable_v<It>
constexpr void push_heap( It first, It last ) {
    using diff_t = typename std::iterator_traits<It>::difference_type;
    diff_t index = ( last - first ) - 1;
    while ( index > 0 ) {
        diff_t parent = ( index - 1 ) / 2;
        if ( std::less<>{}( *( first + parent ), *( first + index ) ) ) {
            std::iter_swap( first + parent, first + index );
            index = parent;
        } else {
            break;
        }
    }
}

template< std::random_access_iterator It, class Comp >
requires std::predicate< Comp, const typename std::iterator_traits<It>::reference, const typename std::iterator_traits<It>::reference >
constexpr void push_heap( It first, It last, Comp comp ) {
    using diff_t = typename std::iterator_traits<It>::difference_type;
    diff_t index = ( last - first ) - 1;
    while ( index > 0 ) {
        diff_t parent = ( index - 1 ) / 2;
        if ( comp( *( first + parent ), *( first + index ) ) ) {
            std::iter_swap( first + parent, first + index );
            index = parent;
        } else {
            break;
        }
    }
}

template< std::random_access_iterator It >
requires std::is_swappable_v<It> && std::is_move_constructible_v<It> && std::is_move_assignable_v<It>
constexpr void pop_heap( It first, It last ) {
    using diff_t = typename std::iterator_traits<It>::difference_type;
    if ( last - first <= 1 ) {
        return;
    }
    std::iter_swap( first, last - 1 );
    diff_t size = ( last - first ) - 1;
    diff_t index = 0;
    while ( true ) {
        diff_t left = 2 * index + 1;
        diff_t right = 2 * index + 2;
        diff_t largest = index;
        if ( left < size && std::less<>{}( *( first + largest ), *( first + left ) ) ) {
            largest = left;
        }
        if ( right < size && std::less<>{}( *( first + largest ), *( first + right ) ) ) {
            largest = right;
        }
        if ( largest != index ) {
            std::iter_swap( first + index, first + largest );
            index = largest;
        } else {
            break;
        }
    }
}

template< std::random_access_iterator It, class Comp >
requires std::predicate< Comp, const typename std::iterator_traits<It>::reference, const typename std::iterator_traits<It>::reference >
constexpr void pop_heap( It first, It last, Comp comp ) {
    using diff_t = typename std::iterator_traits<It>::difference_type;
    if ( last - first <= 1 ) {
        return;
    }
    std::iter_swap( first, last - 1 );
    diff_t size = ( last - first ) - 1;
    diff_t index = 0;
    while ( true ) {
        diff_t left = 2 * index + 1;
        diff_t right = 2 * index + 2;
        diff_t largest = index;
        if ( left < size && comp( *( first + largest ), *( first + left ) ) ) {
            largest = left;
        }
        if ( right < size && comp( *( first + largest ), *( first + right ) ) ) {
            largest = right;
        }
        if ( largest != index ) {
            std::iter_swap( first + index, first + largest );
            index = largest;
        } else {
            break;
        }
    }
}

template< std::random_access_iterator It >
requires std::is_swappable_v<It> && std::is_move_constructible_v<It> && std::is_move_assignable_v<It>
constexpr void make_heap( It first, It last ) {
    using diff_t = typename std::iterator_traits<It>::difference_type;
    std::function<void(It, It, diff_t)> heapify = [&heapify]( It start, It end, diff_t root ) {
        diff_t size = end - start;
        diff_t largest = root;
        diff_t left = 2 * root + 1;
        diff_t right = 2 * root + 2;
        if ( left < size && std::less<>{}( *( start + largest ), *( start + left ) ) ) {
            largest = left;
        }
        if ( right < size && std::less<>{}( *( start + largest ), *( start + right ) ) ) {
            largest = right;
        }
        if ( largest != root ) {
            std::iter_swap( start + root, start + largest );
            heapify( start, end, largest );
        }
    };
    diff_t size = last - first;
    for ( diff_t i = size / 2 - 1; i-- > 0; ) {
        heapify( first, last, i );
    }
}

template< std::random_access_iterator It, class Comp >
requires std::predicate< Comp, const typename std::iterator_traits<It>::reference, const typename std::iterator_traits<It>::reference >
constexpr void make_heap( It first, It last, Comp comp ) {
    using diff_t = typename std::iterator_traits<It>::difference_type;
    std::function<void(It, It, diff_t)> heapify = [&heapify, &comp]( It start, It end, diff_t root ) {
        diff_t size = end - start;
        diff_t largest = root;
        diff_t left = 2 * root + 1;
        diff_t right = 2 * root + 2;
        if ( left < size && comp( *( start + largest ), *( start + left ) ) ) {
            largest = left;
        }
        if ( right < size && comp( *( start + largest ), *( start + right ) ) ) {
            largest = right;
        }
        if ( largest != root ) {
            std::iter_swap( start + root, start + largest );
            heapify( start, end, largest );
        }
    };
    diff_t size = last - first;
    for ( diff_t i = size / 2 - 1; i-- > 0; ) {
        heapify( first, last, i );
    }
}

template< std::random_access_iterator It >
constexpr void sort_heap( It first, It last ) {
    while ( last - first > 1 ) {
        pop_heap( first, last );
        --last;
    }
}

template< std::random_access_iterator It, class Comp >
constexpr void sort_heap( It first, It last, Comp comp ) {
    while ( last - first > 1 ) {
        pop_heap( first, last, comp );
        --last;
    }
}

template< std::random_access_iterator It >
constexpr bool is_heap( It first, It last ) {
    using diff_t = typename std::iterator_traits<It>::difference_type;
    diff_t size = last - first;
    for ( diff_t i = 0; i <= ( size - 2 ) / 2; ++i ) {
        diff_t left = 2 * i + 1;
        diff_t right = 2 * i + 2;
        if ( left < size && std::less<>{}( *( first + i ), *( first + left ) ) ) {
            return false;
        }
        if ( right < size && std::less<>{}( *( first + i ), *( first + right ) ) ) {
            return false;
        }
    }
    return true;
}

template< class ExecutionPolicy, std::random_access_iterator It >
requires std::is_execution_policy_v<std::remove_cvref_t<ExecutionPolicy>>
bool is_heap( ExecutionPolicy&& policy, It first, It last ) {
    return is_heap( first, last );
}

template< std::random_access_iterator It, class Comp >
constexpr bool is_heap( It first, It last, Comp comp ) {
    using diff_t = typename std::iterator_traits<It>::difference_type;
    diff_t size = last - first;
    for ( diff_t i = 0; i <= ( size - 2 ) / 2; ++i ) {
        diff_t left = 2 * i + 1;
        diff_t right = 2 * i + 2;
        if ( left < size && comp( *( first + i ), *( first + left ) ) ) {
            return false;
        }
        if ( right < size && comp( *( first + i ), *( first + right ) ) ) {
            return false;
        }
    }
    return true;
}

template< class ExecutionPolicy, std::random_access_iterator It, class Comp >
requires std::is_execution_policy_v<std::remove_cvref_t<ExecutionPolicy>>
bool is_heap( ExecutionPolicy&& policy, It first, It last, Comp comp ) {
    return is_heap( first, last, comp );
}

template< std::random_access_iterator It >
constexpr It is_heap_until( It first, It last ) {
    using diff_t = typename std::iterator_traits<It>::difference_type;
    diff_t size = last - first;
    for ( diff_t i = 0; i <= ( size - 2 ) / 2; ++i ) {
        diff_t left = 2 * i + 1;
        diff_t right = 2 * i + 2;
        if ( left < size && std::less<>{}( *( first + i ), *( first + left ) ) ) {
            return first + left;
        }
        if ( right < size && std::less<>{}( *( first + i ), *( first + right ) ) ) {
            return first + right;
        }
    }
    return last;
}

template< class ExecutionPolicy, std::random_access_iterator It >
requires std::is_execution_policy_v<std::remove_cvref_t<ExecutionPolicy>>
It is_heap_until( ExecutionPolicy&& policy, It first, It last ) {
    return is_heap_until( first, last );
}

template< std::random_access_iterator It, class Comp >
constexpr It is_heap_until( It first, It last, Comp comp ) {
    using diff_t = typename std::iterator_traits<It>::difference_type;
    diff_t size = last - first;
    for ( diff_t i = 0; i <= ( size - 2 ) / 2; ++i ) {
        diff_t left = 2 * i + 1;
        diff_t right = 2 * i + 2;
        if ( left < size && comp( *( first + i ), *( first + left ) ) ) {
            return first + left;
        }
        if ( right < size && comp( *( first + i ), *( first + right ) ) ) {
            return first + right;
        }
    }
    return last;
}

template< class ExecutionPolicy, std::random_access_iterator It, class Comp >
requires std::is_execution_policy_v<std::remove_cvref_t<ExecutionPolicy>>
It is_heap_until( ExecutionPolicy&& policy, It first, It last, Comp comp ) {
    return is_heap_until( first, last, comp );
}

} // namespace mystd

