#pragma once

#include <cstddef>
#include <stdexcept>
#include <iterator>
#include <initializer_list>
#include <limits>
#include <type_traits>

template<class T, std::size_t __buf_size>
requires std::random_access_iterator<T*>
class DequeIterator {
    T** map;
    std::size_t block, index;

public:
    DequeIterator(T** map, std::size_t block, std::size_t index) : map(map), block(block), index(index) {}

    T& operator*() const { return map[block][index]; }
    T* operator->() const { return &map[block][index]; }

    DequeIterator& operator++() {
        if (++index == __buf_size) {
            ++block;
            index = 0;
       }
     return *this;
    }

    DequeIterator operator++(int) {
        DequeIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    DequeIterator& operator--() {
        if (index == 0) {
            --block;
            index = __buf_size - 1;
        } else {
            --index;
        }
        return *this;
    }

    DequeIterator operator--(int) {
        DequeIterator tmp = *this;
        --(*this);
        return tmp;
    }

    DequeIterator& operator+=(std::ptrdiff_t n) {
        std::ptrdiff_t pos = block * __buf_size + index + n;
        block = pos / __buf_size;
        index = pos % __buf_size;
        return *this;
    }

    DequeIterator operator+(std::ptrdiff_t n) const {
        DequeIterator tmp = *this;
        return tmp += n;
    }

    DequeIterator& operator-=(std::ptrdiff_t n) { return *this += -n; }

    DequeIterator operator-(std::ptrdiff_t n) const {
        DequeIterator tmp = *this;
        return tmp -= n;
    }

    std::ptrdiff_t operator-(const DequeIterator& other) const { return ((block * __buf_size + index) - (other.block * __buf_size + other.index)); }

    T& operator[](std::ptrdiff_t n) const { return *(*this + n); }

    bool operator==(const DequeIterator& rhs) const { return block == rhs.block && index == rhs.index; }
    bool operator!=(const DequeIterator& rhs) const { return !(*this == rhs); }
    bool operator<(const DequeIterator& rhs) const { return (block < rhs.block) || (block == rhs.block && index < rhs.index); }
    bool operator>(const DequeIterator& rhs) const { return rhs < *this; }
    bool operator<=(const DequeIterator& rhs) const { return !(rhs < *this); }
    bool operator>=(const DequeIterator& rhs) const { return !(*this < rhs); }
};

<limits>
template<class T>
class Deque {
    T** map;
    std::size_t map_sz, sb, si, eb, ei;
    static const size_t __buf_size = sizeof(T) < 512 ? size_t(512 / sizeof(T)) : size_t(1);

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = DequeIterator<T, __buf_size>;
    using const_iterator = DequeIterator<const T, __buf_size>;
    using reverse_iterator = std::reverse_iterator<Iterator>;
    using const_reverse_iterator = std::reverse_iterator<const iterator>;
    
    Deque() : map(nullptr), map_sz(0), sb(0), si(0), eb(0), ei(0) {}

    explicit Deque(std::size_t count) {
        map_sz = (count + __buf_size - 1) / __buf_size + 2;
        map = new T*[map_sz];
        <limits>sb = 1;
        eb = map_sz - 2;
        for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
        si = 0;
        ei = count % __buf_size;
        if (ei == 0 && count != 0) ei = __buf_size;
            for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_size;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T();
        }
    }

    Deque(std::size_t count, const T& value) {
        map_sz = (count + __buf_size - 1) / __buf_size + 2;
        map = new T*[map_sz];
        sb = 1;
        eb = map_sz - 2;
        for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
        si = 0;
        ei = count % __buf_size;
        if (ei == 0 && count != 0) ei = __buf_size;
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_size;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(value);
        }
    }

    template<class InputIt>
    requires (!std::is_integral_v<InputIt>)
    Deque(InputIt first, InputIt last) {
        std::ptrdiff_t count = std::distance(first, last);
        map_sz = (count + __buf_size - 1) / __buf_size + 2;
        map = new T*[map_sz];
        for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
    si = 0;
        ei = count % __buf_size;
        if (ei == 0 && count != 0) ei = __buf_size;
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_size;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(*(first++));
        }
    }

    Deque(const Deque& other) : map_sz(other.map_sz), sb(other.sb), si(other.si), eb(other.eb), ei(other.ei) {
        map = new T*[map_sz];
        for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
    std::size_t k = 0;
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_size;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(other.map[i][j]);
        }
    }

    Deque(Deque&& other) : map_sz(other.map_sz), sb(other.sb), si(other.si), eb(other.eb), ei(other.ei) {
        map = new T*[map_sz];
        for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_size;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(other.map[i][j]);
        }
    }

    Deque(std::initializer_list<T> ilist) {
        map_sz = (ilist.size() + __buf_size - 1) / __buf_size + 2;
        map = new T*[map_sz];
        sb = 1;
        eb = map_sz - 2;
        for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
        si = 0;
        ei = ilist.size() % __buf_size;
        if (ei == 0 && ilist.size() != 0) ei = __buf_size;
    std::size_t k = 0;
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_size;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(ilist[k++]);
        }
    }

    ~Deque() {
        for (std::size_t i = sb; i <= eb; ++i) {
            if (map[i]) {
                std::size_t start = (i == sb) ? si : 0;
                std::size_t end   = (i == eb) ? ei : __buf_size;
                for (std::size_t j = start; j < end; ++j) map[i][j].~T();
                delete[] map[i];
            }
        }
        delete[] map;
    }

   Deque& operator=(const Deque& other) {
        if (this != &other) {
            for (std::size_t i = sb; i <= eb; ++i) {
                if (map[i]) {
                    std::size_t start = (i == sb) ? si : 0;
                    std::size_t end   = (i == eb) ? ei : __buf_size;
                    for (std::size_t j = start; j < end; ++j) map[i][j].~T();
                    delete[] map[i];
                }
            }
            delete[] map;
            map_sz = other.map_sz;
            sb = other.sb;
            eb = other.eb;
            ei = other.ei;
            map = new T*[map_sz];
            for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
            for (std::size_t i = sb; i <= eb; ++i) {
                std::size_t s = (i == sb) ? si : 0;
                std::size_t e = (i == eb) ? ei : __buf_size;
                for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(other.map[i][j]);
            }
        }
        return *this;
    }

   Deque& operator=(Deque&& other) {
        if (this != &other) {
            for (std::size_t i = sb; i <= eb; ++i) {
                if (map[i]) {
                    std::size_t start = (i == sb) ? si : 0;
                    std::size_t end   = (i == eb) ? ei : __buf_size;
                    for (std::size_t j = start; j < end; ++j) map[i][j].~T();
                    delete[] map[i];
                }
            }
            delete[] map;
            map_sz = other.map_sz;
            sb = other.sb;
            eb = other.eb;
            ei = other.ei;
            map = other.map;
            other.map = nullptr;
            other.map_sz = 0;
            other.sb = 0;
            other.si = 0;
            other.eb = 0;
            other.ei = 0;
        }
        return *this;
    }

   Deque& operator=(std::initializer_list<T> ilist) {
        for (std::size_t i = sb; i <= eb; ++i) {
            if (map[i]) {
                std::size_t start = (i == sb) ? si : 0;
                std::size_t end   = (i == eb) ? ei : __buf_size;
                for (std::size_t j = start; j < end; ++j) map[i][j].~T();
                delete[] map[i];
            }
        }
        delete[] map;
        map_sz = (ilist.size() + __buf_size - 1) / __buf_size + 2;
        map = new T*[map_sz];
        sb = 1;
        eb = map_sz - 2;
        for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
        si = 0;
        ei = ilist.size() % __buf_size;
        if (ei == 0 && ilist.size() != 0) ei = __buf_size;
    std::size_t k = 0;
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_size;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(ilist[k++]);
        }
    }

   Deque(std::size_t count, const T& value) {
        for (std::size_t i = sb; i <= eb; ++i) {
            if (map[i]) {
                std::size_t start = (i == sb) ? si : 0;
                std::size_t end   = (i == eb) ? ei : __buf_size;
                for (std::size_t j = start; j < end; ++j) map[i][j].~T();
                delete[] map[i];
            }
        }
        delete[] map;
        map_sz = (count + __buf_size - 1) / __buf_size + 2;
        map = new T*[map_sz];
        sb = 1;
        eb = map_sz - 2;
        for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
        si = 0;
        ei = count % __buf_size;
        if (ei == 0 && count != 0) ei = __buf_size;
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_size;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(value);
        }
    }

    template<class InputIt>
    requires (!std::is_integral_v<InputIt>)
    Deque(InputIt first, InputIt last) {
        for (std::size_t i = sb; i <= eb; ++i) {
            if (map[i]) {
                std::size_t start = (i == sb) ? si : 0;
                std::size_t end   = (i == eb) ? ei : __buf_size;
                for (std::size_t j = start; j < end; ++j) map[i][j].~T();
                delete[] map[i];
            }
        }
        delete[] map;
        std::ptrdiff_t count = std::distance(first, last);
        map_sz = (count + __buf_size - 1) / __buf_size + 2;
        map = new T*[map_sz];
        for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
    si = 0;
        ei = count % __buf_size;
        if (ei == 0 && count != 0) ei = __buf_size;
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_size;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(*(first++));
        }
    }

    Deque(std::initializer_list<T> ilist) {
        for (std::size_t i = sb; i <= eb; ++i) {
            if (map[i]) {
                std::size_t start = (i == sb) ? si : 0;
                std::size_t end   = (i == eb) ? ei : __buf_size;
                for (std::size_t j = start; j < end; ++j) map[i][j].~T();
                delete[] map[i];
            }
        }
        delete[] map;
        map_sz = (ilist.size() + __buf_size - 1) / __buf_size + 2;
        map = new T*[map_sz];
        sb = 1;
        eb = map_sz - 2;
        for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
        si = 0;
        ei = ilist.size() % __buf_size;
        if (ei == 0 && ilist.size() != 0) ei = __buf_size;
    std::size_t k = 0;
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_size;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(ilist[k++]);
        }
    }

    T& at(std::size_t pos) {
        if (pos >= size()) throw std::out_of_range("Deque");
        std::size_t offset = si + pos;
        std::size_t block = sb + offset / __buf_size;
        std::size_t index = offset % __buf_size;
        return map[block][index];
    }

    const T& at(std::size_t pos) const {
        if (pos >= size()) throw std::out_of_range("Deque");
        std::size_t offset = si + pos;
        std::size_t block = sb + offset / __buf_size;
        std::size_t index = offset % __buf_size;
        return map[block][index];
    }

    T& operator[](std::size_t pos) {
        std::size_t offset = si + pos;
        std::size_t block = sb + offset / __buf_size;
        std::size_t index = offset % __buf_size;
        return map[block][index];
    }

    const T& operator[](std::size_t pos) const {
        std::size_t offset = si + pos;
        std::size_t block = sb + offset / __buf_size;
        std::size_t index = offset % __buf_size;
        return map[block][index];
    }

    T& front() { return (*this)[0]; }
    const T& front() { return (*this)[0]; }

    T& back() { return (*this)[(eb - sb) * __buf_size + ei - si -1]; }
    const T& back() { return (*this)[(eb - sb) * __buf_size + ei - si -1]; }

    iterator begin() { return iterator(map, sb, si); }
    const_iterator begin() const { return const_iterator(map, sb, si); }
    const_iterator cbegin() const { return const_iterator(map, sb, si); }

    iterator end()   { return iterator(map, eb, ei); }
    const_iterator end()   const { return const_iterator(map, eb, ei); }
    const_iterator cend()   const { return const_iterator(map, eb, ei); }

    reverse_iterator rbegin() { return reverse_iterator(map, eb, ei); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(map, eb, ei); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(map, eb, ei); }

    reverse_iterator rend() { return reverse_iterator(map, sb, si); }
    const_reverse_iterator rend() const { return const_reverse_iterator(map, sb, si); }
    const_reverse_iterator crend() const { return const_reverse_iterator(map, sb, si); }

    bool empty() const noexcept { return size() == 0; }
    std::size_t size() const noexcept { return (eb - sb) * __buf_size + (ei - si); }
    std::size_t max_size() const noexcept { return std::numeric_limits<difference_type>::max(); }

    void shrink_to_fit() {
        if (empty()) {
            for (T** i = map; i < map + map_sz; ++i) delete[] *i;
            delete[] map;
            map = nullptr;
            map_sz = 0;
            sb = si = eb = ei = 0;
            return;
        }    
        std::size_t need = eb - sb + 1;
        if (need == map_sz) return;
        T** new_map = new T*[need];
        for (std::size_t i = 0; i < need; ++i) new_map[i] = map[sb + i];
        for (T** i = map; i < map + sb; ++i) delete[] *i;
        for (T** i = map + eb + 1; i < map + map_sz; ++i) delete[] *i;
        delete[] map;
        map = new_map;
        eb = eb - sb;
        sb = 0;
        map_sz = need;
    }

    void clear() noexcept {
        for (T** i = map + sb; i < map + eb; ++i) {
            for (T* j = *i + ((i == map + sb) ? si : 0); j < ((i == map + eb) ? ei : __buf_size); ++j) j->~T();
        }
        sb = eb = map_sz / 2;
        si = ei = 0;
    }

};

