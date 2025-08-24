#pragma once

#ifndef _MYSTD_VECTOR_GROW
#define _MYSTD_VECTOR_GROW 2
#endif

#include <algorithm>
#include <climits>
#include <compare>
#include <cstddef>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <memory_resource>
#include <stdexcept>
#include <type_traits>
#include <utility>


namespace mystd {

template<class T, class Allocator = std::allocator<T>>
requires std::is_same_v<T, typename Allocator::value_type>
class vector {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    [[no_unique_address]] Allocator alloc;
    T* elems;
    std::size_t sz;
    std::size_t cap;

    constexpr void destroy_deallocate() {
        if (elems) {
            if constexpr (!std::is_trivially_copyable_v<T>)
                std::destroy(elems, elems + sz);
            std::allocator_traits<Allocator>::deallocate(alloc, elems, cap);
        }
        elems = nullptr;
        cap = 0;
    }

public:
    constexpr vector() noexcept(noexcept(Allocator())) : alloc(Allocator()), elems(nullptr), sz(0), cap(0) {}
    explicit constexpr vector(const Allocator& alloc_) noexcept : alloc(alloc_), elems(nullptr), sz(0), cap(0) {}

    explicit vector(std::size_t count, const Allocator& alloc_ = Allocator()) : alloc(alloc_), sz(count), cap(count) {
        if (count == 0) elems = nullptr;
        else {
            elems = std::allocator_traits<Allocator>::allocate(alloc, count);
            if constexpr (!std::is_trivially_copyable_v<T>)
                for (T* i = elems; i < elems + count; ++i) std::allocator_traits<Allocator>::construct(alloc, i);
        }
    }

    constexpr vector(std::size_t count, const T& value, const Allocator& alloc_ = Allocator()) : alloc(alloc_), sz(count), cap(count) {
        if (count == 0) elems = nullptr;
        else {
            elems = std::allocator_traits<Allocator>::allocate(alloc, count);
            if constexpr (std::is_trivially_copyable_v<T>)
                for (T* i = elems; i < elems + count; ++i) *i = value;
            else
                for (T* i = elems; i < elems + count; ++i) std::allocator_traits<Allocator>::construct(alloc, i, value);
        }
    }

    template<std::input_iterator InputIt>
    constexpr vector(InputIt first, InputIt last, const Allocator& alloc_ = Allocator()) : alloc(alloc_), elems(nullptr), sz(0), cap(0) {
        if constexpr (std::forward_iterator<InputIt>) {
            if (first == last) return;
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            if (count > 0) {
                elems = std::allocator_traits<Allocator>::allocate(alloc, count);
                sz = cap = count;
                if constexpr (std::is_trivially_copyable_v<T> && std::contiguous_iterator<InputIt>)
                    std::memmove(elems, &*first, count * sizeof(T));
                else {
                    T* p = elems;
                    try { for (; first != last; ++first, ++p) std::allocator_traits<Allocator>::construct(alloc, p, *first); }
                    catch (...) { std::destroy(elems, p); std::allocator_traits<Allocator>::deallocate(alloc, elems, cap); throw; }
                }
            }
        } else for (; first != last; ++first) push_back(*first);
    }

    constexpr vector(const vector& other) : alloc(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.alloc)), elems(nullptr), sz(other.sz), cap(other.cap) {
        if (cap > 0) {
            elems = std::allocator_traits<Allocator>::allocate(alloc, cap);
            if constexpr (std::is_trivially_copyable_v<T>) std::memmove(elems, other.elems, sz * sizeof(T));
            else {
                T* p = elems;
                try { for (T* j = other.elems; j < other.elems + sz; ++j, ++p) std::allocator_traits<Allocator>::construct(alloc, p, *j); }
                catch (...) { std::destroy(elems, p); std::allocator_traits<Allocator>::deallocate(alloc, elems, cap); throw; }
            }
        }
    }

    constexpr vector(vector&& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || std::allocator_traits<Allocator>::is_always_equal::value) : alloc(std::move(other.alloc)), elems(nullptr), sz(0), cap(0) {
        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || std::allocator_traits<Allocator>::is_always_equal::value || (requires { { alloc == other.alloc } -> std::convertible_to<bool>; } && (alloc == other.alloc))) {
            elems = std::exchange(other.elems, nullptr);
            sz = std::exchange(other.sz, 0);
            cap = std::exchange(other.cap, 0);
        } else {
            if (other.sz > 0) {
                elems = std::allocator_traits<Allocator>::allocate(alloc, other.sz);
                sz = cap = other.sz;
                if constexpr (std::is_trivially_move_constructible_v<T>) std::memmove(elems, other.elems, sz * sizeof(T));
                else {
                    T* p = elems;
                    try { for (T* j = other.elems; j < other.elems + sz; ++j, ++p) std::allocator_traits<Allocator>::construct(alloc, p, std::move(*j)); }
                    catch (...) { std::destroy(elems, p); std::allocator_traits<Allocator>::deallocate(alloc, elems, cap); throw; }
                    std::destroy(other.elems, other.elems + sz);
                }
                other.elems = nullptr;
                other.sz = other.cap = 0;
            }
        }
    }

    constexpr vector(const vector& other, const Allocator& alloc_) : alloc(alloc_), elems(nullptr), sz(other.sz), cap(other.cap) {
        if (cap > 0) {
            elems = std::allocator_traits<Allocator>::allocate(alloc, cap);
            if constexpr (std::is_trivially_copyable_v<T>) std::memmove(elems, other.elems, sz * sizeof(T));
            else {
                T* p = elems;
                try { for (T* j = other.elems; j < other.elems + sz; ++j, ++p) std::allocator_traits<Allocator>::construct(alloc, p, *j); }
                catch (...) { std::destroy(elems, p); std::allocator_traits<Allocator>::deallocate(alloc, elems, cap); throw; }
            }
        }
    }

    vector(std::initializer_list<T> ilist, const Allocator& alloc_ = Allocator()) : alloc(alloc_), elems(nullptr), sz(ilist.size()), cap(ilist.size()) {
        if (cap > 0) {
            elems = std::allocator_traits<Allocator>::allocate(alloc, cap);
            if constexpr (std::is_trivially_copyable_v<T>) std::memmove(elems, ilist.begin(), sz * sizeof(T));
            else {
                T* p = elems;
                try { for (const T* j = ilist.begin(); j != ilist.end(); ++j, ++p) std::allocator_traits<Allocator>::construct(alloc, p, *j); }
                catch (...) { std::destroy(elems, p); std::allocator_traits<Allocator>::deallocate(alloc, elems, cap); throw; }
            }
        }
    }

    constexpr ~vector() { destroy_deallocate(); }

    constexpr vector& operator=(const vector& other) {
        if (this != &other) {
            if constexpr (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value && (requires { { alloc != other.alloc } -> std::convertible_to<bool>; } && (alloc != other.alloc))) {
                destroy_deallocate();
                alloc = other.alloc;
                if (cap < other.sz) { elems = std::allocator_traits<Allocator>::allocate(alloc, other.sz); cap = other.sz; }
            } else if (other.sz > cap) { if (elems) destroy_deallocate(); elems = std::allocator_traits<Allocator>::allocate(alloc, other.sz); cap = other.sz; }
            else if constexpr (!std::is_trivially_copyable_v<T>) std::destroy(elems + other.sz, elems + sz);
            if constexpr (std::is_trivially_copyable_v<T>) std::memcpy(elems, other.elems, other.sz * sizeof(T));
            else if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) { T* p = elems; try { for (T* i = other.elems; i < other.elems + other.sz; ++i, ++p) std::allocator_traits<Allocator>::construct(alloc, p, std::move(*i)); } catch (...) { std::destroy(elems, p); throw; } }
            else { T* p = elems; try { for (T* i = other.elems; i < other.elems + other.sz; ++i, ++p) if (i < elems + sz) *p = *i; else std::allocator_traits<Allocator>::construct(alloc, p, *i); } catch (...) { std::destroy(elems, p); throw; } }
            sz = other.sz;
        }
        return *this;
    }

    constexpr vector& operator=(vector&& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || std::allocator_traits<Allocator>::is_always_equal::value) {
        if (this != &other) {
            if constexpr (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || std::allocator_traits<Allocator>::is_always_equal::value || (requires { { alloc == other.alloc } -> std::convertible_to<bool>; } && (alloc == other.alloc))) {
                destroy_deallocate();
                alloc = std::move(other.alloc);
                elems = std::exchange(other.elems, nullptr);
                sz = std::exchange(other.sz, 0);
                cap = std::exchange(other.cap, 0);
            } else {
                if (other.sz > cap) { if (elems) destroy_deallocate(); elems = std::allocator_traits<Allocator>::allocate(alloc, other.sz); cap = other.sz; }
                else if constexpr (!std::is_trivially_copyable_v<T>) std::destroy(elems + other.sz, elems + sz);
                if constexpr (std::is_trivially_copyable_v<T>) std::memcpy(elems, other.elems, other.sz * sizeof(T));
                else { T* p = elems; try { for (T* i = other.elems; i < other.elems + other.sz; ++i, ++p) std::allocator_traits<Allocator>::construct(alloc, p, std::move(*i)); } catch (...) { std::destroy(elems, p); throw; } }
                sz = other.sz;
                other.~vector();
            }
        }
        return *this;
    }

    constexpr vector& operator=(std::initializer_list<T> ilist) {
        assign(ilist.begin(), ilist.end());
        return *this;
    }

    constexpr void assign(std::size_t count, const T& value) {
        if (count > cap) {
            destroy_deallocate();
            sz = 0;
            elems = std::allocator_traits<Allocator>::allocate(alloc, count);
            cap = count;
        }
        if constexpr (std::is_trivially_copyable_v<T>) {
            for (T* i = elems; i < elems + count; ++i) *i = value;
        } else {
            for (T* i = elems; i < elems + count; ++i) {
                if (i < elems + sz) *i = value;
                else std::allocator_traits<Allocator>::construct(alloc, i, value);
            }
            if (sz > count) std::destroy(elems + count, elems + sz);
        }
        sz = count;
    }

    template<std::input_iterator InputIt>
    constexpr void assign(InputIt first, InputIt last) {
        if constexpr (std::forward_iterator<InputIt>) {
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            if (count > cap) {
                destroy_deallocate();
                sz = 0;
                elems = std::allocator_traits<Allocator>::allocate(alloc, count);
                cap = count;
            }
            if constexpr (std::is_trivially_copyable_v<T>) {
                if constexpr (std::contiguous_iterator<InputIt>) {
                    std::memmove(elems, &*first, count * sizeof(T));
                } else {
                    std::copy(first, last, elems);
                }
            } else {
                for (T* i = elems; i < elems + count; ++i, ++first) {
                    if (i < elems + sz) *i = *first;
                    else std::allocator_traits<Allocator>::construct(alloc, i, *first);
                }
                if (sz > count) std::destroy(elems + count, elems + sz);
            }
            sz = count;
        } else {
            clear();
            for (; first != last; ++first) push_back(*first);
        }
    }

    constexpr void assign(std::initializer_list<T> ilist) { assign(ilist.begin(), ilist.end()); }

    constexpr allocator_type get_allocator() const noexcept { return alloc; }

    constexpr T& at(std::size_t index) {
        if (index >= sz) throw std::out_of_range("vector");
        return elems[index];
    }
    
    constexpr const T& at(std::size_t index) const {
        if (index >= sz) throw std::out_of_range("vector");
        return elems[index];
    }
    
    constexpr T& operator[](std::size_t index) { return elems[index]; }
    constexpr const T& operator[](std::size_t index) const { return elems[index]; }
    constexpr T& front() { return elems[0]; }
    constexpr const T& front() const { return elems[0]; }
    constexpr T& back() { return elems[sz - 1]; }
    constexpr const T& back() const { return elems[sz - 1]; }

    constexpr T* data() noexcept { return elems; }
    constexpr const T* data() const noexcept { return elems; }

    constexpr iterator begin() noexcept { return elems; }
    constexpr const_iterator begin() const noexcept { return elems; }
    constexpr const_iterator cbegin() const noexcept { return elems; }

    constexpr iterator end() noexcept { return elems + sz; }
    constexpr const_iterator end() const noexcept { return elems + sz; }
    constexpr const_iterator cend() const noexcept { return elems + sz; }

    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(cend()); }
    constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }

    constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(cbegin()); }
    constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

    constexpr bool empty() const noexcept { return sz == 0; }
    constexpr std::size_t size() const noexcept { return sz; }
    constexpr std::size_t max_size() const noexcept {
        return std::min(std::allocator_traits<Allocator>::max_size(alloc), static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()));
    }

    constexpr void reserve(std::size_t new_cap) {
        if (new_cap <= cap) return;
        if (new_cap > max_size()) throw std::length_error("vector");
        T* new_elems = std::allocator_traits<Allocator>::allocate(alloc, new_cap);
        if (elems) {
            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memcpy(new_elems, elems, sz * sizeof(T));
            } else if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                T* p = new_elems;
                try {
                    for (T* i = elems; i < elems + sz; ++i, ++p)
                        std::allocator_traits<Allocator>::construct(alloc, p, std::move(*i));
                } catch (...) {
                    std::destroy(new_elems, p);
                    std::allocator_traits<Allocator>::deallocate(alloc, new_elems, new_cap);
                    throw;
                }
            } else {
                T* p = new_elems;
                try {
                    for (T* i = elems; i < elems + sz; ++i, ++p)
                        std::allocator_traits<Allocator>::construct(alloc, p, *i);
                } catch (...) {
                    std::destroy(new_elems, p);
                    std::allocator_traits<Allocator>::deallocate(alloc, new_elems, new_cap);
                    throw;
                }
            }
            destroy_deallocate();
        }
        elems = new_elems;
        cap = new_cap;
    }

    constexpr std::size_t capacity() const noexcept { return cap; }

    constexpr void shrink_to_fit() {
        if (cap != sz) {
            if (sz == 0) destroy_deallocate();
            else {
                T* new_elems = std::allocator_traits<Allocator>::allocate(alloc, sz);
                if constexpr (std::is_trivially_copyable_v<T>) {
                    std::memcpy(new_elems, elems, sz * sizeof(T));
                } else if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                    T* p = new_elems;
                    try {
                        for (T* i = elems; i < elems + sz; ++i, ++p)
                            std::allocator_traits<Allocator>::construct(alloc, p, std::move(*i));
                    } catch (...) {
                        std::destroy(new_elems, p);
                        std::allocator_traits<Allocator>::deallocate(alloc, new_elems, sz);
                        throw;
                    }
                } else {
                    T* p = new_elems;
                    try {
                        for (T* i = elems; i < elems + sz; ++i, ++p)
                            std::allocator_traits<Allocator>::construct(alloc, p, *i);
                    } catch (...) {
                        std::destroy(new_elems, p);
                        std::allocator_traits<Allocator>::deallocate(alloc, new_elems, sz);
                        throw;
                    }
                }
                destroy_deallocate();
                elems = new_elems;
                cap = sz;
            }
        }
    }

    constexpr void clear() noexcept {
        if constexpr (!std::is_trivially_copyable_v<T>)
            std::destroy(elems, elems + sz);
        sz = 0;
    }

    constexpr iterator insert(const_iterator pos, const T& value) {
        std::size_t index = pos - elems;
        if (sz == cap) reserve(sz ? sz * _MYSTD_VECTOR_GROW : 1);
        if (index < sz) {
            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memmove(elems + index + 1, elems + index, (sz - index) * sizeof(T));
                elems[index] = value;
            } else {
                std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::move(elems[sz - 1]));
                for (T* i = elems + sz - 1; i > elems + index; --i)
                    *i = std::move(*(i - 1));
                elems[index] = value;
            }
        } else {
            std::allocator_traits<Allocator>::construct(alloc, elems + index, value);
        }
        ++sz;
        return elems + index;
    }

    constexpr iterator insert(const_iterator pos, T&& value) {
        std::size_t index = pos - elems;
        if (sz == cap) reserve(sz ? sz * _MYSTD_VECTOR_GROW : 1);
        if (index < sz) {
            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memmove(elems + index + 1, elems + index, (sz - index) * sizeof(T));
                elems[index] = std::move(value);
            } else {
                std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::move(elems[sz - 1]));
                for (T* i = elems + sz - 1; i > elems + index; --i)
                    *i = std::move(*(i - 1));
                elems[index] = std::move(value);
            }
        } else {
            std::allocator_traits<Allocator>::construct(alloc, elems + index, std::move(value));
        }
        ++sz;
        return elems + index;
    }

    constexpr iterator insert(const_iterator pos, std::size_t count, const T& value) {
        if (count == 0) return const_cast<iterator>(pos);
        std::size_t index = pos - elems;
        if (sz + count > cap) reserve(std::max(sz ? sz * _MYSTD_VECTOR_GROW : 1, sz + count));
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memmove(elems + index + count, elems + index, (sz - index) * sizeof(T));
            for (T* i = elems + index; i < elems + index + count; ++i)
                *i = value;
        } else {
            for (T* i = elems + sz + count - 1; i >= elems + index + count; --i) {
                if (i < elems + sz) *i = std::move(*(i - count));
                else std::allocator_traits<Allocator>::construct(alloc, i, std::move(*(i - count)));
            }
            for (T* i = elems + index; i < elems + index + count; ++i) {
                if (i < elems + sz) *i = value;
                else std::allocator_traits<Allocator>::construct(alloc, i, value);
            }
        }
        sz += count;
        return elems + index;
    }

    template<std::input_iterator InputIt>
    constexpr iterator insert(const_iterator pos, InputIt first, InputIt last) {
        std::size_t index = pos - elems;
        if constexpr (std::forward_iterator<InputIt>) {
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            if (count == 0) return const_cast<iterator>(pos);
            if (sz + count > cap) reserve(std::max(sz ? sz * _MYSTD_VECTOR_GROW : 1, sz + count));
            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memmove(elems + index + count, elems + index, (sz - index) * sizeof(T));
                if constexpr (std::contiguous_iterator<InputIt>)
                    std::memmove(elems + index, &*first, count * sizeof(T));
                else
                    std::copy(first, last, elems + index);
            } else {
                for (T* i = elems + sz + count - 1; i >= elems + index + count; --i) {
                    if (i < elems + sz) *i = std::move(*(i - count));
                    else std::allocator_traits<Allocator>::construct(alloc, i, std::move(*(i - count)));
                }
                for (T* i = elems + index; i < elems + index + count; ++i, ++first) {
                    if (i < elems + sz) *i = *first;
                    else std::allocator_traits<Allocator>::construct(alloc, i, *first);
                }
            }
            sz += count;
            return elems + index;
        } else {
            vector<T, Allocator> tmp(first, last, alloc);
            return insert(pos, tmp.begin(), tmp.end());
        }
    }

    constexpr iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
        return insert(pos, ilist.begin(), ilist.end());
    }

    template<class Arg>
    constexpr iterator emplace(const_iterator pos, Arg&& arg) {
        std::size_t index = pos - elems;
        if (sz == cap) reserve(sz ? sz * _MYSTD_VECTOR_GROW : 1);
        if (index == sz) {
            if constexpr (std::is_trivially_move_constructible_v<T>)
                elems[sz] = T(std::forward<Arg>(arg));
            else
                std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::forward<Arg>(arg));
        } else {
            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memmove(elems + index + 1, elems + index, (sz - index) * sizeof(T));
                elems[index] = T(std::forward<Arg>(arg));
            } else {
                std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::move(elems[sz - 1]));
                for (T* i = elems + sz - 1; i > elems + index; --i)
                    *i = std::move(*(i - 1));
                elems[index] = T(std::forward<Arg>(arg));
            }
        }
        ++sz;
        return elems + index;
    }

    constexpr iterator erase(const_iterator pos) {
        std::size_t index = pos - elems;
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memmove(elems + index, elems + index + 1, (sz - index - 1) * sizeof(T));
        } else {
            for (T* i = elems + index; i < elems + sz - 1; ++i) *i = std::move(*(i + 1));
            std::allocator_traits<Allocator>::destroy(alloc, elems + sz - 1);
        }
        --sz;
        return elems + index;
    }

    constexpr iterator erase(const_iterator first, const_iterator last) {
        if (first == last) return const_cast<iterator>(first);
        std::size_t index = first - elems;
        std::size_t end_index = last - elems;
        if (index >= sz) return elems + sz;
        if (end_index > sz) end_index = sz;
        std::size_t count = end_index - index;
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memmove(elems + index, elems + index + count, (sz - index - count) * sizeof(T));
        } else {
            for (T* i = elems + index; i + count < elems + sz; ++i)
                *i = std::move(*(i + count));
            for (T* i = elems + sz - count; i < elems + sz; ++i)
                std::allocator_traits<Allocator>::destroy(alloc, i);
        }
        sz -= count;
        return elems + index;
    }

    constexpr void push_back(const T& value) {
        if (sz == cap) reserve(sz ? sz * _MYSTD_VECTOR_GROW : 1);
        if constexpr (std::is_trivially_copyable_v<T>)
            elems[sz++] = value;
        else {
            std::allocator_traits<Allocator>::construct(alloc, elems + sz, value);
            ++sz;
        }
    }

    constexpr void push_back(T&& value) {
        if (sz == cap) reserve(sz ? sz * _MYSTD_VECTOR_GROW : 1);
        if constexpr (std::is_trivially_move_constructible_v<T>)
            elems[sz++] = std::move(value);
        else {
            std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::move(value));
            ++sz;
        }
    }

    template<class Arg>
    constexpr T& emplace_back(Arg&& arg) {
        if (sz == cap) reserve(sz ? sz * _MYSTD_VECTOR_GROW : 1);
        if constexpr (std::is_trivially_move_constructible_v<T>)
            elems[sz] = T(std::forward<Arg>(arg));
        else
            std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::forward<Arg>(arg));
        return elems[sz++];
    }

    constexpr void pop_back() {
        if (sz > 0) {
            --sz;
            if constexpr (!std::is_trivially_copyable_v<T>)
                std::allocator_traits<Allocator>::destroy(alloc, elems + sz);
        }
    }

    constexpr void resize(std::size_t new_size) {
        if (new_size > cap) reserve(new_size);
        if (new_size < sz) {
            if constexpr (!std::is_trivially_copyable_v<T>)
                for (T* i = elems + new_size; i < elems + sz; ++i)
                    std::allocator_traits<Allocator>::destroy(alloc, i);
            sz = new_size;
        } else if (new_size > sz) {
            for (T* i = elems + sz; i < elems + new_size; ++i)
                std::allocator_traits<Allocator>::construct(alloc, i);
            sz = new_size;
        }
    }

    constexpr void resize(std::size_t new_size, const T& value) {
        if (new_size > cap) reserve(new_size);
        if (new_size < sz) {
            if constexpr (!std::is_trivially_copyable_v<T>)
                for (T* i = elems + new_size; i < elems + sz; ++i)
                    std::allocator_traits<Allocator>::destroy(alloc, i);
            sz = new_size;
        } else if (new_size > sz) {
            for (T* i = elems + sz; i < elems + new_size; ++i)
                std::allocator_traits<Allocator>::construct(alloc, i, value);
            sz = new_size;
        }
    }

    constexpr void swap(vector& other) noexcept(
        std::allocator_traits<Allocator>::propagate_on_container_swap::value ||
        std::allocator_traits<Allocator>::is_always_equal::value) {
        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_swap::value)
            std::swap(alloc, other.alloc);
        std::swap(elems, other.elems);
        std::swap(sz, other.sz);
        std::swap(cap, other.cap);
    }
};


template<class InputIt, class Allocator = std::allocator<typename std::iterator_traits<InputIt>::value_type>>
vector(InputIt, InputIt, Allocator = Allocator()) -> vector<typename std::iterator_traits<InputIt>::value_type, Allocator>;

namespace pmr {
    template<class T>
    using vector = vector<T, std::pmr::polymorphic_allocator<T>>;
}

}

template<class T, class Allocator>
constexpr bool operator==(const mystd::vector<T, Allocator>& lhs, const mystd::vector<T, Allocator>& rhs) {
    if (lhs.size() != rhs.size()) return false;
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template<class T, class Allocator>
constexpr auto operator<=>(const mystd::vector<T, Allocator>& lhs, const mystd::vector<T, Allocator>& rhs) { return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); }

namespace std {

template<class T, class Allocator>
constexpr void swap(mystd::vector<T, Allocator>& lhs, mystd::vector<T, Allocator>& rhs) noexcept(noexcept(lhs.swap(rhs))) { lhs.swap(rhs); }

template<class T, class Allocator, class U>
constexpr typename mystd::vector<T, Allocator>::size_type erase(mystd::vector<T, Allocator>& c, const U& value) {
    auto it = std::remove(c.begin(), c.end(), value);
    auto count = std::distance(it, c.end());
    c.erase(it, c.end());
    return count;
}

template<class T, class Allocator, class Pred>
constexpr typename mystd::vector<T, Allocator>::size_type erase_if(mystd::vector<T, Allocator>& c, Pred pred) {
    auto it = std::remove_if(c.begin(), c.end(), pred);
    auto count = std::distance(it, c.end());
    c.erase(it, c.end());
    return count;
}

}


namespace mystd {

class _Bit_reference {
    unsigned long long* p_;
    unsigned long long mask_;

public:
    _Bit_reference(unsigned long long* data, std::size_t offset) : p_(data + (offset / word_bit)), mask_(1 << (offset % word_bit)) {}
    _Bit_reference(const _Bit_reference&) = default;
    ~_Bit_reference() = default;

    _Bit_reference& operator=(bool x) noexcept {
        if (x) *p_ |= mask_;
        else *p_ &= ~mask_;
        return *this;
    }

    _Bit_reference& operator=(const _Bit_reference& rhs) noexcept {
        bool v = static_cast<bool>(rhs);
        return *this = v;
    }

    operator bool() const noexcept { return (*p_ & mask_) != 0; }

    void flip() noexcept { *p_ ^= mask_; }
};


class _Bit_iterator {
    unsigned long long* data;
    std::size_t offset;

public:
    using difference_type = std::ptrdiff_t;
    using value_type = bool;
    using pointer = _Bit_pointer;
    using _Bit_reference = _Bit_reference;
    using iterator_category = std::random_access_iterator_tag;

    _Bit_iterator() = default;
    _Bit_iterator(unsigned long long* d, std::size_t o) : data(d), offset(o) {}
    operator _Bit_const_iterator() const noexcept { return _Bit_const_iterator(data, offset); }

    _Bit_reference operator*() const { return _Bit_reference(data, offset); }

    _Bit_iterator& operator++() { ++offset; return *this; }
    _Bit_iterator operator++(int) { _Bit_iterator tmp = *this; ++*this; return tmp; }

    _Bit_iterator& operator--() { --offset; return *this; }
    _Bit_iterator operator--(int) { _Bit_iterator tmp = *this; --*this; return tmp; }

    _Bit_iterator& operator+=(std::ptrdiff_t n) { offset += n; return *this; }
    _Bit_iterator operator+(std::ptrdiff_t n) const { return _Bit_iterator(data, offset + n); }
    friend _Bit_iterator operator+(std::ptrdiff_t n, const _Bit_iterator& it) { return it + n; }

    _Bit_iterator& operator-=(std::ptrdiff_t n) { offset -= n; return *this; }
    _Bit_iterator operator-(std::ptrdiff_t n) const { return _Bit_iterator(data, offset - n); }
    std::ptrdiff_t operator-(const _Bit_iterator& rhs) const { return offset - rhs.offset; }

    _Bit_reference operator[](std::ptrdiff_t n) const { return *(*this + n); }

    bool operator==(const _Bit_iterator& rhs) const = default;
    auto operator<=>(const _Bit_iterator& rhs) const = default;
    friend bool operator==(const _Bit_iterator& lhs, const _Bit_const_iterator& rhs) noexcept { return lhs == static_cast<_Bit_iterator>(rhs); }
    friend auto operator<=>(const _Bit_iterator& lhs, const _Bit_const_iterator& rhs) noexcept { return lhs <=> static_cast<_Bit_iterator>(rhs); }
};


class _Bit_const_iterator {
    const unsigned long long* data;
    std::size_t offset;

public:
    using difference_type = std::ptrdiff_t;
    using value_type = bool;
    using pointer = void;
    using _Bit_reference = bool;
    using iterator_category = std::random_access_iterator_tag;

    _Bit_const_iterator() = default;
    _Bit_const_iterator(const unsigned long long* d, std::size_t o) : data(d), offset(o) {}
    operator _Bit_iterator() const noexcept { return _Bit_iterator(data, offset); }

    bool operator*() const {
        std::size_t word = offset / word_bit;
        std::size_t bit = offset % word_bit;
        return (data[word] >> bit) & 1;
    }

    _Bit_const_iterator& operator++() { ++offset; return *this; }
    _Bit_const_iterator operator++(int) { _Bit_const_iterator tmp = *this; ++*this; return tmp; }

    _Bit_const_iterator& operator--() { --offset; return *this; }
    _Bit_const_iterator operator--(int) { _Bit_const_iterator tmp = *this; --*this; return tmp; }

    _Bit_const_iterator& operator+=(std::ptrdiff_t n) { offset += n; return *this; }
    _Bit_const_iterator operator+(std::ptrdiff_t n) const { return _Bit_const_iterator(data, offset + n); }
    friend _Bit_const_iterator operator+(std::ptrdiff_t n, const _Bit_const_iterator& it) { return it + n; }

    _Bit_const_iterator& operator-=(std::ptrdiff_t n) { offset -= n; return *this; }
    _Bit_const_iterator operator-(std::ptrdiff_t n) const { return _Bit_const_iterator(data, offset - n); }
    std::ptrdiff_t operator-(const _Bit_const_iterator& rhs) const { return offset - rhs.offset; }

    _Bit_reference operator[](std::ptrdiff_t n) const { return *(*this + n); }

    bool operator==(const _Bit_const_iterator& rhs) const = default;
    auto operator<=>(const _Bit_const_iterator& rhs) const = default;
    friend bool operator==(const _Bit_const_iterator& lhs, const _Bit_iterator& rhs) noexcept { return lhs == static_cast<_Bit_const_iterator>(rhs); }
    friend auto operator<=>(const _Bit_const_iterator& lhs, const _Bit_iterator& rhs) noexcept { return lhs <=> static_cast<_Bit_const_iterator>(rhs); }
};


template<class Allocator>
class vector<bool, Allocator> {
public:
    using value_type = bool;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = _Bit_reference;
    using const_reference = bool;
    using pointer = _Bit_reference*;
    using const_pointer = const bool*;
    using iterator = _Bit_iterator;
    using const_iterator = _Bit_const_iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    static constexpr std::size_t word_bit = sizeof(unsigned long long) * CHAR_BIT;

private:
    vector<unsigned long long, std::allocator_traits<Allocator>::rebind_alloc<unsigned long long>> elems;
    std::size_t sz;
    static constexpr std::size_t word_index(std::size_t pos) noexcept { return pos / word_bit; }
    static constexpr std::size_t bit_index(std::size_t pos) noexcept { return pos % word_bit; }
    static constexpr unsigned long long bit_mask(std::size_t pos) noexcept { return 1 << bit_index(pos); }

public:
    constexpr vector() noexcept(noexcept(Allocator())) : elems(), sz(0) {}

    explicit constexpr vector(const Allocator& alloc_) noexcept : elems(alloc_), sz(0) {}

    explicit vector(std::size_t count, const Allocator& alloc_ = Allocator()) : elems((count + word_bit - 1) / word_bit, alloc_), sz(count) {}

    constexpr vector(std::size_t count, const bool& value, const Allocator& alloc_ = Allocator()) : elems((count + word_bit - 1) / word_bit, alloc_), sz(count) {
        if (value) {
            std::fill(elems.begin(), elems.end(), ~unsigned long long(0));
            std::size_t last_bits = bit_index(sz);
            if (last_bits != 0) elems.back() &= (1 << last_bits) - 1;
        }
    }

    template<std::input_iterator InputIt>
    constexpr vector(InputIt first, InputIt last, const Allocator& alloc_ = Allocator()) : elems(alloc_), sz(0) { for (; first != last; ++first) push_back(static_cast<bool>(*first)); }

    constexpr vector(const vector& other) : elems(other.elems), sz(other.sz) {}

    constexpr vector(vector&& other) noexcept : elems(std::move(other.elems)), sz(other.sz) { other.sz = 0; }

    constexpr vector(const vector& other, const Allocator& alloc_) : elems(other.elems, alloc_), sz(other.sz) {}

    constexpr vector(vector&& other, const Allocator& alloc_) : elems(std::move(other.elems), alloc_), sz(other.sz) { other.sz = 0; }

    vector(std::initializer_list<bool> ilist, const Allocator& alloc_ = Allocator()) : elems(alloc_), sz(0) {
        reserve(ilist.size());
        for (bool v : ilist) push_back(v);
    }

    constexpr ~vector() = default;

    constexpr vector& operator=(const vector& other) {
        elems = other.elems;
        sz = other.sz;
        return *this;
    }

    constexpr vector& operator=(vector&& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || std::allocator_traits<Allocator>::is_always_equal::value) {
        elems = other.elems;
        sz = other.sz;
        return *this;
    }

    constexpr vector& operator=(std::initializer_list<bool> ilist) {
        assign(ilist);
        return *this;
    }

    constexpr void assign(std::size_t count, const bool& value) {
        if (value) {
            elems.assign((count + word_bit - 1) / word_bit, ~unsigned long long(0));
            std::size_t last_bits = bit_index(count);
            if (last_bits != 0) elems.back() &= (1 << last_bits) - 1;
        } else elems.assign((count + word_bit - 1) / word_bit, unsigned long long(0));
        sz = count;
    }

    template<std::input_iterator InputIt>
    constexpr void assign(InputIt first, InputIt last) {
        if constexpr (std::forward_iterator<InputIt>) {
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            assign(count, false);
            for (std::size_t pos = 0; first != last; ++first, ++pos) {
                if (static_cast<bool>(*first)) elems[word_index(pos)] |= bit_mask(pos);
            }
            sz = count;
        } else {
            for (; first != last; ++first) push_back(*first);
        }
    }

    constexpr void assign(std::initializer_list<bool> ilist) { assign(ilist.begin(), ilist.end()); }

    constexpr allocator_type get_allocator() const noexcept { return elems.get_allocator(); }

    constexpr reference at(std::size_t index) {
        if (index >= sz) throw std::out_of_range("vector");
        return (*this)[index];
    }

    constexpr bool at(std::size_t index) const {
        if (index >= sz) throw std::out_of_range("vector");
        return (*this)[index];
    }

    constexpr reference operator[](std::size_t index) { return reference(elems.data(), index); }
    constexpr bool operator[](std::size_t index) const { return (elems.data()[word_index(index)] >> bit_index(index)) & 1; }

    constexpr reference front() { return (*this)[0]; }
    constexpr bool front() const { return (*this)[0]; }

    constexpr reference back() { return (*this)[sz - 1]; }
    constexpr bool back() const { return (*this)[sz - 1]; }

    constexpr unsigned long long* data() noexcept { return elems.data(); } // delete
    constexpr const unsigned long long* data() const noexcept { return elems.data(); } // delete

    constexpr iterator begin() noexcept { return iterator(elems.data(), 0); }
    constexpr const_iterator begin() const noexcept { return const_iterator(elems.data(), 0); }
    constexpr const_iterator cbegin() const noexcept { return const_iterator(elems.data(), 0); }

    constexpr iterator end() noexcept { return iterator(elems.data(), sz); }
    constexpr const_iterator end() const noexcept { return const_iterator(elems.data(), sz); }
    constexpr const_iterator cend() const noexcept { return const_iterator(elems.data(), sz); }

    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(cend()); }
    constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }

    constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(cbegin()); }
    constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

    constexpr bool empty() const noexcept { return sz == 0; }
    constexpr std::size_t size() const noexcept { return sz; }
    constexpr std::size_t max_size() const noexcept { return elems.max_size() * word_bit; }

    constexpr void reserve(std::size_t new_cap) { elems.reserve((new_cap + word_bit - 1) / word_bit); }

    constexpr std::size_t capacity() const noexcept { return elems.capacity() * word_bit; }

    constexpr void shrink_to_fit() { elems.shrink_to_fit(); }

    constexpr void clear() noexcept {
        elems.clear();
        sz = 0;
    }

    constexpr iterator insert(const_iterator pos, const bool& value) {
        std::size_t index = static_cast<std::size_t>(pos - cbegin());
        if (index > sz) resize(index);
        if (index == sz) {
            push_back(value);
            return iterator(elems.data(), index);
        }
        if (sz % word_bit == 0) elems.push_back(unsigned long long(0));
        ++sz;
        std::size_t w = word_index(index);
        std::size_t b = bit_index(index);
        for (auto i = elems.end() - 1; i > elems.begin() + w; --i) {
            *i = (*i << 1) | (*(i - 1) >> (word_bit - 1));
        }
        if (b != 0) {
            unsigned long long mask = (1 << b) - 1;
            elems[w] = (elems[w] & mask) | ((elems[w] & ~mask) << 1);
        } else elems[w] <<= 1;
        if (value) elems[w] |= (1 << b);
        else elems[w] &= ~(1 << b);
        unsigned long long last_bits = bit_index(sz);
        if (last_bits != 0) elems.back() &= (1 << last_bits) - 1;
        return iterator(elems.data(), index);
    }

    constexpr iterator insert(const_iterator pos, bool&& value) { return insert(pos, value); }

    constexpr iterator insert(const_iterator pos, std::size_t count, const bool& value) {
        std::size_t index  = static_cast<std::size_t>(pos - cbegin());
        if (index > sz) resize(index);
        if (count == 0) return iterator(elems.data(), index);
        std::size_t new_sz = sz + count;
        std::size_t need_words = (new_sz + word_bit - 1) / word_bit;
        if (elems.size() < need_words) elems.resize(need_words);
        if (index == sz) {
            std::size_t sw = word_index(sz);
            std::size_t sb = bit_index(sz);
            std::size_t ew = word_index(new_sz);
            std::size_t eb = bit_index(new_sz);
            if (value) {
                if (sw == ew) {
                    elems[sw] |= ~((sb == 0) ? unsigned long long(0) : ((1 << sb) - 1)) & ((1 << eb) - 1);
                    if (eb != 0) elems[sw] &= (1 << eb) - 1;
                } else {
                    elems[sw] |= ~((1 << sb) - 1);
                    for (std::size_t w = sw + 1; w < ew; ++w) elems[w] = ~unsigned long long(0);
                    if (eb != 0) elems[ew] |= (1 << eb) - 1;
                }
            }
            sz = new_sz;
            return iterator(elems.data(), index);
        }
        std::size_t sw = word_index(index);
        std::size_t sb = bit_index(index);
        std::size_t end_bit = index + count;
        std::size_t ew = word_index(end_bit);
        std::size_t eb = bit_index(end_bit);
        std::size_t word_diff = ew - sw;
        std::ptrdiff_t bit_diff = eb - sb;
        if (bit_diff == 0) std::memmove(static_cast<void*>(&elems[ew]), static_cast<const void*>(&elems[sw]), ((sz + word_bit - 1) / word_bit - sw) * sizeof(unsigned long long));
        else if (bit_diff > 0) {
            if (word_diff != 0) {
                for (auto i = elems.end() - 1; i > elems.begin() + ew; --i) {
                    *i = (*(i - word_diff) << bit_diff) | (*(i - word_diff - 1) >> (word_bit - bit_diff));
                }
                elems[ew] = elems[sw] << bit_diff;
                elems[sw] &= (sb == 0) ? unsigned long long(0) : ((1 << sb) - 1);
            } else {
                for (auto i = elems.end() - 1; i > elems.begin() + ew; --i) {
                    *i = (*i << bit_diff) | (*(i - 1) >> (word_bit - bit_diff));
                }
                unsigned long long mask = (sb == 0) ? unsigned long long(0) : ((1 << sb) - 1);
                elems[sw] = (elems[sw] & mask) | ((elems[sw] & ~mask) << bit_diff);
            }
        } else {
            std::size_t rshift = static_cast<std::size_t>(-bit_diff);
            for (auto i = elems.end() - 1; i >= elems.begin() + ew; --i) {
                *i = (*(i - word_diff) >> rshift) | (*(i - word_diff + 1) << (word_bit - rshift));
            }
            elems[sw] &= (sb == 0) ? unsigned long long(0) : ((1 << sb) - 1);
        }
        if (value) {
            if (word_diff == 0) {
                elems[sw] |= ~((sb == 0) ? unsigned long long(0) : (1 << sb) - 1) & ((1 << eb) - 1);
            } else {
                elems[sw] |= ~((1 << sb) - 1);
                for (std::size_t w = sw + 1; w < ew; ++w) elems[w] = ~unsigned long long(0);
                if (eb != 0) elems[ew] |= (1 << eb) - 1;
            }
        } else {
            elems[sw] &= (sb == 0) ? unsigned long long(0) : ((1 << sb) - 1);
            if (sw != ew) {
               for (std::size_t w = sw + 1; w < ew; ++w) elems[w] = unsigned long long(0);
               if (eb != 0) elems[ew] &= ~((1 << eb) - 1);
            }
        }
        sz = new_sz;
        unsigned long long last_bits = bit_index(sz);
        if (last_bits != 0) elems.back() &= (1 << last_bits) - 1;
        return iterator(elems.data(), index);
    }

    template<std::input_iterator InputIt>
    constexpr iterator insert(const_iterator pos, InputIt first, InputIt last) {
        std::size_t index  = static_cast<std::size_t>(pos - cbegin());
        if (index > sz) resize(index);
        if constexpr (std::forward_iterator<InputIt>) {
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            if (count == 0) return iterator(elems.data(), index);
            insert(pos, count, false);
            std::size_t index = pos - cbegin();
            std::size_t cur = index;
            for (; first != last; ++first, ++cur) {
                if (static_cast<bool>(*first)) elems[word_index(cur)] |= bit_mask(cur);
            }
            return iterator(elems.data(), index);
        } else {
            std::size_t index = pos - cbegin();
            std::size_t cur = iterator(elems.data(), index);
            for (; first != last; ++first, ++cur) {
                insert(const_iterator(cur), static_cast<bool>(*first));
            }
            return iterator(elems.data(), index);
        }
    }

    constexpr iterator insert(const_iterator pos, std::initializer_list<bool> ilist) { return insert(pos, ilist.begin(), ilist.end()); }

    template<class Arg>
    constexpr iterator emplace(const_iterator pos, Arg&& arg) {
        return insert(pos, static_cast<bool>(std::forward<Arg>(arg)));
    }

    constexpr iterator erase(const_iterator pos) {
        std::size_t index = static_cast<std::size_t>(pos - cbegin());
        if (index > sz) {
            resize(index);
            return end();
        }
        if (index == sz) return end();
        --sz;
        std::size_t last_bits = bit_index(sz);
        if (index == sz) {
            if (last_bits != 0) elems.back() &= (1 << last_bits) - 1;
            else elems.pop_back();
            return iterator(elems.data(), index);
        }
        std::size_t w = word_index(index);
        std::size_t b = bit_index(index);
        if (b != 0) {
            elems[w] = (elems[w] & ((1 << b) - 1)) | ((elems[w] & ~((1 << (b + 1)) - 1)) >> 1);
        } else elems[w] >>= 1;
        elems[w] |= (elems[w + 1] & 1) << (word_bit - 1);
        for (auto i = elems.begin() + w + 1; i < elems.end() - 1; ++i) {
            *i >>= 1;
            *i |= (*(i + 1) & 1) << (word_bit - 1);
        }
        *(elems.end() - 1) >>= 1;
        if (last_bits != 0) elems.back() &= (1 << last_bits) - 1;
        else elems.pop_back();
        return iterator(elems.data(), index);
    }

    constexpr iterator erase(const_iterator first, const_iterator last) {
        std::size_t index  = static_cast<std::size_t>(first - cbegin());
        if (index > sz) {
            resize(index);
            return end();
        }
        if (index == sz) return end();
        std::size_t count;
        if (last > cend()) count = static_cast<std::size_t>(std::distance(first, cend()));
        else count = static_cast<std::size_t>(std::distance(first, last));
        if (count == 0) return iterator(elems.data(), index);
        sz -= count;
        std::size_t w = word_index(index);
        std::size_t b = bit_index(index);
        std::size_t last_word = word_index(sz - 1);
        std::size_t last_bits = bit_index(sz);
        if (index == sz) {
            if (last_bits != 0) elems[last_word] &= (1 << last_bits) - 1;
            if (elems.size() > last_word + 1) elems.resize(last_word + 1);
            return iterator(elems.data(), index);
        }
        count += b;
        std::size_t word_diff = count / word_bit;
        std::size_t bit_left  = count % word_bit;
        if (bit_left == b) {
            if (b != 0) {
                elems[w] = (elems[w] & ((1 << b) - 1)) | (elems[w + word_diff] & ~((1 << b) - 1));
                if (w + word_diff + 1 < elems.size()) std::memmove(static_cast<void*>(&elems[w + 1]), static_cast<const void*>(&elems[w + word_diff + 1]), (elems.size() - w - word_diff - 1) * sizeof(unsigned long long));
            } else std::memmove(static_cast<void*>(&elems[w]), static_cast<const void*>(&elems[w + word_diff]), (elems.size() - w - word_diff) * sizeof(unsigned long long));
        } else if (bit_left < b) {
            std::size_t bit_diff = b - bit_left;
            elems[w] = (elems[w] & ((1 << b) - 1)) | ((elems[w + word_diff] & ~((1 << bit_left) - 1)) << bit_diff);
            for (auto i = elems.begin() + w + 1; i <= elems.begin() + last_word; ++i) {
                *i = (*(i + word_diff - 1) >> (word_bit - bit_diff)) | (*(i + word_diff) << bit_diff);
            }
        } else {
            std::size_t bit_diff = bit_left - b;
            elems[w] = (elems[w] & (((b == 0) ? unsigned long long(0) : (1 << b)) - 1)) | ((elems[w + word_diff] & ~((1 << bit_left) - 1)) >> bit_diff);
            if (w + word_diff + 1 < elems.size()) elems[w] |= elems[w + word_diff + 1] << (word_bit - bit_diff);
            for (auto i = elems.begin() + w + 1; i < elems.begin() + last_word; ++i) {
                *i = (*(i + word_diff) >> bit_diff) | (*(i + word_diff + 1) << (word_bit - bit_diff));
            }
            elems[last_word] = elems[last_word + word_diff] >> bit_diff;
            if (last_word + word_diff + 1 < elems.size()) {
                elems[last_word] |= elems[last_word + word_diff + 1] << (word_bit - bit_diff);
            }
        }
        if (last_bits != 0) elems[last_word] &= (1 << last_bits) - 1;
        if (elems.size() > last_word + 1) elems.resize(last_word + 1);
        return iterator(elems.data(), index);
    }

    constexpr void push_back(const bool& value) {
        if (sz == capacity()) reserve(sz ? sz * _MYSTD_VECTOR_GROW : 1);
        std::size_t b = bit_index(sz);
        if (b == 0) elems.push_back(unsigned long long(0));
        if (value) elems.back() |= 1 << b;
        ++sz;
    }

    constexpr void push_back(bool&& value) { push_back(value); }

    template<class Arg>
    constexpr reference emplace_back(Arg&& arg) {
        bool value = static_cast<bool>(std::forward<Arg>(arg));
        push_back(value);
        return back();
    }

    constexpr void pop_back() {
        if (sz == 0) return;
        --sz;
        if (bit_index(sz) == 0) elems.pop_back();
        else elems.back() &= ~bit_mask(sz);
    }

    constexpr void resize(std::size_t new_size) {
        elems.resize((new_size + word_bit - 1) / word_bit);
        sz = new_size;
    }

    constexpr void resize(std::size_t new_size, const bool& value) {
        if (value) {
            elems.resize((new_size + word_bit - 1) / word_bit, ~unsigned long long(0));
            if (new_size > sz) {
                std::size_t last_bits = bit_index(sz);
                if (last_bits != 0) elems[word_index(sz)] |= ~((1 << last_bits) - 1);
            }
            std::size_t last_bits = bit_index(new_size);
            if (last_bits != 0) elems.back() &= (1 << last_bits) - 1;
            sz = new_size;
        } else resize(new_size);
    }

    constexpr void swap(vector<bool, Allocator>& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value || std::allocator_traits<Allocator>::is_always_equal::value) {
        elems.swap(other.elems);
        std::swap(sz, other.sz);
    }

    constexpr void flip() {
        if (sz == 0) return;
        for (auto& word : elems) word = ~word;
        std::size_t last_bits = bit_index(sz);
        if (last_bits != 0) elems.back() &= (1 << last_bits) - 1;
    }

    static constexpr void swap(reference x, reference y) {
        bool xb = static_cast<bool>(x);
        bool yb = static_cast<bool>(y);
        if (xb != yb) {
            x = !xb;
            y = !yb;
        }
    }
};

}

namespace std {

template<class Allocator>
struct hash<mystd::vector<bool, Allocator>> {
    std::size_t operator()(const mystd::vector<bool, Allocator>& v) const noexcept {
        std::size_t h = 0xcbf29ce484222325ull;
        constexpr std::size_t prime = 0x100000001b3ull;
        for (auto i : v) {
            h ^= static_cast<bool>(i);
            h *= prime;
        }
        return h;
    }
};

}

