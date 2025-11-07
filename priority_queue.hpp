#pragma once

#include <functional>
#include "vector.hpp"

namespace mystd {
template<class T, class Container = mystd::vector<T>, class Compare = std::less<typename Container::value_type>>
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

    template< class InputIt >
    priority_queue( InputIt first, InputIt last, const Compare& compare = Compare() ) : c(first, last), comp(compare) {
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    template< class InputIt >
    priority_queue( InputIt first, InputIt last, const Compare& compare, const Container& cont ) : c(cont), comp(compare) {
        c.insert(c.end(), first, last);
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    template< class InputIt >
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

    template< class InputIt, class Alloc >
    priority_queue( InputIt first, InputIt last, const Alloc& alloc ) : c(alloc), comp(Compare()) {
        c.insert(c.end(), first, last);
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    template< class InputIt, class Alloc >
    priority_queue( InputIt first, InputIt last, const Compare& compare, const Alloc& alloc ) : c(alloc), comp(compare) {
        c.insert(c.end(), first, last);
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    template< class InputIt, class Alloc >
    priority_queue( InputIt first, InputIt last, const Compare& compare, const Container& cont, const Alloc& alloc ) : c(cont, alloc), comp(compare) {
        c.insert(c.end(), first, last);
        mystd::make_heap(c.begin(), c.end(), comp);
    }

    template< class InputIt, class Alloc >
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

    void swap( priority_queue& other ) noexcept( noexcept(mystd::swap(c, other.c)) && noexcept(mystd::swap(comp, other.comp)) ) {
        mystd::swap(c, other.c);
        mystd::swap(comp, other.comp);
    }
};

template< class T, class Container, class Compare >
void swap( std::priority_queue<T, Container, Compare>& lhs, std::priority_queue<T, Container, Compare>& rhs ) noexcept( noexcept(lhs.swap(rhs)) ) {
    lhs.swap(rhs);
}

template< class T, class Container, class Compare, class Alloc >
struct uses_allocator<std::priority_queue<T, Compare, Container>, Alloc> : std::uses_allocator<Container, Alloc>::type {};

template< class Comp, class Container >
priority_queue( Comp, Container ) -> priority_queue<typename Container::value_type, Container, Comp>;

template< class InputIt, class Comp = std::less<typename std::iterator_traits<InputIt>::value_type>, class Container = mystd::vector<typename std::iterator_traits<InputIt>::value_type> >

template< class Comp, class Container, class Alloc >
priority_queue( Comp, Container, Alloc ) -> priority_queue<typename Container::value_type, Container, Comp>;

template< class InputIt, class Alloc >
priority_queue( InputIt, InputIt, Alloc ) -> priority_queue<typename std::iterator_traits<InputIt>::value_type, mystd::vector<typename std::iterator_traits<InputIt>::value_type>, std::less<typename std::iterator_traits<InputIt>::value_type>>;

template< class InputIt, class Comp, class Alloc >
priority_queue( InputIt, InputIt, Comp, Alloc ) -> priority_queue<typename std::iterator_traits<InputIt>::value_type, mystd::vector<typename std::iterator_traits<InputIt>::value_type>, Comp>;

template< class InputIt, class Comp, class Container, class Alloc >
priority_queue( InputIt, InputIt, Comp, Container, Alloc ) -> priority_queue<typename Container::value_type, Container, Comp>;

} // namespace mystd

