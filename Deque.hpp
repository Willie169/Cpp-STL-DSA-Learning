#pragma once

#include <cstddef>

template<class T>
class Deque {
    T** map;
    std::size_t sz, sb, si, eb, ei;
    static const size_t __buf_size = sizeof(T) < 512 ? size_t(512 / sizeof(T)) : size_t(1);

public:
    constexpr Deque() : map(nullptr), sz(0), sb(0), si(0), eb(0), ei(0);

    explicit constexpr Deque(std::size_t count) {
        sz = (count + __buf_size - 1) / __buf_size + 2;
        map = new T*[sz];
        sb = 1;
        eb = sz - 2;
        for (std::size_t i = 0; i < sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
        si = 0;
        ei = count % __buf_size;
        if (ei == 0 && count != 0) ei = __buf_size;
            for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_size;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T();
        }
    }

    constexpr Deque(std::size_t count, const T& value) {
        sz = (count + __buf_size - 1) / __buf_size + 2;
        map = new T*[sz];
        sb = 1;
        eb = sz - 2;
        for (std::size_t i = 0; i < sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
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
        std::size_t count = std::distance(first, last);
        sz = (count + __buf_size - 1) / __buf_size + 2;
        map = new T*[sz];
        for (std::size_t i = 0; i < sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
    si = 0;
        ei = count % __buf_size;
        if (ei == 0 && count != 0) ei = __buf_size;
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_size;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(*(first++));
        }
    }

    Deque(const Deque& other) : sz(other.sz), sb(other.sb), si(other.si), eb(other.eb), ei(other.ei) {
        map = new T*[sz];
        for (std::size_t i = 0; i < sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
    std::size_t k = 0;
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_size;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(other.map[i][j]);
        }
    }

    Deque(Deque&& other) : sz(other.sz), sb(other.sb), si(other.si), eb(other.eb), ei(other.ei) {
        map = new T*[sz];
        for (std::size_t i = 0; i < sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
        for (std::size_t i = sb; i <= eb; ++i) {
            std::size_t s = (i == sb) ? si : 0;
            std::size_t e = (i == eb) ? ei : __buf_size;
            for (std::size_t j = s; j < e; ++j) new (&map[i][j]) T(other.map[i][j]);
        }
    }

    Deque(std::initializer_list<T> ilist) {
        sz = (ilist.size() + __buf_size - 1) / __buf_size + 2;
        map = new T*[sz];
        sb = 1;
        eb = sz - 2;
        for (std::size_t i = 0; i < sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
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
            sz = other.sz;
            sb = other.sb;
            eb = other.eb;
            ei = other.ei;
            map = new T*[sz];
            for (std::size_t i = 0; i < sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
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
            sz = other.sz;
            sb = other.sb;
            eb = other.eb;
            ei = other.ei;
            map = other.map;
            other.map = nullptr;
            other.sz = 0;
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
        sz = (ilist.size() + __buf_size - 1) / __buf_size + 2;
        map = new T*[sz];
        sb = 1;
        eb = sz - 2;
        for (std::size_t i = 0; i < sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
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
        sz = (count + __buf_size - 1) / __buf_size + 2;
        map = new T*[sz];
        sb = 1;
        eb = sz - 2;
        for (std::size_t i = 0; i < sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
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
        std::size_t count = std::distance(first, last);
        sz = (count + __buf_size - 1) / __buf_size + 2;
        map = new T*[sz];
        for (std::size_t i = 0; i < sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
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
        sz = (ilist.size() + __buf_size - 1) / __buf_size + 2;
        map = new T*[sz];
        sb = 1;
        eb = sz - 2;
        for (std::size_t i = 0; i < sz; ++i) map[i] = (i < sb || i > eb) ? nullptr : new T[__buf_size];
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

};

