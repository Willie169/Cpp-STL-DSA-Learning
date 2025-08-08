#pragma once

#ifndef VECTOR_GROW
#define VECTOR_GROW 2
#endif

#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <memory_resource>
#include <stdexcept>
#include <type_traits>
#include <utility>


template<class T, class Allocator = std::allocator<T>>
class Vector {
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

    inline constexpr void __destroy_deallocate() {
        if (elems) {
            for (T* i = elems; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
            std::allocator_traits<Allocator>::deallocate(alloc, elems, cap);
        }
    }

public:
    constexpr Vector() noexcept(noexcept(Allocator())) : alloc(Allocator()), elems(nullptr), sz(0), cap(0) {}
    
    explicit constexpr Vector(const Allocator& alloc_) noexcept : alloc(alloc_), elems(nullptr), sz(0), cap(0) {}

    explicit Vector(std::size_t count, const Allocator& alloc_ = Allocator()) : alloc(alloc_), sz(count), cap(count) {
        if (count == 0) elems = nullptr;
        else {
            elems = std::allocator_traits<Allocator>::allocate(alloc, count);
            for (T* i = elems; i < elems + count; ++i) std::allocator_traits<Allocator>::construct(alloc, i);
        }
    }

    constexpr Vector(std::size_t count, const T& value, const Allocator& alloc_ = Allocator()) : alloc(alloc_), sz(count), cap(count) {
        if (count == 0) elems = nullptr;
        else {
            elems = std::allocator_traits<Allocator>::allocate(alloc, count);
            for (T* i = elems; i < elems + count; ++i) std::allocator_traits<Allocator>::construct(alloc, i, value);
        }
    }

    template<std::input_iterator InputIt>
    constexpr Vector(InputIt first, InputIt last, const Allocator& alloc_ = Allocator()) : alloc(alloc_), elems(nullptr), sz(0), cap(0) {
        if constexpr (std::random_access_iterator<InputIt>) {
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            if (count > 0) {
                elems = std::allocator_traits<Allocator>::allocate(alloc, count);
                sz = cap = count;
                for (T* i = elems; i < elems + count; ++i, ++first) std::allocator_traits<Allocator>::construct(alloc, i, *first);
            }
        } else for (; first != last; ++first) push_back(*first);
    }

    constexpr Vector(const Vector& other) : alloc(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.alloc)), elems(nullptr), sz(other.sz), cap(other.cap) {
        if (cap > 0) {
            elems = std::allocator_traits<Allocator>::allocate(alloc, cap);
            for (T* i = elems, * j = other.elems; i < elems + sz; ++i, ++j) std::allocator_traits<Allocator>::construct(alloc, i, *j);
        }
    }

    constexpr Vector(Vector&& other) noexcept : alloc(std::move(other.alloc)), elems(nullptr), sz(0), cap(0) {
        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value) {
            alloc = std::move(other.alloc);
            elems = std::exchange(other.elems, nullptr);
            sz = std::exchange(other.sz, 0);
            cap = std::exchange(other.cap, 0);
        } else if (alloc == other.alloc) {
            elems = std::exchange(other.elems, nullptr);
            sz = std::exchange(other.sz, 0);
            cap = std::exchange(other.cap, 0);
        } else {
            if (other.sz > 0) {
                sz = cap = other.sz;
                elems = std::allocator_traits<Allocator>::allocate(alloc, cap);
                for (T* i = other.elems, * j = elems; i < other.elems + other.sz; ++i, ++j) std::allocator_traits<Allocator>::construct(alloc, j, *i);
            }
            other.__destroy_deallocate();
            other.elems = nullptr;
            other.sz = other.cap = 0;
        }
    }

    constexpr Vector(const Vector& other, const Allocator& alloc_) : alloc(alloc_), elems(nullptr), sz(other.sz), cap(other.cap) {
        if (cap > 0) {
            elems = std::allocator_traits<Allocator>::allocate(alloc, cap);
            for (T* i = elems, * j = other.elems; i < elems + sz; ++i, ++j) std::allocator_traits<Allocator>::construct(alloc, i, *j);
        }
    }

    constexpr Vector(Vector&& other, const Allocator& alloc_) : alloc(alloc_), elems(nullptr), sz(0), cap(0) {
        if (alloc == other.alloc) {
            elems = std::exchange(other.elems, nullptr);
            sz = std::exchange(other.sz, 0);
            cap = std::exchange(other.cap, 0);
        } else {
            sz = other.sz;
            cap = other.cap;
            if (cap > 0) {
                elems = std::allocator_traits<Allocator>::allocate(alloc, cap);
                for (T* i = elems, * j = other.elems; i < elems + sz; ++i, ++j) std::allocator_traits<Allocator>::construct(alloc, i, std::move(*j));
            }
        }
    }

    Vector(std::initializer_list<T> ilist, const Allocator& alloc_ = Allocator()) : alloc(alloc_), elems(nullptr), sz(ilist.size()), cap(ilist.size()) {
        if (cap > 0) {
            elems = std::allocator_traits<Allocator>::allocate(alloc, cap);
            for (T* i = const_cast<T*>(ilist.begin()), * j = elems; i < const_cast<T*>(ilist.end()); ++i, ++j) std::allocator_traits<Allocator>::construct(alloc, j, *i);
        }
    }

    constexpr ~Vector() {
        if (elems) {
            for (T* i = elems; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
            std::allocator_traits<Allocator>::deallocate(alloc, elems, cap);
        }
    }

    constexpr Vector& operator=(const Vector& other) {
        if (this != &other) {
            if constexpr (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value && alloc != other.alloc) {
                __destroy_deallocate();
                alloc = other.alloc;
                cap = other.sz;
                elems = std::allocator_traits<Allocator>::allocate(alloc, cap);
            } else if (other.sz > cap) {
                __destroy_deallocate();
                cap = other.sz;
                elems = std::allocator_traits<Allocator>::allocate(alloc, cap);
            } else if (elems) for (T* i = elems + other.sz; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
            for (T* i = other.elems, * j = elems; i < other.elems + other.sz; ++i, ++j) {
                if (i < elems + sz) *j = *i;
                else std::allocator_traits<Allocator>::construct(alloc, j, *i);
            }
            sz = other.sz;
        }
        return *this;
    }

    constexpr Vector& operator=(Vector&& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || std::allocator_traits<Allocator>::is_always_equal::value) {
        if (this != &other) {
            if constexpr (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value) {
                __destroy_deallocate();
                alloc = std::move(other.alloc);
                elems = std::exchange(other.elems, nullptr);
                sz = std::exchange(other.sz, 0);
                cap = std::exchange(other.cap, 0);
            } else if (alloc == other.alloc) {
                __destroy_deallocate();
                elems = std::exchange(other.elems, nullptr);
                sz = std::exchange(other.sz, 0);
                cap = std::exchange(other.cap, 0);
            } else {
                if (other.sz > cap) {
                    __destroy_deallocate();
                    cap = other.sz;
                    elems = std::allocator_traits<Allocator>::allocate(alloc, cap);
                } else if (elems) for (T* i = elems + other.sz; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
                for (T* i = other.elems, * j = elems; i < other.elems + other.sz; ++i, ++j) {
                    if (i < elems + sz) *j = *i;
                    else std::allocator_traits<Allocator>::construct(alloc, j, *i);
                }
                sz = other.sz;
                other.elems = nullptr;
                other.sz = other.cap = 0;
            }
        }
        return *this;
    }

    constexpr Vector& operator=(std::initializer_list<T> ilist) {
        assign(ilist);
        return *this;
    }

    constexpr void assign(std::size_t count, const T& value) {
        if (count > cap) {
            __destroy_deallocate();
            elems = std::allocator_traits<Allocator>::allocate(alloc, count);
            cap = count;
        } else if (elems) for (T* i = elems + count; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
        for (T* i = elems; i < elems + count; ++i) {
            if (i < elems + sz) *i = value;
            else std::allocator_traits<Allocator>::construct(alloc, i, value);
        }
        sz = count;
    }

    template<std::input_iterator InputIt>
    constexpr void assign(InputIt first, InputIt last) {
        if constexpr (std::random_access_iterator<InputIt>) {
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            if (count > cap) {
                __destroy_deallocate();
                elems = std::allocator_traits<Allocator>::allocate(alloc, count);
                cap = count;
            } else if (elems) for (T* i = elems + count; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
            for (T* i = elems; i < elems + count; ++i, ++first) {
                if (i < elems + sz) *i = *first;
                else std::allocator_traits<Allocator>::construct(alloc, i, *first);
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
    constexpr std::size_t max_size() const noexcept { return std::min(std::allocator_traits<Allocator>::max_size(alloc), static_cast<std::size_t>(std::numeric_limits<difference_type>::max())); }

    constexpr void reserve(std::size_t new_cap) {
        if (new_cap <= cap) return;
        T* new_elems = std::allocator_traits<Allocator>::allocate(alloc, new_cap);
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            for (T* i = elems, * j = new_elems; i < elems + sz; ++i, ++j) std::allocator_traits<Allocator>::construct(alloc, j, std::move(*i));
        } else for (T* i = elems, * j = new_elems; i < elems + sz; ++i, ++j) std::allocator_traits<Allocator>::construct(alloc, j, *i);
        for (T* i = elems; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
        if (elems) std::allocator_traits<Allocator>::deallocate(alloc, elems, cap);
        elems = new_elems;
        cap = new_cap;
    }

    constexpr std::size_t capacity() const noexcept { return cap; }

    constexpr void shrink_to_fit() {
        if (cap != sz) {
            if (sz == 0) {
                if (elems) {
                    std::allocator_traits<Allocator>::deallocate(alloc, elems, cap);
                    elems = nullptr;
                    cap = 0;
                }
            } else {
                T* new_elems = std::allocator_traits<Allocator>::allocate(alloc, sz);
                for (T* i = new_elems, * j = elems; i < new_elems + sz; ++i, ++j) {
                    std::allocator_traits<Allocator>::construct(alloc, i, std::move(*j));
                    std::allocator_traits<Allocator>::destroy(alloc, j);
                }
                std::allocator_traits<Allocator>::deallocate(alloc, elems, cap);
                elems = new_elems;
                cap = sz;
            }
        }
    }

    constexpr void clear() noexcept {
        for (T* i = elems; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
        sz = 0;
    }

    constexpr iterator insert(const_iterator pos, const T& value) {
        std::size_t index = pos - elems;
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        if (index < sz) {
            std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::move(elems[sz - 1]));
            for (T* i = elems + sz - 1; i > elems + index; --i) *i = std::move(*(i - 1));
            elems[index] = value;
        } else std::allocator_traits<Allocator>::construct(alloc, elems + index, value);
        ++sz;
        return elems + index;
    }

    constexpr iterator insert(const_iterator pos, T&& value) {
        std::size_t index = pos - elems;
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        if (index < sz) {
            std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::move(elems[sz - 1]));
            for (T* i = elems + sz - 1; i > elems + index; --i) *i = std::move(*(i - 1));
            elems[index] = std::move(value);
        } else std::allocator_traits<Allocator>::construct(alloc, elems + index, std::move(value));
        ++sz;
        return elems + index;
    }

    constexpr iterator insert(const_iterator pos, std::size_t count, const T& value) {
        if (count == 0) return const_cast<iterator>(pos);
        std::size_t index = pos - elems;
        if (sz + count > cap) reserve(std::max(sz ? sz * VECTOR_GROW : 1, sz + count));
        for (T* i = elems + sz + count - 1; i >= elems + index + count; --i) {
            if (i >= elems + sz) std::allocator_traits<Allocator>::construct(alloc, i, std::move(*(i - count)));
            else *i = std::move(*(i - count));
        }
        for (T* i = elems + index; i < elems + index + count; ++i) {
            if (i < elems + sz) *i = value;
            else std::allocator_traits<Allocator>::construct(alloc, i, value);
        }
        sz += count;
        return elems + index;
    }

    template<std::input_iterator InputIt>
    constexpr iterator insert(const_iterator pos, InputIt first, InputIt last) {
        if (first == last) return const_cast<iterator>(pos);
        if constexpr (std::random_access_iterator<InputIt>) {
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            if (sz + count > cap) reserve(std::max(sz ? sz * VECTOR_GROW : 1, sz + count));
            for (T* i = elems; i < elems + count; ++i, ++first) {
                if (i < elems + sz) *i = *first;
                else std::allocator_traits<Allocator>::construct(alloc, i, *first);
            }
            return const_cast<iterator>(pos);
        } else {
            Vector<T, Allocator> tmp(first, last, alloc);
            return insert(pos, tmp.begin(), tmp.end());
        }
    }

    constexpr iterator insert(const_iterator pos, std::initializer_list<T> ilist) { return insert(pos, ilist.begin(), ilist.end()); }

    template<class... Args>
    constexpr iterator emplace(const_iterator pos, Args&&... args) {
        std::size_t index = pos - elems;
        if (index < sz) {
            if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
            std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::move(elems[sz - 1]));
            std::allocator_traits<Allocator>::destroy(alloc, elems + sz - 1);
            for (T* i = elems + sz - 1; i > elems + index; --i) *i = std::move(*(i - 1));
            std::allocator_traits<Allocator>::construct(alloc, elems + index, std::forward<Args>(args)...);
        } else std::allocator_traits<Allocator>::construct(alloc, elems + index, std::forward<Args>(args)...);
        ++sz;
        return elems + index;
    }

    constexpr iterator erase(const_iterator pos) {
        std::size_t index = pos - elems;
        if (index >= sz) return end();
        for (T* i = elems + index; i < elems + sz - 1; ++i) *i = std::move(*(i+1));
        std::allocator_traits<Allocator>::destroy(alloc, elems + sz - 1);
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
        for (T* i = elems + index; i + count < elems + sz; ++i) *i = std::move(*(i + count));
        for (T* i = elems + sz - count; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
        sz -= count;
        return elems + index;
    }


    constexpr void push_back(const T& value) {
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        std::allocator_traits<Allocator>::construct(alloc, elems + sz, value);
        ++sz;
    }

    constexpr void push_back(T&& value) {
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::move(value));
        ++sz;
    }

    template<class... Args>
    constexpr T& emplace_back(Args&&... args) {
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::forward<Args>(args)...);
        return elems[sz++];
    }

    constexpr void pop_back() {
        if (sz > 0) {
            --sz;
            std::allocator_traits<Allocator>::destroy(alloc, elems + sz);
        }
    }

    constexpr void resize(std::size_t new_size) {
        if (new_size > cap) reserve(new_size);
        if (new_size < sz) for (T* i = elems + new_size; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
        else if (new_size > sz) for (T* i = elems + sz; i < elems + new_size; ++i) std::allocator_traits<Allocator>::construct(alloc, i, T());
        sz = new_size;
    }

    constexpr void resize(std::size_t new_size, const T& value) {
        if (new_size > cap) reserve(new_size);
        if (new_size < sz) for (T* i = elems + new_size; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
        else if (new_size > sz) for (T* i = elems + sz; i < elems + new_size; ++i) std::allocator_traits<Allocator>::construct(alloc, i, value);
        sz = new_size;
    }

    constexpr void swap(Vector& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value || std::allocator_traits<Allocator>::is_always_equal::value) {
        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_swap::value) std::swap(alloc, other.alloc);
        std::swap(elems, other.elems);
        std::swap(sz, other.sz);
        std::swap(cap, other.cap);
    }
};

template<class InputIt, class Alloc = std::allocator<typename std::iterator_traits<InputIt>::value_type>>
Vector(InputIt, InputIt, Alloc = Alloc()) -> Vector<typename std::iterator_traits<InputIt>::value_type, Alloc>;

template<class T, class Allocator>
constexpr bool operator==(const Vector<T, Allocator>& lhs, const Vector<T, Allocator>& rhs) {
    if (lhs.size() != rhs.size()) return false;
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template<class T, class Allocator>
constexpr auto operator<=>(const Vector<T, Allocator>& lhs, const Vector<T, Allocator>& rhs) { return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); }

template<class T, class Allocator, class U>
constexpr typename Vector<T, Allocator>::size_type erase(Vector<T, Allocator>& c, const U& value) {
    auto it = std::remove(c.begin(), c.end(), value);
    auto count = std::distance(it, c.end());
    c.erase(it, c.end());
    return count;
}

template<class T, class Allocator, class Pred>
constexpr typename Vector<T, Allocator>::size_type erase_if(Vector<T, Allocator>& c, Pred pred) {
    auto it = std::remove_if(c.begin(), c.end(), pred);
    auto count = std::distance(it, c.end());
    c.erase(it, c.end());
    return count;
}

template<class T, class Allocator>
constexpr void swap(Vector<T, Allocator>& lhs, Vector<T, Allocator>& rhs) noexcept(noexcept(lhs.swap(rhs))) { lhs.swap(rhs); }

namespace pmr {
    template<class T>
    using Vector = Vector<T, std::pmr::polymorphic_allocator<T>>;
}

