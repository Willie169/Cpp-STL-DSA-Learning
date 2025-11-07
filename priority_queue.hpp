#pragma once // priority_queue.hpp

#include <concepts>
#include <functional>
#include <iterator>
#include <memory>
#include <utility>
#include "algorithm-heap.hpp"
#include "vector.hpp"

namespace mystd {
template<class T, class Container = mystd::vector<T>, class Compare = std::less<typename Container::value_type>>
requires std::random_access_iterator<typename Container::iterator> && std::predicate<Compare, const T&, const T&>
class priority_queue {
public:
    using container_type = Container;
    using value_compare = Compare;
    using value_type = typename Container::value_type;
    using size_type = typename Container::size_type;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;

protected:
    Container c = Container();
    Compare comp = Compare();

public:
    priority_queue() : priority_queue(Compare(), Container()) {}

    explicit priority_queue( const Compare& compare ) : priority_queue(compare, Container()) {}

    priority_queue( const Compare& compare, const Container& cont ) : c(cont), comp(compare) {
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    priority_queue( const Compare& compare, Container&& cont ) : c(std::move(cont)), comp(compare) {
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    priority_queue( const priority_queue& other ) : c(other.c), comp(other.comp) {}

    priority_queue( priority_queue&& other ) noexcept : c(std::move(other.c)), comp(std::move(other.comp)) {}

    template< std::input_iterator InputIt >
    requires requires(InputIt first, InputIt last) { { Container(first, last) }; }
    priority_queue( InputIt first, InputIt last, const Compare& compare = Compare() ) : c(first, last), comp(compare) {
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    template< std::input_iterator InputIt >
    requires requires(InputIt first, InputIt last) { { std::declval<Container&>().insert(first, last) }; }
    priority_queue( InputIt first, InputIt last, const Compare& compare, const Container& cont ) : c(cont), comp(compare) {
        c.insert(c.end(), first, last);
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    template< std::input_iterator InputIt >
    requires requires(InputIt first, InputIt last) { { std::declval<Container&>().insert(first, last) }; }
    priority_queue( InputIt first, InputIt last, const Compare& compare, Container&& cont ) : c(std::move(cont)), comp(compare) {
        c.insert(c.end(), first, last);
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    template< class Alloc >
    explicit priority_queue( const Alloc& alloc ) : c(alloc), comp(Compare()) {}

    template< class Alloc >
    priority_queue( const Compare& compare, const Alloc& alloc ) : c(alloc), comp(compare) {}

    template< class Alloc >
    priority_queue( const Compare& compare, const Container& cont, const Alloc& alloc ) : c(cont, alloc), comp(compare) {
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    template< class Alloc >
    priority_queue( const Compare& compare, Container&& cont, const Alloc& alloc ) : c(std::move(cont), alloc), comp(compare) {
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    template< class Alloc >
    priority_queue( const priority_queue& other, const Alloc& alloc ) : c(other.c, alloc), comp(other.comp) {}

    template< class Alloc >
    priority_queue( priority_queue&& other, const Alloc& alloc ) : c(std::move(other.c), alloc), comp(std::move(other.comp)) {}

    template< std::input_iterator InputIt, class Alloc >
    requires requires(InputIt first, InputIt last) { { std::declval<Container&>().insert(first, last) }; }
    priority_queue( InputIt first, InputIt last, const Alloc& alloc ) : c(alloc), comp(Compare()) {
        c.insert(c.end(), first, last);
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    template< std::input_iterator InputIt, class Alloc >
    requires requires(InputIt first, InputIt last) { { std::declval<Container&>().insert(first, last) }; }
    priority_queue( InputIt first, InputIt last, const Compare& compare, const Alloc& alloc ) : c(alloc), comp(compare) {
        c.insert(c.end(), first, last);
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    template< std::input_iterator InputIt, class Alloc >
    requires requires(InputIt first, InputIt last) { { std::declval<Container&>().insert(first, last) }; }
    priority_queue( InputIt first, InputIt last, const Compare& compare, const Container& cont, const Alloc& alloc ) : c(cont, alloc), comp(compare) {
        c.insert(c.end(), first, last);
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    template< std::input_iterator InputIt, class Alloc >
    requires requires(InputIt first, InputIt last) { { std::declval<Container&>().insert(first, last) }; }
    priority_queue( InputIt first, InputIt last, const Compare& compare, Container&& cont, const Alloc& alloc ) : c(std::move(cont), alloc), comp(compare) {
        c.insert(c.end(), first, last);
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    constexpr ~priority_queue() = default;

    priority_queue& operator=( const priority_queue& other ) {
        if (this != &other) {
            c = other.c;
            comp = other.comp;
        }
        return *this;
    }

    priority_queue& operator=( priority_queue&& other ) noexcept {
        if (this != &other) {
            c = std::move(other.c);
            comp = std::move(other.comp);
        }
        return *this;
    }

    const_reference top() const {
        return c.front();
    }

    bool empty() const {
        return c.empty();
    }

    size_type size() const {
        return c.size();
    }

    void push( const value_type& value ) {
        c.push_back(value);
        mystd::push_heap(c.begin(), c.end(), comp);
    }

    void push( value_type&& value ) {
        c.push_back(std::move(value));
        mystd::push_heap(c.begin(), c.end(), comp);
    }

    template< class... Args >
    reference emplace( Args&&... args ) {
        c.emplace_back(std::forward<Args>(args)...);
        mystd::push_heap(c.begin(), c.end(), comp);
        return c.back();
    }

    void pop() {
        mystd::pop_heap(c.begin(), c.end(), comp);
        c.pop_back();
    }

    void swap( priority_queue& other ) noexcept( noexcept(std::swap(c, other.c)) && noexcept(std::swap(comp, other.comp)) ) {
        std::swap(c, other.c);
        std::swap(comp, other.comp);
    }
};

template< class T, class Container, class Compare >
requires std::predicate<Compare, const T&, const T&>
void swap( mystd::priority_queue<T, Container, Compare>& lhs, mystd::priority_queue<T, Container, Compare>& rhs ) noexcept( noexcept(lhs.swap(rhs)) ) {
    lhs.swap(rhs);
}

template< class Comp, class Container >
priority_queue( Comp, Container ) -> priority_queue<typename Container::value_type, Container, Comp>;

template< std::input_iterator InputIt, class Comp = std::less<typename std::iterator_traits<InputIt>::value_type>, class Container = mystd::vector<typename std::iterator_traits<InputIt>::value_type> >
priority_queue( InputIt, InputIt, Comp = Comp(), Container = Container() ) -> priority_queue<typename std::iterator_traits<InputIt>::value_type, Container, Comp>;

template< class Comp, class Container, class Alloc >
priority_queue( Comp, Container, Alloc ) -> priority_queue<typename Container::value_type, Container, Comp>;

template< std::input_iterator InputIt, class Alloc >
priority_queue( InputIt, InputIt, Alloc ) -> priority_queue<typename std::iterator_traits<InputIt>::value_type, mystd::vector<typename std::iterator_traits<InputIt>::value_type>, std::less<typename std::iterator_traits<InputIt>::value_type>>;

template< std::input_iterator InputIt, class Comp, class Alloc >
priority_queue( InputIt, InputIt, Comp, Alloc ) -> priority_queue<typename std::iterator_traits<InputIt>::value_type, mystd::vector<typename std::iterator_traits<InputIt>::value_type>, Comp>;

template< std::input_iterator InputIt, class Comp, class Container, class Alloc >
priority_queue( InputIt, InputIt, Comp, Container, Alloc ) -> priority_queue<typename Container::value_type, Container, Comp>;

} // namespace mystd

namespace std {

template< class T, class Container, class Compare, class Alloc >
requires std::predicate<Compare, const T&, const T&>
struct uses_allocator<mystd::priority_queue<T, Compare, Container>, Alloc> : std::uses_allocator<Container, Alloc> {};

} // namespace std

