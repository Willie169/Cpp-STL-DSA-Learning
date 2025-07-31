#pragma once

#ifndef VECTOR_GROW
#define VECTOR_GROW 2
#endif

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <iterator>
#include <algorithm>
#include <utility>
#include <initializer_list>
#include <type_traits>
#include <utility>

template<class T>
class Vector {
    T* elems;
    std::size_t sz;
    std::size_t cap;

    constexpr void __new_reserve(std::size_t new_cap) {
        if (new_cap > SIZE_MAX) throw std::length_error("Vector");
        T* new_elems = new T[new_cap];
        delete[] elems;
        elems = new_elems;
        cap = new_cap;
    }

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr Vector() : elems(nullptr), sz(0), cap(0) {}

    explicit Vector(std::size_t count) : sz(count) {
        __new_reserve(count);
    }

    constexpr Vector(std::size_t count, const T& value) : sz(count) {
        __new_reserve(count);
	    for (T& elem : *this) elem = value;
    }

    template<std::input_iterator InputIt>
    constexpr Vector(InputIt first, InputIt last) {
        if constexpr (std::random_access_iterator<InputIt>) {
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            __new_reserve(count);
            for (T& elem : *this) elem = *(first++);
            sz = count;
        } else {
            Vector(1);
            for (; first != last; ++first) push_back(*first);
        }
    }

    constexpr Vector(const Vector& other) : elems(static_cast<T*>(operator new[](other.cap * sizeof(T)))), sz(other.sz), cap(other.cap) { std::uninitialized_copy(other.elems, other.elems + sz, elems); }

    constexpr Vector(Vector&& other) noexcept : elems(std::exchange(other.elems, nullptr)), sz(std::exchange(other.sz, 0)), cap(std::exchange(other.cap, 0)) {}

    Vector(std::initializer_list<T> ilist) : Vector() {
        __new_reserve(ilist.size());
        for (const auto& value : ilist) elems[sz++] = T(value);
    }

    constexpr ~Vector() {
        if (!elems) return;
        delete[] elems;
    }

    constexpr Vector& operator=(const Vector& other) {
        if (this != &other) {
            Vector tmp(other);
            swap(tmp;)
        }
        return *this;
    }

    constexpr Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            delete[] elems;
            elems = std::exchange(other.elems, nullptr);
            sz = std::exchange(other.sz, 0);
            cap = std::exchange(other.cap, 0);
        }
        return *this;
    }

    constexpr Vector& operator=(std::initializer_list<T> ilist) {
        sz = 0;
	    if (ilist.size() > cap) __new_reserve(ilist.size());
        for (T& value : ilist) elems[sz++] = T(value);
        return *this;
    }

    constexpr void assign(std::size_t count, const T& value) {
        if (count > cap) {
            delete[] elems;
            elems = new T[count];
            cap = count;
        }
        for (T& elem : *this) elem = value;
        sz = count;
    }

    template<std::input_iterator InputIt>
    constexpr void assign(InputIt first, InputIt last) {
        if constexpr (std::random_access_iterator<InputIt>) {
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            if (count > cap) __new_reserve(count);
            for (T& elem : *this) elem = *(first++);
            sz = count;
        } else {
            for (; first != last; ++first) push_back(*first);
        }
    }

    constexpr void assign(std::initializer_list<T> ilist) { *this = Vector(ilist); }

    constexpr T& at(std::size_t index) {
        if (index >= sz) throw std::out_of_range("Vector");
        return elems[index];
    }

    constexpr const T& at(std::size_t index) const {
        if (index >= sz) throw std::out_of_range("Vector");
        return elems[index];
    }

    constexpr T& operator[](std::size_t index) { return elems[index]; }
    constexpr const T& operator[](std::size_t index) const { return elems[index]; }

    constexpr T& front() { return elems[0]; }
    constexpr const T& front() const { return elems[0]; }

    constexpr T& back() { return elems[sz - 1]; }
    constexpr const T& back() const { return elems[sz - 1]; }

    constexpr T* data() { return elems; }
    constexpr const T* data() const { return elems; }

    constexpr T* begin() noexcept { return elems; }
    constexpr const T* begin() const noexcept { return elems; }
    constexpr const T* cbegin() const noexcept { return elems; }

    constexpr T* end() { return elems + sz; }
    constexpr const T* end() const noexcept { return elems + sz; }
    constexpr const T* cend() const noexcept { return elems + sz; }

    constexpr std::reverse_iterator<T*> rbegin() noexcept { return std::reverse_iterator<T*>(end()); }
    constexpr std::reverse_iterator<const T*> rbegin() const noexcept { return std::reverse_iterator<const T*>(cend()); }
    constexpr std::reverse_iterator<const T*> crbegin() const noexcept { return std::reverse_iterator<const T*>(cend()); }

    constexpr std::reverse_iterator<T*> rend() noexcept { return std::reverse_iterator<T*>(begin()); }
    constexpr std::reverse_iterator<const T*> rend() const noexcept { return std::reverse_iterator<const T*>(cbegin()); }
    constexpr std::reverse_iterator<const T*> crend() const noexcept { return std::reverse_iterator<const T*>(cbegin()); }

    constexpr bool empty() const noexcept { return sz == 0; }
    constexpr std::size_t size() const noexcept { return sz; }
    constexpr std::size_t max_size() const noexcept { return sz; }

    constexpr void reserve(std::size_t new_cap) {
        if (new_cap <= cap) return;
        if (new_cap > SIZE_MAX) throw std::length_error("Vector");
        T* new_elems = new T[new_cap];
        for (std::size_t i = 0; i < sz; ++i) new_elems[i] = std::move(elems[i]);
        delete[] elems;
        elems = new_elems;
        cap = new_cap;
    }

    constexpr std::size_t capacity() const noexcept { return cap; }
    
    constexpr void shrink_to_fit() {
        if (cap != sz) {
            T* new_elems = new T[sz];
            for (std::size_t i = 0; i < sz; ++i) new_elems[i] = std::move(elems[i]);
            delete[] elems;
            elems = new_elems;
            cap = sz;
        }
    }

    constexpr void clear() {
        if (empty() || !elems) return;
        for (T& elem : *this) elem.~T();
        sz = 0;
    }

    constexpr T* insert(const T* pos, const T& value) {
        std::size_t index = pos - elems;
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        for (T* elem = end(); elem > begin(); --elem) elem = std::move(elem[-1]);
        elems[index] = value;
        ++sz;
        return elems + index;
    }

    constexpr T* insert(const T* pos, T&& value) {
        std::size_t index = pos - elems;
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        for (T* elem = end(); elem > begin(); --elem) *elem = std::move(elem[-1]);
        elems[index] = std::move(value);
        ++sz;
        return elems + index;
    }

    constexpr T* insert(const T* pos, std::size_t count, const T& value) {
        std::size_t index = pos - elems;
        reserve(std::max(sz ? sz * VECTOR_GROW : 1, sz + count));
        for (T* elem = end(); elem > elems + index; --elem) elem[count - 1] = std::move(elem[-1]);
        for (T* elem = begin(); elem < begin() + count; ++elem) elem[index] = value;
        sz += count;
        return elems + index;
    }

    template<std::input_iterator InputIt>
    constexpr T* insert(const T* pos, InputIt first, InputIt last) {
        if constexpr (std::random_access_iterator<InputIt>) {
            std::size_t index = pos - elems;
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            reserve(std::max(sz ? sz * VECTOR_GROW : 1, sz + count));
            for (T* elem = end(); elem > elems + index; --elem) elem[count - 1] = std::move(elem[-1]);
            for (T* elem = begin(); elem < begin() + count; ++elem) elem[index] = *(first++);
            sz += count;
            return elems + index;
        } else {
            Vector<T> tmp;
            for (; first != last; ++first) tmp.push_back(*first);
            return insert(pos, tmp.begin(), tmp.end());
        }
    }

    constexpr T* insert(const T* pos, std::initializer_list<T> ilist) { return insert(pos, ilist.begin(), ilist.end()); }

    template<class... Args>
    constexpr T* emplace(const T* pos, Args&&... args) {
        std::size_t index = pos - elems;
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        for (T* elem = end(); elem > elems + index; --elem) {
            new (elem) T(std::move(elem[-1]));
            elem[-1].~T();
        }
        new (elems + index) T(std::forward<Args>(args)...);
        ++sz;
        return elems + index;
    }
 
    constexpr T* erase(const T* pos) {
        if (sz == 0) return elems;    
        std::size_t index = pos - elems;
        if (index >= sz) return elems + sz;
        elems[index].~T();
        for (T* elem = elems + index; elem < end(); ++elem) {
            new (elem) T(std::move(elem[1]));
            elem[1].~T();
        } 
        --sz;
        return elems + index;
    }
    
    constexpr T* erase(const T* first, const T* last) {
        if (first == last) return const_cast<T*>(first);
        std::size_t start = first - elems;
        std::size_t end = last - elems;
        if (start >= sz) return elems + sz;
        if (end > sz) end = sz;
        std::size_t count = end - start;
        for (std::size_t i = start; i < end; ++i) elems[i].~T();
        for (std::size_t i = end; i < sz; ++i) {
            new (elems + i - count) T(std::move(elems[i]));
            elems[i].~T();
        }
        sz -= count;
        return elems + start;
    }

    constexpr void push_back(const T& value) {
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        elems[sz++] = value;
    }

    constexpr void push_back(T&& value) {
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        elems[sz++] = value;
    }

    template<class... Args>
    constexpr T& emplace_back(Args&&... args) {
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        new (elems + sz) T(std::forward<Args>(args)...);
        return elems[sz++];
    }

    constexpr void pop_back() {
        if (sz > 0) elems[--sz].~T();
    }

    constexpr void resize(std::size_t new_size) {
        if (new_size > cap) reserve(new_size);
        if (new_size < sz) {
            for (T* elem = elems + new_size; elem < end(); ++elem) elem->~T();
        } else if (new_size > sz) {
            for (T* elem = end(); elem < elems + new_size; ++elem) new (elem) T();
        }
        sz = new_size;
    }
    
    constexpr void resize(std::size_t new_size, const T& value) {
        if (new_size > cap) reserve(new_size);
                if (new_size < sz) {
            for (T* elem = elems + new_size; elem < end(); ++elem) elem->~T();
        } else if (new_size > sz) {
            for (T* elem = end(); elem < elems + new_size; ++elem) new (elem) T(value);
        }
        sz = new_size;
    }
   
    constexpr void swap(Vector& other) {
        std::swap(elems, other.elems);
        std::swap(sz, other.sz);
        std::swap(cap, other.cap);
    }
};

