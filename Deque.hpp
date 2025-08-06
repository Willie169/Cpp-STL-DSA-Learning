#pragma once

#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <memory_resource>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include "Vector.hpp"


template<class T, std::size_t __buf_sz, class Allocator>
class __DequeIterator {
    T** map;
    std::size_t block, index;


public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;


    __DequeIterator(T** map, std::size_t block, std::size_t index) : map(map), block(block), index(index) {}


    T& operator*() const { return map[block][index]; }
    T* operator->() const { return &map[block][index]; }


    __DequeIterator& operator++() {
        if (++index == __buf_sz) {
            ++block;
            index = 0;
        }
        return *this;
    }


    __DequeIterator operator++(int) {
        __DequeIterator tmp = *this;
        ++(*this);
        return tmp;
    }


    __DequeIterator& operator--() {
        if (index == 0) {
            --block;
            index = __buf_sz - 1;
        } else {
            --index;
        }
        return *this;
    }


    __DequeIterator operator--(int) {
        __DequeIterator tmp = *this;
        --(*this);
        return tmp;
    }


    __DequeIterator& operator+=(std::ptrdiff_t n) {
        std::ptrdiff_t pos = static_cast<std::ptrdiff_t>(block * __buf_sz + index) + n;
        block = pos / __buf_sz;
        index = pos % __buf_sz;
        return *this;
    }


    __DequeIterator operator+(std::ptrdiff_t n) const {
        __DequeIterator tmp = *this;
        return tmp += n;
    }


    __DequeIterator& operator-=(std::ptrdiff_t n) { return *this += -n; }


    __DequeIterator operator-(std::ptrdiff_t n) const {
        __DequeIterator tmp = *this;
        return tmp -= n;
    }


    std::ptrdiff_t operator-(const __DequeIterator& other) const { return ((block * __buf_sz + index) - (other.block * __buf_sz + other.index)); }


    T& operator[](std::ptrdiff_t n) const { return *(*this + n); }


    bool operator==(const __DequeIterator& rhs) const { return map == rhs.map && block == rhs.block && index == rhs.index; }


    std::strong_ordering operator<=>(const __DequeIterator& rhs) const {
        if (map != rhs.map) return std::strong_ordering::unordered;
        std::ptrdiff_t diff = (*this) - rhs;
        if (diff < 0) return std::strong_ordering::less;
        if (diff > 0) return std::strong_ordering::greater;
        return std::strong_ordering::equal;
    }
};


template<class T, std::size_t __buf_sz, class Allocator>
__DequeIterator<T, __buf_sz, Allocator> operator+(std::ptrdiff_t n, const __DequeIterator<T, __buf_sz, Allocator>& it) { return it + n; }


template<class T, class Allocator = std::allocator<T>>
class Deque {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
    using iterator = __DequeIterator<T, __buf_sz, Allocator>;
    using const_iterator = __DequeIterator<const T, __buf_sz, Allocator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;


private:
    T** map;
    std::size_t map_sz, sb, si, eb, ei;
    static const size_t __buf_sz = sizeof(T) < 512 ? size_t(512 / sizeof(T)) : size_t(1);
    [[no_unique_address]] Allocator alloc;


    using AllocTraits = std::allocator_traits<Allocator>;
    using MapAllocator = typename AllocTraits::template rebind_alloc<T*>;
    using MapAllocTraits = std::allocator_traits<MapAllocator>;


public:    
    Deque() : Deque(Allocator{}) {}


    explicit Deque(const Allocator& a) : map(nullptr), map_sz(0), sb(0), si(0), eb(0), ei(0), alloc(a) {}


    explicit Deque(std::size_t count, const Allocator& a = Allocator{}) : alloc(a) {
        if (count == 0) {
            map = nullptr;
            map_sz = sb = si = eb = ei = 0;
            return;
        }
        map_sz = (count + __buf_sz - 1) / __buf_sz + 2;
        MapAllocator map_alloc(alloc);
        map = MapAllocTraits::allocate(map_alloc, map_sz);
        std::uninitialized_fill(map, map + map_sz, nullptr);
        sb = 1;
        eb = map_sz - 2;
        for (std::size_t i = sb; i <= eb; ++i) map[i] = AllocTraits::allocate(alloc, __buf_sz);
        si = 0;
        ei = count % __buf_sz;
        if (ei == 0 && count != 0) ei = __buf_sz;
        for (std::size_t i = sb; i <= eb; ++i) { std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_sz;
            for (std::size_t j = s; j < e; ++j) { AllocTraits::construct(alloc, &map[i][j]); }
        }
    }


    Deque(std::size_t count, const T& value, const Allocator& a = Allocator{}) : alloc(a) {
        if (count == 0) {
            map = nullptr;
            map_sz = sb = si = eb = ei = 0;
            return;
        }
        map_sz = (count + __buf_sz - 1) / __buf_sz + 2;
        MapAllocator map_alloc(alloc);
        map = MapAllocTraits::allocate(map_alloc, map_sz);
        std::uninitialized_fill(map, map + map_sz, nullptr);
        sb = 1;
        eb = map_sz - 2;
        for (std::size_t i = sb; i <= eb; ++i) map[i] = AllocTraits::allocate(alloc, __buf_sz);
        si = 0;
        ei = count % __buf_sz;
        if (ei == 0 && count != 0) ei = __buf_sz;
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_sz;
            for (std::size_t j = s; j < e; ++j) AllocTraits::construct(alloc, &map[i][j], value);
        }
    }


    template<std::input_iterator InputIt>
    Deque(InputIt first, InputIt last, const Allocator& a = Allocator{}) : alloc(a) {
        if constexpr (std::random_access_iterator<InputIt>) {
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            if (count == 0) {
                map = nullptr;
                map_sz = sb = si = eb = ei = 0;
                return;
            }
            map_sz = (count + __buf_sz - 1) / __buf_sz + 2;
            sb = 1;
            eb = map_sz - 2;
            MapAllocator map_alloc(alloc);
            map = MapAllocTraits::allocate(map_alloc, map_sz);
            std::uninitialized_fill(map, map + map_sz, nullptr);
            for (std::size_t i = sb; i <= eb; ++i) map[i] = AllocTraits::allocate(alloc, __buf_sz);
            si = 0;
            ei = count % __buf_sz;
            if (ei == 0 && count != 0) ei = __buf_sz;
            for (std::size_t i = sb; i <= eb; ++i) {
                std::size_t s = (i == sb) ? si : 0;
                std::size_t e = (i == eb) ? ei : __buf_sz;
                for (std::size_t j = s; j < e; ++j) {
                    AllocTraits::construct(alloc, &map[i][j], *first);
                    ++first;
                }
            }
        } else {
            *this = Deque(1, T{}, a);
            for (; first != last; ++first) push_back(*first);
        }
    }


    Deque(const Deque& other) : Deque(other, AllocTraits::select_on_container_copy_construction(other.alloc)) {}


    Deque(const Deque& other, const Allocator& a) : alloc(a), map_sz(other.map_sz), sb(other.sb), si(other.si), eb(other.eb), ei(other.ei) {
        if (map_sz == 0) {
            map = nullptr;
            return;
        }
        MapAllocator map_alloc(alloc);
        map = MapAllocTraits::allocate(map_alloc, map_sz);
        std::uninitialized_fill(map, map + map_sz, nullptr);
        for (std::size_t i = sb; i <= eb; ++i) map[i] = AllocTraits::allocate(alloc, __buf_sz);
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_sz;
            for (std::size_t j = s; j < e; ++j) AllocTraits::construct(alloc, &map[i][j], other.map[i][j]);
        }
    }


    Deque(Deque&& other) noexcept : Deque(std::move(other), std::move(other.alloc)) {}


    Deque(Deque&& other, const Allocator& a) : map(std::exchange(other.map, nullptr)), map_sz(std::exchange(other.map_sz, 0)), sb(std::exchange(other.sb, 0)), si(std::exchange(other.si, 0)), eb(std::exchange(other.eb, 0)), ei(std::exchange(other.ei, 0)), alloc(a) {}

    Deque(std::initializer_list<T> ilist, const Allocator& a = Allocator{}) : alloc(a) {
        if (ilist.size() == 0) {
            map = nullptr;
            map_sz = sb = si = eb = ei = 0;
            return;
        }
        map_sz = (ilist.size() + __buf_sz - 1) / __buf_sz + 2;
        MapAllocator map_alloc(alloc);
        map = MapAllocTraits::allocate(map_alloc, map_sz);
        std::uninitialized_fill(map, map + map_sz, nullptr);
        sb = 1;
        eb = map_sz - 2;
        for (std::size_t i = sb; i <= eb; ++i) map[i] = AllocTraits::allocate(alloc, __buf_sz);
        si = 0;
        ei = ilist.size() % __buf_sz;
        if (ei == 0 && ilist.size() != 0) ei = __buf_sz;
        auto k = ilist.begin();
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_sz;
            for (std::size_t j = s; j < e; ++j, ++k) AllocTraits::construct(alloc, &map[i][j], *k);
        }
    }


    ~Deque() {
        if (!map) return;
        for (std::size_t i = sb; i <= eb; ++i) {
            if (map[i]) {
                std::size_t start = (i == sb) ? si : 0;
                std::size_t end   = (i == eb) ? ei : __buf_sz;
                for (std::size_t j = start; j < end; ++j) AllocTraits::destroy(alloc, &map[i][j]);
                AllocTraits::deallocate(alloc, map[i], __buf_sz);
            }
        }
        MapAllocator map_alloc(alloc);
        MapAllocTraits::deallocate(map_alloc, map, map_sz);
    }


    Deque& operator=(const Deque& other) {
        if (this != &other) {
            if constexpr (AllocTraits::propagate_on_container_copy_assignment::value) {
                if (alloc != other.alloc) {
                    clear();
                    deallocate_map();
                }
                alloc = other.alloc;
            }
            Deque tmp(other, alloc);
            swap(tmp);
        }
        return *this;
    }


    Deque& operator=(Deque&& other) noexcept(AllocTraits::is_always_equal::value || AllocTraits::propagate_on_container_move_assignment::value) {
        if (this != &other) {
            if constexpr (AllocTraits::propagate_on_container_move_assignment::value) {
                clear();
                deallocate_map();
                alloc = std::move(other.alloc);
                map = std::exchange(other.map, nullptr);
                map_sz = std::exchange(other.map_sz, 0);
                sb = std::exchange(other.sb, 0);
                si = std::exchange(other.si, 0);
                eb = std::exchange(other.eb, 0);
                ei = std::exchange(other.ei, 0);
            } else if (alloc == other.alloc) {
                clear();
                deallocate_map();
                map = std::exchange(other.map, nullptr);
                map_sz = std::exchange(other.map_sz, 0);
                sb = std::exchange(other.sb, 0);
                si = std::exchange(other.si, 0);
                eb = std::exchange(other.eb, 0);
                ei = std::exchange(other.ei, 0);
            } else *this = Deque(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()), alloc);
        }
        return *this;
    }


    Deque& operator=(std::initializer_list<T> ilist) {
        clear();
        deallocate_map();
        if (ilist.size() == 0) {
            map = nullptr;
            map_sz = sb = si = eb = ei = 0;
            return *this;
        }
        map_sz = (ilist.size() + __buf_sz - 1) / __buf_sz + 2;
        MapAllocator map_alloc(alloc);
        map = MapAllocTraits::allocate(map_alloc, map_sz);
        std::uninitialized_fill(map, map + map_sz, nullptr);
        sb = 1;
        eb = map_sz - 2;
        for (std::size_t i = sb; i <= eb; ++i) map[i] = AllocTraits::allocate(alloc, __buf_sz);
        si = 0;
        ei = ilist.size() % __buf_sz;
        if (ei == 0 && ilist.size() != 0) ei = __buf_sz;
        auto k = ilist.begin();
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_sz;
            for (std::size_t j = s; j < e; ++j, ++k) AllocTraits::construct(alloc, &map[i][j], *k);
        }
        return *this;
    }


    allocator_type get_allocator() const noexcept { return alloc; }


    T& at(std::size_t pos) {
        if (pos >= size()) throw std::out_of_range("Deque");
        std::size_t offset = si + pos;
        std::size_t block = sb + offset / __buf_sz;
        std::size_t index = offset % __buf_sz;
        return map[block][index];
    }


    const T& at(std::size_t pos) const {
        if (pos >= size()) throw std::out_of_range("Deque");
        std::size_t offset = si + pos;
        std::size_t block = sb + offset / __buf_sz;
        std::size_t index = offset % __buf_sz;
        return map[block][index];
    }


    T& operator[](std::size_t pos) {
        std::size_t offset = si + pos;
        std::size_t block = sb + offset / __buf_sz;
        std::size_t index = offset % __buf_sz;
        return map[block][index];
    }


    const T& operator[](std::size_t pos) const {
        std::size_t offset = si + pos;
        std::size_t block = sb + offset / __buf_sz;
        std::size_t index = offset % __buf_sz;
        return map[block][index];
    }


    T& front() { return (*this)[0]; }
    const T& front() const { return (*this)[0]; }

    T& back() { return (*this)[size() - 1]; }
    const T& back() const { return (*this)[size() - 1]; }


    iterator begin() { return iterator(map, sb, si); }
    const_iterator begin() const { return const_iterator(map, sb, si); }
    const_iterator cbegin() const { return const_iterator(map, sb, si); }

    iterator end()   { return iterator(map, eb, ei); }
    const_iterator end()   const { return const_iterator(map, eb, ei); }
    const_iterator cend()   const { return const_iterator(map, eb, ei); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(cend()); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }

    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(cbegin()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }


    bool empty() const noexcept {
        if (map_sz == 0) return true;
        return sb == eb && si >= ei;
    }


    std::size_t size() const noexcept {
        if (map_sz == 0) return 0;
        if (sb == eb) return ei - si;
        return (eb - sb - 1) * __buf_sz + __buf_sz - si + ei;
    }


    std::size_t max_size() const noexcept {
        return std::min(std::numeric_limits<difference_type>::max(),
                       AllocTraits::max_size(alloc));
    }


    void shrink_to_fit() {
        if (empty()) {
            clear();
            deallocate_map();
            map = nullptr;
            map_sz = 0;
            sb = si = eb = ei = 0;
            return;
        }
        std::size_t need = eb - sb + 1;
        if (need == map_sz) return;
        MapAllocator map_alloc(alloc);
        T** new_map = MapAllocTraits::allocate(map_alloc, need);
        std::uninitialized_fill(new_map, new_map + need, nullptr);
        for (std::size_t i = 0; i < need; ++i) new_map[i] = map[sb + i];
        for (std::size_t i = 0; i < sb; ++i) if (map[i]) AllocTraits::deallocate(alloc, map[i], __buf_sz);
        for (std::size_t i = eb + 1; i < map_sz; ++i) if (map[i]) AllocTraits::deallocate(alloc, map[i], __buf_sz);
        MapAllocTraits::deallocate(map_alloc, map, map_sz);
        map = new_map;
        eb = eb - sb;
        sb = 0;
        map_sz = need;
    }


    void clear() noexcept {
        if (empty() || !map) {
            sb = eb = map_sz / 2;
            si = ei = __buf_sz / 2;
            return;
        }
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_sz;
            for (std::size_t j = s; j < e; ++j) AllocTraits::destroy(alloc, &map[i][j]);
        }
        sb = eb = map_sz / 2;
        si = ei = __buf_sz / 2;
    }


    iterator insert(const_iterator pos, const T& value) { return emplace(pos, value); }
    iterator insert(const_iterator pos, T&& value) { return emplace(pos, std::move(value)); }


    iterator insert(const_iterator pos, std::size_t count, const T& value) {
        Vector<T> tmp(count, value);
        return insert(pos, tmp.begin(), tmp.end());
    }


    template<std::input_iterator InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        if constexpr (std::random_access_iterator<InputIt>) {
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            if (count == 0) return iterator(const_cast<T**>(pos.map), pos.block, pos.index);
            std::size_t index = pos - cbegin();
            ensure_capacity_for_insert(count);
            iterator new_pos = begin() + index;
            if (index <= size() / 2) {
                for (std::size_t i = 0; i < count; ++i) push_front(T{});
                std::move(begin() + count, begin() + index + count, begin());
            } else {
                for (std::size_t i = 0; i < count; ++i) push_back(T{});
                std::move_backward(begin() + index, end() - count, end());
            }
            for (std::size_t i = 0; i < count; ++i, ++first) *(new_pos + i) = *first;
            return new_pos;
        } else {
            Vector<T> tmp;
            for (; first != last; ++first) tmp.push_back(*first);
            return insert(pos, tmp.begin(), tmp.end());
        }
    }


    iterator insert(const_iterator pos, std::initializer_list<T> ilist) { return insert(pos, ilist.begin(), ilist.end()); }


    template<class... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        std::size_t index = pos - cbegin();
        if (index <= size() / 2) {
            push_front(T{});
            iterator new_pos = begin() + index;
            std::move(begin(), new_pos, begin() + 1);
            AllocTraits::destroy(alloc, &*new_pos);
            AllocTraits::construct(alloc, &*new_pos, std::forward<Args>(args)...);
            return new_pos;
        } else {
            push_back(T{});
            iterator new_pos = begin() + index;
            std::move_backward(new_pos, end() - 1, end());
            AllocTraits::destroy(alloc, &*new_pos);
            AllocTraits::construct(alloc, &*new_pos, std::forward<Args>(args)...);
            return new_pos;
        }
    }


    iterator erase(const_iterator pos) {
        iterator mutable_pos = begin() + (pos - cbegin());
        if (pos - cbegin() <= size() / 2) {
            std::move_backward(begin(), mutable_pos, mutable_pos + 1);
            pop_front();
        } else {
            std::move(mutable_pos + 1, end(), mutable_pos);
            pop_back();
        }
        return mutable_pos;
    }


    iterator erase(const_iterator first, const_iterator last) {
        if (first == last) return begin() + (first - cbegin());
        std::size_t count = last - first;
        iterator mutable_first = begin() + (first - cbegin());
        if (first - cbegin() <= (size() - count) / 2) {
            std::move_backward(begin(), mutable_first, mutable_first + count);
            for (std::size_t i = 0; i < count; ++i) pop_front();
        } else {
            std::move(begin() + (last - cbegin()), end(), mutable_first);
            for (std::size_t i = 0; i < count; ++i) pop_back();
        }
        return mutable_first;
    }


    void push_back(const T& value) { emplace_back(value); }
    void push_back(T&& value) { emplace_back(std::move(value)); }


    template<class... Args>
    reference emplace_back(Args&&... args) {
        if (map_sz == 0) initialize_empty_deque();
        if (ei != __buf_sz - 1) ++ei;
        else if (eb != map_sz - 1) {
            ++eb;
            if (!map[eb]) map[eb] = AllocTraits::allocate(alloc, __buf_sz);
            ei = 0;
        } else {
            reallocate_map_back();
            ++eb;
            if (!map[eb]) map[eb] = AllocTraits::allocate(alloc, __buf_sz);
            ei = 0;
        }
        AllocTraits::construct(alloc, &map[eb][ei], std::forward<Args>(args)...);
        return map[eb][ei];
    }


    void pop_back() {
        AllocTraits::destroy(alloc, &map[eb][ei]);
        if (ei != 0) --ei;
        else {
            --eb;
            ei = __buf_sz - 1;
        }
    }


    void push_front(const T& value) { emplace_front(value); }
    void push_front(T&& value) { emplace_front(std::move(value)); }


    template<class... Args>
    reference emplace_front(Args&&... args) {
        if (map_sz == 0) initialize_empty_deque();
        if (si != 0) --si;
        else if (sb != 0) {
            --sb;
            if (!map[sb]) map[sb] = AllocTraits::allocate(alloc, __buf_sz);
            si = __buf_sz - 1;
        } else {
            reallocate_map_front();
            --sb;
            if (!map[sb]) map[sb] = AllocTraits::allocate(alloc, __buf_sz);
            si = __buf_sz - 1;
        }
        AllocTraits::construct(alloc, &map[sb][si], std::forward<Args>(args)...);
        return map[sb][si];
    }


    void pop_front() {
        AllocTraits::destroy(alloc, &map[sb][si]);
        if (si != __buf_sz - 1) ++si;
        else {
            ++sb;
            si = 0;
        }
    }


    void resize(std::size_t count) { resize(count, T{}); }

    void resize(std::size_t count, const T& value) {
        std::size_t current_size = size();
        if (count == current_size) return;
        if (count < current_size) {
            std::size_t to_remove = current_size - count;
            for (std::size_t i = 0; i < to_remove; ++i) pop_back();
        } else {
            std::size_t to_add = count - current_size;
            for (std::size_t i = 0; i < to_add; ++i) emplace_back(value);
        }
    }


    void swap(Deque& other) noexcept(AllocTraits::is_always_equal::value || AllocTraits::propagate_on_container_swap::value) {
        if constexpr (AllocTraits::propagate_on_container_swap::value) std::swap(alloc, other.alloc);
        std::swap(map, other.map);
        std::swap(map_sz, other.map_sz);
        std::swap(sb, other.sb);
        std::swap(si, other.si);
        std::swap(eb, other.eb);
        std::swap(ei, other.ei);
    }


private:
    void deallocate_map() {
        if (!map) return;
        for (std::size_t i = 0; i < map_sz; ++i) if (map[i]) AllocTraits::deallocate(alloc, map[i], __buf_sz);
        MapAllocator map_alloc(alloc);
        MapAllocTraits::deallocate(map_alloc, map, map_sz);
        map = nullptr;
        map_sz = 0;
    }


    void initialize_empty_deque() {
        map_sz = 3;
        MapAllocator map_alloc(alloc);
        map = MapAllocTraits::allocate(map_alloc, map_sz);
        std::uninitialized_fill(map, map + map_sz, nullptr);
        sb = eb = 1;
        si = ei = __buf_sz / 2;
        map[1] = AllocTraits::allocate(alloc, __buf_sz);
    }


    void reallocate_map_back() {
        std::size_t new_map_sz = map_sz * 2;
        MapAllocator map_alloc(alloc);
        T** new_map = MapAllocTraits::allocate(map_alloc, new_map_sz);
        std::uninitialized_fill(new_map, new_map + new_map_sz, nullptr);
        for (std::size_t i = 0; i < map_sz; ++i) new_map[i] = map[i];
        MapAllocTraits::deallocate(map_alloc, map, map_sz);
        map = new_map;
        map_sz = new_map_sz;
    }


    void reallocate_map_front() {
        std::size_t new_map_sz = map_sz * 2;
        std::size_t offset = map_sz;
        MapAllocator map_alloc(alloc);
        T** new_map = MapAllocTraits::allocate(map_alloc, new_map_sz);
        std::uninitialized_fill(new_map, new_map + new_map_sz, nullptr);
        for (std::size_t i = 0; i < map_sz; ++i) new_map[i + offset] = map[i];
        MapAllocTraits::deallocate(map_alloc, map, map_sz);
        map = new_map;
        sb += offset;
        eb += offset;
        map_sz = new_map_sz;
    }


    void ensure_capacity_for_insert(std::size_t count) {        
        while (map_sz < (size() + count + __buf_sz - 1) / __buf_sz + 2) reallocate_map_back();
        std::size_t needed_blocks = (size() + count + __buf_sz - 1) / __buf_sz;
        for (std::size_t i = sb; i < sb + needed_blocks && i < map_sz; ++i) if (!map[i]) map[i] = AllocTraits::allocate(alloc, __buf_sz);
    }
};


template<class T, class Alloc>
bool operator==(const Deque<T, Alloc>& lhs, const Deque<T, Alloc>& rhs) { return lhs.operator==(rhs); }


template<class T, class Alloc>
auto operator<=>(const Deque<T, Alloc>& lhs, const Deque<T, Alloc>& rhs) { return lhs.operator<=>(rhs); }


template<class T, class Alloc>
void swap(Deque<T, Alloc>& lhs, Deque<T, Alloc>& rhs) noexcept(noexcept(lhs.swap(rhs))) { lhs.swap(rhs); }


template<class T, class Alloc, class U>
typename Deque<T, Alloc>::size_type erase(Deque<T, Alloc>& c, const U& value) {
    auto it = std::remove(c.begin(), c.end(), value);
    auto r = std::distance(it, c.end());
    c.erase(it, c.end());
    return r;
}


template<class T, class Alloc, class Pred>
typename Deque<T, Alloc>::size_type erase_if(Deque<T, Alloc>& c, Pred pred) {
    auto it = std::remove_if(c.begin(), c.end(), pred);
    auto r = std::distance(it, c.end());
    c.erase(it, c.end());
    return r;
}


template<class InputIt, class Alloc = std::allocator<typename std::iterator_traits<InputIt>::value_type>>
Deque(InputIt, InputIt, Alloc = Alloc()) -> Deque<typename std::iterator_traits<InputIt>::value_type, Alloc>;


template<class T, class Alloc = std::allocator<T>>
Deque(std::initializer_list<T>, Alloc = Alloc()) -> Deque<T, Alloc>;


namespace pmr {
    template<class T>
    using deque = std::deque<T, std::pmr::polymorphic_allocator<T>>;
}

