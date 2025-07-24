#pragma once

#ifndef VECTOR_GROW
#define VECTOR_GROW 2
#endif

#include <new>
#include <utility>
#include <cstddef>
#include <stdexcept>
#include <initializer_list>
#include <iterator>
#include <algorithm>
#include <type_traits>
#include <cstdint>

template<typename T>
class Vector {
private:
    T* elems;
    size_t sz;
    size_t cap;

public:
    constexpr Vector() : elems(nullptr), sz(0), cap(0) {}

    constexpr Vector(std::initializer_list<T> ilist) : Vector() {
        reserve(ilist.size());
        for (const auto& val : ilist) {
            new (elems + sz++) T(val);
        }
    }

    constexpr ~Vector() {
        delete[] elems;
    }

    constexpr Vector(const Vector& other) : elems(new T[other.cap]), sz(other.sz), cap(other.cap) {
        for (size_t i = 0; i < sz; ++i) elems[i] = other.elems[i];
    }

    constexpr Vector(Vector&& other) noexcept : elems(other.elems), sz(other.sz), cap(other.cap) {
        other.elems = nullptr;
        other.sz = 0;
        other.cap = 0;
    }

    constexpr Vector& operator=(const Vector& other) {
        if (this != &other) {
            T* new_elems = nullptr;
            if (other.cap > 0) {
                new_elems = new T[other.cap];
                for (size_t i = 0; i < other.sz; ++i) {
                    new_elems[i] = other.elems[i];
                }
            }
            delete[] elems;
            elems = new_elems;
            sz = other.sz;
            cap = other.cap;
        }
        return *this;
    }

    constexpr Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            delete[] elems;
            elems = other.elems;
            sz = other.sz;
            cap = other.cap;
            other.elems = nullptr;
            other.sz = 0;
            other.cap = 0;
        }
        return *this;
    }

    constexpr T& operator[](size_t index) { return elems[index]; }
    constexpr const T& operator[](size_t index) const { return elems[index]; }

    constexpr T& at(size_t index) {
        if (index >= sz) throw std::out_of_range("Vector");
        return elems[index];
    }

    constexpr const T& at(size_t index) const {
        if (index >= sz) throw std::out_of_range("Vector");
        return elems[index];
    }

    constexpr T& front() { return elems[0]; }
    constexpr const T& front() const { return elems[0]; }

    constexpr T& back() { return elems[sz - 1]; }
    constexpr const T& back() const { return elems[sz - 1]; }

    constexpr void reserve(size_t new_cap) {
        if (new_cap <= cap) return;
        if (new_cap > SIZE_MAX) throw std::length_error("Vector");
        T* new_elems = new T[new_cap];
        for (size_t i = 0; i < sz; ++i) new_elems[i] = std::move(elems[i]);
        delete[] elems;
        elems = new_elems;
        cap = new_cap;
    }

    constexpr void shrink_to_fit() {
        if (cap != sz) {
            T* new_elems = new T[sz];
            for (size_t i = 0; i < sz; ++i) new_elems[i] = std::move(elems[i]);
            delete[] elems;
            elems = new_elems;
            cap = sz;
        }
    }

    constexpr void push_back(const T& value) {
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        elems[sz++] = value;
    }

    constexpr void clear() {
        for (size_t i = 0; i < sz; ++i) {
            elems[i].~T();
        }
        sz = 0;
    }
    
    constexpr void pop_back() {
        if (sz > 0) {
            elems[--sz].~T();
        }
    }

    constexpr void resize(size_t new_size) {
        if (new_size > cap) {
            reserve(new_size);
        }
        if (new_size < sz) {
            for (size_t i = new_size; i < sz; ++i) {
                elems[i].~T();
            }
        } else if (new_size > sz) {
            for (size_t i = sz; i < new_size; ++i) {
                new (elems + i) T();
            }
        }
        sz = new_size;
    }
    
    constexpr void resize(size_t new_size, const T& value) {
        if (new_size > cap) {
            reserve(new_size);
        }
        if (new_size < sz) {
            for (size_t i = new_size; i < sz; ++i) {
                elems[i].~T();
            }
        } else if (new_size > sz) {
            for (size_t i = sz; i < new_size; ++i) {
                new (elems + i) T(value);
            }
        }
        sz = new_size;
    }

    constexpr void assign(size_t count, const T& value) {
        if (count > cap) {
            delete[] elems;
            elems = new T[count];
            cap = count;
        }
        for (size_t i = 0; i < count; ++i) elems[i] = value;
        sz = count;
    }

    template<class InputIt>
    requires (!std::is_integral_v<InputIt>)
    constexpr void assign(InputIt first, InputIt last) {
        size_t count = std::distance(first, last);
        if (count > cap) {
            delete[] elems;
            elems = new T[count];
            cap = count;
        }
        for (size_t i = 0; i < count; ++i) elems[i] = *(first++);
        sz = count;
    }

    constexpr void assign(std::initializer_list<T> ilist) {
        *this = Vector(ilist);
    }

    constexpr T* insert(const T* pos, const T& value) {
        size_t index = pos - elems;
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        for (size_t i = sz; i > index; --i) elems[i] = std::move(elems[i - 1]);
        elems[index] = value;
        ++sz;
        return elems + index;
    }

    constexpr T* insert(const T* pos, T&& value) {
        size_t index = pos - elems;
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        for (size_t i = sz; i > index; --i) elems[i] = std::move(elems[i - 1]);
        elems[index] = std::move(value);
        ++sz;
        return elems + index;
    }

    constexpr T* insert(const T* pos, size_t count, const T& value) {
        size_t index = pos - elems;
        reserve(std::max(sz ? sz * VECTOR_GROW : 1, sz + count));
        for (size_t i = sz; i > index; --i) elems[i + count - 1] = std::move(elems[i - 1]);
        for (size_t i = 0; i < count; ++i) elems[index + i] = value;
        sz += count;
        return elems + index;
    }

    template<class InputIt>
    requires (!std::is_integral_v<InputIt>)
    constexpr T* insert(const T* pos, InputIt first, InputIt last) {
        size_t index = pos - elems;
        size_t count = std::distance(first, last);
        reserve(std::max(sz ? sz * VECTOR_GROW : 1, sz + count));
        for (size_t i = sz; i > index; --i) elems[i + count - 1] = std::move(elems[i - 1]);
        for (size_t i = 0; i < count; ++i) elems[index + i] = *(first++);
        sz += count;
        return elems + index;
    }

    constexpr T* insert(const T* pos, std::initializer_list<T> ilist) {
        return insert(pos, ilist.begin(), ilist.end());
    }

    template<class... Args>
    constexpr T& emplace_back(Args&&... args) {
        if (sz == cap) {
            reserve(sz ? sz * VECTOR_GROW : 1);
        }
        new (elems + sz) T(std::forward<Args>(args)...);
        return elems[sz++];
    }
    
    template<class... Args>
    constexpr T* emplace(const T* pos, Args&&... args) {
        size_t index = pos - elems;
        if (sz == cap) {
            reserve(sz ? sz * VECTOR_GROW : 1);
        }
        for (size_t i = sz; i > index; --i) {
            new (elems + i) T(std::move(elems[i - 1]));
            elems[i - 1].~T();
        }
        new (elems + index) T(std::forward<Args>(args)...);
        ++sz;
        return elems + index;
    }
    
    constexpr T* erase(const T* pos) {
        if (sz == 0) return elems;    
        size_t index = pos - elems;
        if (index >= sz) return elems + sz;
        elems[index].~T();
        for (size_t i = index; i < sz - 1; ++i) {
            new (elems + i) T(std::move(elems[i + 1]));
            elems[i + 1].~T();
        } 
        --sz;
        return elems + index;
    }
    
    constexpr T* erase(const T* first, const T* last) {
        if (first == last) return const_cast<T*>(first);
        size_t start = first - elems;
        size_t end = last - elems;
        size_t count = end - start;
        if (start >= sz) return elems + sz;
        if (end > sz) end = sz;
        count = end - start;
        for (size_t i = start; i < end; ++i) {
            elems[i].~T();
        }
        for (size_t i = end; i < sz; ++i) {
            new (elems + i - count) T(std::move(elems[i]));
            elems[i].~T();
        }
        sz -= count;
        return elems + start;
    }

    constexpr void swap(Vector& other) noexcept {
        std::swap(elems, other.elems);
        std::swap(sz, other.sz);
        std::swap(cap, other.cap);
    }

    constexpr T* data() { return elems; }
    constexpr const T* data() const { return elems; }
    constexpr T* begin() { return elems; }
    constexpr const T* begin() const { return elems; }
    constexpr T* end() { return elems + sz; }
    constexpr const T* end() const { return elems + sz; }
    constexpr std::reverse_iterator<T*> rbegin() { return std::reverse_iterator<T*>(end()); }
    constexpr std::reverse_iterator<T*> rend() { return std::reverse_iterator<T*>(begin()); }
    constexpr size_t size() const { return sz; }
    constexpr size_t capacity() const { return cap; }
    constexpr bool empty() const { return sz == 0; }
};

