#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include "Vector.hpp"

template<class T, std::size_t __buf_sz>
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
        for (std::size_t i = sb; i <= eb; ++i) map[i] = static_cast<T*>(operator new(__buf_sz * sizeof(T)));
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
        for (std::size_t i = sb; i <= eb; ++i) map[i] = static_cast<T*>(operator new(__buf_sz * sizeof(T)));
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
            for (std::size_t i = sb; i <= eb; ++i) map[i] = static_cast<T*>(operator new(__buf_sz * sizeof(T)));
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
        for (std::size_t i = sb; i <= eb; ++i) map[i] = static_cast<T*>(operator new(__buf_sz * sizeof(T)));
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
        for (std::size_t i = sb; i <= eb; ++i) map[i] = static_cast<T*>(operator new(__buf_sz * sizeof(T)));
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
        for (std::size_t i = sb; i <= eb; ++i) { map[i] = static_cast<T*>(operator new(__buf_sz * sizeof(T))); }
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

    bool empty() const noexcept { 
        if (map_sz == 0) return true;
        return sb == eb && si >= ei; 
    }
    
    std::size_t size() const noexcept { 
        if (map_sz == 0) return 0;
        if (sb == eb) return ei - si;
        return (eb - sb - 1) * __buf_sz + __buf_sz - si + ei;
    }
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
            si = ei = __buf_sz / 2;
            return;
        }
        for (T** i = map + sb; i <= map + eb; ++i) {
            std::size_t s = (i == map + sb) ? si : 0;
            std::size_t e = (i == map + eb) ? ei : __buf_sz;
            for (T* j = *i + s; j < *i + e; ++j) j->~T();
        }
        sb = eb = map_sz / 2;
        si = ei = __buf_sz / 2;
    }

    iterator insert(const_iterator pos, const T& value) {
        if (pos > (size() - 1) / 2) {
            if (eb < map_sz - 1 || ei < __buf_sz - 1) {
                for (iterator i = end(); i > pos; --i) {
                    *i = std::move(*(i - 1));
                    (i - 1)->~T();
                }
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
            eb = eb - sb + 1;
            sb = 1;
            map_sz = need;
            iterator new_pos = begin() + index;
            for (iterator i = end(); i > new_pos; --i) {
                *i = std::move(*(i - 1));
                (i - 1)->~T();
            }
            *new_pos = value;
            if (ei != __buf_sz - 1) {
                ei++;
            } else {
                eb++;
                ei = 0;
            }
            return new_pos;
        }
        if (sb > 0 || si > 0) {
            for (iterator i = begin() - 1; i < pos; ++i) {
                *i = std::move(*(i + 1));
                (i + 1)->~T();
            }
            *pos = value;
            if (si != 0) {
                si--;
            } else {
                sb--;
                si = __buf_sz - 1;
            }
            return pos;
        }
        std::size_t index = pos - begin();
        std::size_t need = eb - sb + 3 + ((si == 0) ? 1 : 0);
        T** new_map = new T*[need]();
        for (std::size_t i = 1; i < eb - sb + 2; ++i) new_map[i] = map[sb + i - 1];
        for (T** i = map; i < map + sb; ++i) delete[] *i;
        for (T** i = map + eb + 1; i < map + map_sz; ++i) delete[] *i;
        delete[] map;
        map = new_map;
        eb = eb - sb + 1;
        sb = 1;
        map_sz = need;
        iterator new_pos = begin() + index;
        for (iterator i = begin() - 1; i < new_pos; ++i) {
            *i = std::move(*(i + 1));
            (i + 1)->~T();
        }
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
        if (pos > (size() - 1) / 2) {
            if (eb < map_sz - 1 || ei < __buf_sz - 1) {
                for (iterator i = end(); i > pos; --i) {
                    *i = std::move(*(i - 1));
                    (i - 1)->~T();
                }
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
            eb = eb - sb + 1;
            sb = 1;
            map_sz = need;
            iterator new_pos = begin() + index;
            for (iterator i = end(); i > new_pos; --i) {
                *i = std::move(*(i - 1));
                (i - 1)->~T();
            }
            *new_pos = value;
            if (ei != __buf_sz - 1) {
                ei++;
            } else {
                eb++;
                ei = 0;
            }
            return new_pos;
        }
        if (sb > 0 || si > 0) {
            for (iterator i = begin() - 1; i < pos; ++i) {
                *i = std::move(*(i + 1));
                (i + 1)->~T();
            }
            *pos = value;
            if (si != 0) {
                si--;
            } else {
                sb--;
                si = __buf_sz - 1;
            }
            return pos;
        }
        std::size_t index = pos - begin();
        std::size_t need = eb - sb + 3 + ((si == 0) ? 1 : 0);
        T** new_map = new T*[need]();
        for (std::size_t i = 1; i < eb - sb + 2; ++i) new_map[i] = map[sb + i - 1];
        for (T** i = map; i < map + sb; ++i) delete[] *i;
        for (T** i = map + eb + 1; i < map + map_sz; ++i) delete[] *i;
        delete[] map;
        map = new_map;
        eb = eb - sb + 1;
        sb = 1;
        map_sz = need;
        iterator new_pos = begin() + index;
        for (iterator i = begin() - 1; i < new_pos; ++i) {
            *i = std::move(*(i + 1));
            (i + 1)->~T();
        }
        *new_pos = value;
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
            if (pos > (size() - 1) / 2) {
                std::size_t count = static_cast<std::size_t>(std::distance(first, last));
                if ((map_sz - eb) * __buf_sz - ei >= count) {
                    for (iterator i = end() + count - 1; i >= pos + count; --i) {
                        *i = std::move(*(i - count));
                        (i - count)->~T();
                    }
                    for (std::size_t i = 0; i < count; ++i) *(pos + i) = T(*(first + i));
                    std::size_t tmp = ei + count;
                    eb += tmp / __buf_sz;
                    ei += tmp % __buf_sz;
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
                eb = eb - sb + 1;
                sb = 1;
                map_sz = need;
                iterator new_pos = begin() + index;
                for (iterator i = end() + count - 1; i >= new_pos + count; --i) {
                    *i = std::move(*(i - count));
                    (i - count)->~T();
                }
                for (std::size_t i = 0; i < count; ++i) *(new_pos + i) = T(*(first + i));
                std::size_t tmp = ei + count;
                eb += tmp / __buf_sz;
                ei += tmp % __buf_sz;
                return new_pos;
            }
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            if (sb * __buf_sz + si >= count) {
                for (iterator i = begin() - count; i < pos - count; --i) {
                    *i = std::move(*(i + count));
                    (i + count)->~T();
                }
                for (std::size_t i = 0; i < count; ++i) *(pos + i) = T(*(first + i));
                std::size_t tmp = __buf_sz - si + count;
                sb -= tmp / __buf_sz;
                ei -= tmp % __buf_sz;
                return pos;
            }
            std::size_t tmp = __buf_sz - si + count;
            std::size_t index = pos - begin();
            std::size_t need = eb - sb + 3 + tmp / __buf_sz;
            T** new_map = new T*[need]();
            for (std::size_t i = 1; i < eb - sb + 2; ++i) new_map[i] = map[sb + i - 1];
            for (T** i = map; i < map + sb; ++i) delete[] *i;
            for (T** i = map + eb + 1; i < map + map_sz; ++i) delete[] *i;
            delete[] map;
            map = new_map;
            eb = eb - sb + 1;
            sb = 1;
            map_sz = need;
            iterator new_pos = begin() + index;
            for (iterator i = begin() - count; i < pos - count; --i) {
                *i = std::move(*(i + count));
                (i + count)->~T();
            }
            for (std::size_t i = 0; i < count; ++i) *(new_pos + i) = T(*(first + i));
            sb -= tmp / __buf_sz;
            ei -= tmp % __buf_sz;
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
        if (eb < map_sz - 1 || ei < __buf_sz - 1) {
            for (iterator i = end(); i > pos; --i) {
                *i = std::move(*(i - 1));
                (i - 1)->~T();
            }
            new (&*pos) T(std::forward<Args>(args)...);
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
        iterator new_pos = begin() + index;
        for (iterator i = end(); i > new_pos; --i) {
            *i = std::move(*(i - 1));
            (i - 1)->~T();
        }
        new (&*new_pos) T(std::forward<Args>(args)...);
        if (ei != __buf_sz - 1) {
            ei++;
        } else {
            eb++;
            ei = 0;
        }
        return new_pos;
    }

    iterator erase(const_iterator pos) {
        for (iterator i = pos; i < end() - 1; ++i) {
            *i = std::move(*(i + 1));
            (i + 1)->~T();
        }
        if (ei != 0) ei --;
        else {
            ei = __buf_sz - 1;
            eb--;   
        }   
        return pos;
    }

    iterator erase(const_iterator first, const_iterator last) {
        std::size_t count = last - first;
        for (iterator i = first; i < end() - count; ++i) {
            *i = std::move(*(i + count));
            (i + count)->~T();
        }
        if (count > 0) {
            if (ei >= count) ei -= count;
            else {
                std::size_t tmp = count - ei;
                ei = __buf_sz - tmp % __buf_sz;
                eb -= (tmp + __buf_sz - 1) / __buf_sz;
            }
        }
        return first;
    }

    void push_back(const T& value) {
        if (map_sz == 0) {
            map_sz = 3;
            map = new T*[map_sz]();
            sb = eb = 1;
            si = ei = __buf_sz / 2;
            map[1] = new T[__buf_sz];
        }
        if (ei != __buf_sz - 1) ei++;
        else if (eb != map_sz - 1) {
            eb++;
            ei = 0;
        } else {
            std::size_t need = eb - sb + 4;
            T** new_map = new T*[need]();
            for (std::size_t i = 1; i < eb - sb + 2; ++i) new_map[i] = map[sb + i - 1];
            for (T** i = map; i < map + sb; ++i) delete[] *i;
            for (T** i = map + eb + 1; i < map + map_sz; ++i) delete[] *i;
            delete[] map;
            map = new_map;
            eb = eb - sb + 1;
            sb = 1;
            map_sz = need;
            eb++;
            ei = 0;
        }
        new (&map[eb][ei]) T(value);
    }

    void push_back(T&& value) {
        if (map_sz == 0) {
            map_sz = 3;
            map = new T*[map_sz]();
            sb = eb = 1;
            si = ei = __buf_sz / 2;
            map[1] = new T[__buf_sz];
        }
        if (ei != __buf_sz - 1) ei++;
        else if (eb != map_sz - 1) {
            eb++;
            ei = 0;
        } else {
            std::size_t need = eb - sb + 4;
            T** new_map = new T*[need]();
            for (std::size_t i = 1; i < eb - sb + 2; ++i) new_map[i] = map[sb + i - 1];
            for (T** i = map; i < map + sb; ++i) delete[] *i;
            for (T** i = map + eb + 1; i < map + map_sz; ++i) delete[] *i;
            delete[] map;
            map = new_map;
            eb = eb - sb + 1;
            sb = 1;
            map_sz = need;
            eb++;
            ei = 0;
        }
        new (&map[eb][ei]) T(std::move(value));
    }

    template<class... Args>
    reference emplace_back(Args&&... args) {
        if (map_sz == 0) {
            map_sz = 3;
            map = new T*[map_sz]();
            sb = eb = 1;
            si = ei = __buf_sz / 2;
            map[1] = new T[__buf_sz];
        }
        if (ei != __buf_sz - 1) new (&(map[eb][++ei])) T(std::forward<Args>(args)...);
        else if (eb != map_sz - 1) {
            if (!map[++eb]) map[eb] = new T[__buf_sz];
            new (&(map[eb][0])) T(std::forward<Args>(args)...);
            ei = 0;
        } else {
            std::size_t need = eb - sb + 4;
            T** new_map = new T*[need]();
            for (std::size_t i = 1; i < eb - sb + 2; ++i) new_map[i] = map[sb + i - 1];
            for (T** i = map; i < map + sb; ++i) delete[] *i;
            for (T** i = map + eb + 1; i < map + map_sz; ++i) delete[] *i;
            delete[] map;
            map = new_map;
            eb = eb - sb + 1;
            sb = 1;
            map_sz = need;
            if (!map[++eb]) map[eb] = new T[__buf_sz];
            new (&(map[eb][0])) T(std::forward<Args>(args)...);
            ei = 0;
        }
        return map[eb][ei];
    }

    void pop_back() {
        map[eb][ei].~T();
        if (ei != 0) {
            ei--;
        } else {
            eb--;
            ei = __buf_sz - 1;
        }
    }

    void push_front(const T& value) {
        if (map_sz == 0) {
            map_sz = 3;
            map = new T*[map_sz]();
            sb = eb = 1;
            si = ei = __buf_sz / 2;
            map[1] = new T[__buf_sz];
        }
        if (si != 0) si--;
        else if (sb != 0) {
            sb--;
            si = __buf_sz - 1;
        } else {
            std::size_t need = eb - sb + 4;
            T** new_map = new T*[need]();
            for (std::size_t i = 1; i < eb - sb + 2; ++i) new_map[i] = map[sb + i - 1];
            for (T** i = map; i < map + sb; ++i) delete[] *i;
            for (T** i = map + eb + 1; i < map + map_sz; ++i) delete[] *i;
            delete[] map;
            map = new_map;
            eb = eb - sb + 1;
            sb = 1;
            map_sz = need;
            sb--;
            si = __buf_sz - 1;
        }
        new (&map[sb][si]) T(value);
    }

    void push_front(T&& value) {
        if (map_sz == 0) {
            map_sz = 3;
            map = new T*[map_sz]();
            sb = eb = 1;
            si = ei = __buf_sz / 2;
            map[1] = new T[__buf_sz];
        }
        if (si != 0) si--;
        else if (sb != 0) {
            sb--;
            si = __buf_sz - 1;
        } else {
            std::size_t need = eb - sb + 4;
            T** new_map = new T*[need]();
            for (std::size_t i = 1; i < eb - sb + 2; ++i) new_map[i] = map[sb + i - 1];
            for (T** i = map; i < map + sb; ++i) delete[] *i;
            for (T** i = map + eb + 1; i < map + map_sz; ++i) delete[] *i;
            delete[] map;
            map = new_map;
            eb = eb - sb + 1;
            sb = 1;
            map_sz = need;
            sb--;
            si = __buf_sz - 1;
        }
        new (&map[sb][si]) T(std::move(value));
    }

    template<class... Args>
    reference emplace_front(Args&&... args) {
        if (map_sz == 0) {
            map_sz = 3;
            map = new T*[map_sz]();
            sb = eb = 1;
            si = ei = __buf_sz / 2;
            map[1] = new T[__buf_sz]; 
        }
        if (si != 0) new (&(map[sb][--si])) T(std::forward<Args>(args)...);
        else if (sb != 0) {
            if (!map[--sb]) map[sb] = new T[__buf_sz];
            new (&(map[sb][__buf_sz - 1])) T(std::forward<Args>(args)...);
            si = __buf_sz - 1;
        } else {
            std::size_t need = eb - sb + 4;
            T** new_map = new T*[need]();
            for (std::size_t i = 1; i < eb - sb + 2; ++i) new_map[i] = map[sb + i - 1];
            for (T** i = map; i < map + sb; ++i) delete[] *i;
            for (T** i = map + eb + 1; i < map + map_sz; ++i) delete[] *i;
            delete[] map;
            map = new_map;
            eb = eb - sb + 1;
            sb = 1;
            map_sz = need;
            if (!map[--sb]) map[sb] = new T[__buf_sz];
            new (&(map[sb][__buf_sz - 1])) T(std::forward<Args>(args)...);
            si = __buf_sz - 1;
        }
        return map[sb][si];
    }

    void pop_front() {
        map[sb][si].~T();
        if (si != __buf_sz - 1) {
            si++;
        } else {
            sb++;
            si = 0;
        }
    }

    void resize(std::size_t count) {
        std::size_t tmp = size();
        if (count == tmp) return;
        if (count < tmp) {
            for (iterator i = end() - 1; i >= begin() + count; --i) i->~T();
            std::size_t index = si + count - 1;
            ei = index % __buf_sz;
            eb = sb + index / __buf_sz;
            return;
        }
        std::size_t need = (si + count - 1) / __buf_sz;
        if (map_sz < need) {
            T** new_map = new T*[need + 2]();
            for (std::size_t i = 1; i < eb - sb + 1; ++i) new_map[i] = map[sb + i - 1];
            for (T** i = map; i < map + sb; ++i) delete[] *i;
            for (T** i = map + eb + 1; i < map + map_sz; ++i) delete[] *i;
            delete[] map;
            map = new_map;
            eb = eb - sb + 1;
            sb = 1;
            map_sz = need;
        }
        for (std::size_t i = 0; i < count - tmp; ++i) {
            if (ei != __buf_sz - 1) ei++;
            else {
                eb++;
                ei = 0;
            }
            new (&map[eb][ei]) T();
        }
    }

    void resize(std::size_t count, const T& value) {
        std::size_t tmp = size();
        if (count == tmp) return;
        if (count < tmp) {
            for (iterator i = end() - 1; i >= begin() + count; --i) i->~T();
            std::size_t index = si + count - 1;
            ei = index % __buf_sz;
            eb = sb + index / __buf_sz;
            return;
        }
        std::size_t need = (si + count - 1) / __buf_sz;
        if (map_sz < need) {
            T** new_map = new T*[need + 2]();
            for (std::size_t i = 1; i < eb - sb + 1; ++i) new_map[i] = map[sb + i - 1];
            for (T** i = map; i < map + sb; ++i) delete[] *i; 
            for (T** i = map + eb + 1; i < map + map_sz; ++i) delete[] *i;
            delete[] map;
            map = new_map; 
            eb = eb - sb + 1;
            sb = 1;
            map_sz = need;
        }
        for (std::size_t i = 0; i < count - tmp; ++i) {
            if (ei != __buf_sz - 1) ei++;
            else {
                eb++;
                ei = 0;
            }
            new (&map[eb][ei]) T(value);
        }
    }

    void swap(Deque& other) noexcept {
        std::swap(map, other.map);
        std::swap(map_sz, other.map_sz);
        std::swap(sb, other.sb);
        std::swap(si, other.si);
        std::swap(eb, other.eb);
        std::swap(ei, other.ei);
    }
};

