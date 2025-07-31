#pragma once

#include <cstddef>
#include <stdexcept>
#include <iterator>
#include <initializer_list>
#include <limits>
#include <type_traits>
#include <utility>
#include "Vector.hpp"

template<class T, std::size_t __buf_sz>
requires std::random_access_iterator<T*>
class DequeIterator {
    T** map;
    std::size_t block, index;

public:
    DequeIterator(T** map, std::size_t block, std::size_t index) : map(map), block(block), index(index) {}

    T& operator*() const { return map[block][index]; }
    T* operator->() const { return &map[block][index]; }

    DequeIterator& operator++() {
        if (++index == __buf_sz) {
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
            index = __buf_sz - 1;
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
        std::ptrdiff_t pos = static_cast<std::ptrdiff_t>(block * __buf_sz + index) + n;
        if (pos < 0) throw std::out_of_range("Deque");
        block = pos / __buf_sz;
        index = pos % __buf_sz;
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

    std::ptrdiff_t operator-(const DequeIterator& other) const { return ((block * __buf_sz + index) - (other.block * __buf_sz + other.index)); }

    T& operator[](std::ptrdiff_t n) const { return *(*this + n); }

    bool operator==(const DequeIterator& rhs) const { return map == rhs.map && block == rhs.block && index == rhs.index; }
    bool operator!=(const DequeIterator& rhs) const { return !(*this == rhs); }
    bool operator<(const DequeIterator& rhs) const { return (block < rhs.block) || (block == rhs.block && index < rhs.index); }
    bool operator>(const DequeIterator& rhs) const { return rhs < *this; }
    bool operator<=(const DequeIterator& rhs) const { return !(rhs < *this); }
    bool operator>=(const DequeIterator& rhs) const { return !(*this < rhs); }
};


template<class T>
class Deque {
    T** map;
    std::size_t map_sz, sb, si, eb, ei;
    static const size_t __buf_sz = sizeof(T) < 512 ? size_t(512 / sizeof(T)) : size_t(1);

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = DequeIterator<T, __buf_sz>;
    using const_iterator = DequeIterator<const T, __buf_sz>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    
    Deque() : map(nullptr), map_sz(0), sb(0), si(0), eb(0), ei(0) {}

    explicit Deque(std::size_t count) {
        if (count == 0) {
            map = nullptr;
            map_sz = sb = si = eb = ei = 0;
            return;
        }
        map_sz = (count + __buf_sz - 1) / __buf_sz + 2;
        map = new T*[map_sz]();
        sb = 1;
        eb = map_sz - 2;
        for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_sz];
        si = 0;
        ei = count % __buf_sz;
        if (ei == 0 && count != 0) ei = __buf_sz;
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_sz;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T();
        }
    }

    Deque(std::size_t count, const T& value) {
        if (count == 0) {
            map = nullptr;
            map_sz = sb = si = eb = ei = 0;
            return;
        }
        map_sz = (count + __buf_sz - 1) / __buf_sz + 2;
        map = new T*[map_sz]();
        sb = 1;
        eb = map_sz - 2;
        for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_sz];
        si = 0;
        ei = count % __buf_sz;
        if (ei == 0 && count != 0) ei = __buf_sz;
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_sz;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(value);
        }
    }

    template<std::input_iterator InputIt>
    Deque(InputIt first, InputIt last) {
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
            map = new T*[map_sz]();
            for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_sz];
            si = 0;
            ei = count % __buf_sz;
            if (ei == 0 && count != 0) ei = __buf_sz;
            for (std::size_t i = sb; i <= eb; ++i) {
                std::size_t s = (i == sb) ? si : 0;
                std::size_t e = (i == eb) ? ei : __buf_sz;
                for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(*(first++));
            }
        } else {
            Deque(1);
            for (; first != last; ++first) push_back(*first);
        }
    }

    Deque(const Deque& other) : map_sz(other.map_sz), sb(other.sb), si(other.si), eb(other.eb), ei(other.ei) {
        if (map_sz == 0) {
            map = nullptr;
            return;
        }
        map = new T*[map_sz]();
        for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_sz];
        std::size_t k = 0;
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_sz;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(other.map[i][j]);
        }
    }

    Deque(Deque&& other) noexcept : map(std::exchange(other.map, nullptr)), map_sz(std::exchange(other.map_sz, 0)), sb(std::exchange(other.sb, 0)), si(std::exchange(other.si, 0)), eb(std::exchange(other.eb, 0)), ei(std::exchange(other.ei, 0)) {}

    Deque(std::initializer_list<T> ilist) {
        map_sz = (ilist.size() + __buf_sz - 1) / __buf_sz + 2;
        map = new T*[map_sz]();
        sb = 1;
        eb = map_sz - 2;
        for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_sz];
        si = 0;
        ei = ilist.size() % __buf_sz;
        if (ei == 0 && ilist.size() != 0) ei = __buf_sz;
        auto k = ilist.begin();
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_sz;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(*k++);
        }
    }

    ~Deque() {
        if (!map) return;
        for (std::size_t i = sb; i <= eb; ++i) {
            if (map[i]) {
                std::size_t start = (i == sb) ? si : 0;
                std::size_t end   = (i == eb) ? ei : __buf_sz;
                for (std::size_t j = start; j < end; ++j) map[i][j].~T();
                delete[] map[i];
            }
        }
        delete[] map;
    }

    Deque& operator=(const Deque& other) {
        if (this != &other) {
            Deque tmp(other);
            swap(tmp);
        }
        return *this;
    }

    Deque& operator=(Deque&& other) {
        if (this != &other) {
            this->~Deque();
            map = std::exchange(other.map, nullptr);
            map_sz = std::exchange(other.map_sz, 0);
            sb = std::exchange(other.sb, 0);
            si = std::exchange(other.si, 0);
            eb = std::exchange(other.eb, 0);
            ei = std::exchange(other.ei, 0);
        }
        return *this;
    }

    Deque& operator=(std::initializer_list<T> ilist) {
        for (std::size_t i = sb; i <= eb; ++i) {
            if (map[i]) {
                std::size_t start = (i == sb) ? si : 0;
                std::size_t end   = (i == eb) ? ei : __buf_sz;
                for (std::size_t j = start; j < end; ++j) map[i][j].~T();
                delete[] map[i];
            }
        }
        delete[] map;
        map_sz = (ilist.size() + __buf_sz - 1) / __buf_sz + 2;
        map = new T*[map_sz]();
        sb = 1;
        eb = map_sz - 2;
        for (std::size_t i = 0; i < map_sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_sz];
        si = 0;
        ei = ilist.size() % __buf_sz;
        if (ei == 0 && ilist.size() != 0) ei = __buf_sz;
        auto k = ilist.begin();
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_sz;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(*k++);
        }
    }

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

    bool empty() const noexcept { return size() == 0; }
    std::size_t size() const noexcept { return (eb - sb) * __buf_sz + (ei - si); }
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
        T** new_map = new T*[need]();
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
        if (empty() || !map) {
            sb = eb = map_sz / 2;
            si = ei = 0;
            return;
        }
        for (T** i = map + sb; i <= map + eb; ++i) {
            std::size_t s = (i == map + sb) ? si : 0;
            std::size_t e = (i == map + eb) ? ei : __buf_sz;
            for (T* j = *i + s; j < *i + e; ++j) j->~T();
        }
        sb = eb = map_sz / 2;
        si = ei = 0;
    }

    iterator insert(const_iterator pos, const T& value) {
        if (eb < map_sz - 1 || ei < __buf_sz - 1) {
            for (iterator i = end(); i > pos; --i) *i = std::move(*(i - 1));
            *pos = value;
            if (ei != __buf_sz - 1) {
                ei++;
            } else {
                eb++;
                ei = 0;
            }
            return pos;
        }
        std::size_t index = pos - begin();
        std::size_t need = eb - sb + 3 + ((ei == __buf_sz - 1) ? 1 : 0);
        T** new_map = new T*[need]();
        for (std::size_t i = 1; i < eb - sb + 2; ++i) new_map[i] = map[sb + i - 1];
        for (T** i = map; i < map + sb; ++i) delete[] *i;
        for (T** i = map + eb + 1; i < map + map_sz; ++i) delete[] *i;
        delete[] map;
        map = new_map;
        sb = 2;
        eb = need - 2;
        map_sz = need;
        std::size_t new_pos = begin() + index;
        for (iterator i = end(); i > new_pos; --i) *i = std::move(*(i - 1));
        *new_pos = value;
        if (ei != __buf_sz - 1) {
            ei++;
        } else {
            eb++;
            ei = 0;
        }
        return new_pos;
    }

    iterator insert(const_iterator pos, T&& value) {
        if (eb < map_sz - 1 || ei < __buf_sz - 1) {
            for (iterator i = end(); i > pos; --i) *i = std::move(*(i - 1));
            *pos = std::move(value);
            if (ei != __buf_sz - 1) {
                ei++;
            } else {
                eb++;
                ei = 0;
            }
            return pos;
        }
        std::size_t index = pos - begin();
        std::size_t need = eb - sb + 3 + ((ei == __buf_sz - 1) ? 1 : 0);
        T** new_map = new T*[need]();
        for (std::size_t i = 1; i < eb - sb + 2; ++i) new_map[i] = map[sb + i - 1];
        for (T** i = map; i < map + sb; ++i) delete[] *i;
        for (T** i = map + eb + 1; i < map + map_sz; ++i) delete[] *i;
        delete[] map;
        map = new_map;
        sb = 2;
        eb = need - 2;
        map_sz = need;
        std::size_t new_pos = begin() + index;
        for (iterator i = end(); i > new_pos; --i) *i = std::move(*(i - 1));
        *new_pos = std::move(value);
        if (ei != __buf_sz - 1) {
            ei++;
        } else {
            eb++;
            ei = 0;
        }
        return new_pos;
    }

    template<std::input_iterator InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        if constexpr (std::random_access_iterator<InputIt>) {
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            if ((map_sz - eb) * __buf_sz - ei >= count) {
                for (iterator i = end() + count - 1; i > pos + count - 1; --i) *i = std::move(*(i - count));
                for (std::size_t i = 0; i < count; ++i) *(pos + i) = T(*(first + i));
                eb += (ei + count) / __buf_sz;
                ei += (ei + count) % __buf_sz;
                return pos;
            }
            std::size_t index = pos - begin();
            std::size_t need = eb - sb + 3 + (ei + count) / __buf_sz;
            T** new_map = new T*[need]();
            for (std::size_t i = 1; i < eb - sb + 2; ++i) new_map[i] = map[sb + i - 1];
            for (T** i = map; i < map + sb; ++i) delete[] *i;
            for (T** i = map + eb + 1; i < map + map_sz; ++i) delete[] *i;
            delete[] map;
            map = new_map;
            sb = 2;
            eb = need - 2;
            map_sz = need;
            std::size_t new_pos = begin() + index;
            for (iterator i = end() + count - 1; i > new_pos + count - 1; --i) *i = std::move(*(i - count));
            for (std::size_t i = 0; i < count; ++i) *(new_pos + i) = T(*(first + i));
            eb += (ei + count) / __buf_sz;
            ei += (ei + count) % __buf_sz;
            return new_pos;
        } else {
            Vector<T> tmp;
            for (; first != last; ++first) tmp.push_back(*first);
            return insert(pos, tmp.begin(), tmp.end());
        }
    }

    iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
        return insert(pos, ilist.begin(), ilist.end());
    }


    void swap(Deque& a, Deque& b) noexcept {
        std::swap(a.map, b.map);
        std::swap(a.map_sz, b.map_sz);
        std::swap(a.sb, b.sb);
        std::swap(a.si, b.si);
        std::swap(a.eb, b.eb);
        std::swap(a.ei, b.ei);
    }
};

