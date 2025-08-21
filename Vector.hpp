#pragma once

#ifndef VECTOR_GROW
#define VECTOR_GROW 2
#endif

#include <algorithm>
#include <climits>
#include <compare>
#include <cstddef>
#include <cstring>
#include <climits>
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

	constexpr void destroy_deallocate() {
	    if (elems) {
	        if constexpr (!std::is_trivially_copyable_v<T>) {
	            for (T* i = elems; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
	        }
	        std::allocator_traits<Allocator>::deallocate(alloc, elems, cap);
	        elems = nullptr;
	        sz = cap = 0;
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
        if constexpr (std::forward_iterator<InputIt>) {
            if (first == last) return;
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
            other.destroy_deallocate();
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
                destroy_deallocate();
                alloc = other.alloc;
                cap = other.sz;
                elems = std::allocator_traits<Allocator>::allocate(alloc, cap);
            } else if (other.sz > cap) {
                destroy_deallocate();
                cap = other.sz;
                elems = std::allocator_traits<Allocator>::allocate(alloc, cap);
            } else if constexpr (!std::is_trivially_copyable_v<T>) {
                for (T* i = elems + other.sz; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
            }
	        if constexpr (std::is_trivially_copyable_v<T>) std::memmove(elems, other.elems, other.sz * sizeof(T));
	        else {
	            for (T* i = other.elems, * j = elems; i < other.elems + other.sz; ++i, ++j) {
	                if (i < elems + sz) *j = *i;
	                else std::allocator_traits<Allocator>::construct(alloc, j, *i);
	            }
	        }
	        sz = other.sz;
	    }
	    return *this;
	}

    constexpr Vector& operator=(Vector&& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || std::allocator_traits<Allocator>::is_always_equal::value) {
        if (this != &other) {
            if constexpr (alloc == other.alloc) {
                destroy_deallocate();
                elems = std::exchange(other.elems, nullptr);
                sz = std::exchange(other.sz, 0);
                cap = std::exchange(other.cap, 0);
            } else if constexpr (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value) {
                destroy_deallocate();
                alloc = std::move(other.alloc);
                elems = std::exchange(other.elems, nullptr);
                sz = std::exchange(other.sz, 0);
                cap = std::exchange(other.cap, 0);
            } else {
                if (other.sz > cap) {
                    destroy_deallocate();
                    cap = other.sz;
                    elems = std::allocator_traits<Allocator>::allocate(alloc, cap);
                } else if constexpr (!std::is_trivially_copyable_v<T>) {
                    for (T* i = elems + other.sz; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
                }
                if constexpr (std::is_trivially_copyable_v<T>) std::memmove(elems, other.elems, other.sz * sizeof(T));
    	        else {
                    for (T* i = other.elems, * j = elems; i < other.elems + other.sz; ++i, ++j) {
                        if (i < elems + sz) *j = *i;
                        else std::allocator_traits<Allocator>::construct(alloc, j, *i);
                    }
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
            destroy_deallocate();
            elems = std::allocator_traits<Allocator>::allocate(alloc, count);
            cap = count;
        } else if constexpr (!std::is_trivially_copyable_v<T>) for (T* i = elems + count; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
        for (T* i = elems; i < elems + count; ++i) {
            if (i < elems + sz) *i = value;
            else std::allocator_traits<Allocator>::construct(alloc, i, value);
        }
        sz = count;
    }

    template<std::input_iterator InputIt>
    constexpr void assign(InputIt first, InputIt last) {
        if constexpr (std::forward_iterator<InputIt>) {
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            if (count > cap) {
                destroy_deallocate();
                elems = std::allocator_traits<Allocator>::allocate(alloc, count);
                cap = count;
            } else if constexpr (!std::is_trivially_copyable_v<T>) for (T* i = elems + count; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
            if constexpr (std::is_trivially_copyable_v<T>) {
                if constexpr (std::contiguous_iterator<InputIt>) std::memmove(elems, &*first, count * sizeof(T));
                else std::copy(first, last, elems);
            } else {
                for (T* i = elems; i < elems + count; ++i, ++first) {
                    if (i < elems + sz) *i = *first;
                    else std::allocator_traits<Allocator>::construct(alloc, i, *first);
                }
            }
            sz = count;
        } else {
            clear();
            for (; first != last; ++first) push_back(*first);
        }
    }

    constexpr void assign(std::initializer_list<T> ilist) { assign(ilist.begin(), ilist.end()); }

    constexpr allocator_type get_allocator() const noexcept { return alloc; }

    constexpr T& at(size_type index) {
        if (index >= sz) throw std::out_of_range("Vector");
        return elems[index];
    }

    constexpr const T& at(size_type index) const {
        if (index >= sz) throw std::out_of_range("Vector");
        return elems[index];
    }

    constexpr T& operator[](size_type index) { return elems[index]; }
    constexpr const T& operator[](size_type index) const { return elems[index]; }

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
        if (new_cap > max_size()) throw std::length_error("Vector");
        T* new_elems = std::allocator_traits<Allocator>::allocate(alloc, new_cap);
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            for (T* i = elems, * j = new_elems; i < elems + sz; ++i, ++j) std::allocator_traits<Allocator>::construct(alloc, j, std::move(*i));
        } else for (T* i = elems, * j = new_elems; i < elems + sz; ++i, ++j) std::allocator_traits<Allocator>::construct(alloc, j, *i);
        if constexpr (!std::is_trivially_copyable_v<T>) for (T* i = elems; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
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
                    if constexpr (!std::is_trivially_copyable_v<T>) std::allocator_traits<Allocator>::destroy(alloc, j);
                }
                std::allocator_traits<Allocator>::deallocate(alloc, elems, cap);
                elems = new_elems;
                cap = sz;
            }
        }
    }

    constexpr void clear() noexcept {
        if constexpr (!std::is_trivially_copyable_v<T>) for (T* i = elems; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
        sz = 0;
    }

    constexpr iterator insert(const_iterator pos, const T& value) {
        size_type index = pos - elems;
        if (index > sz) resize(index);
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        if (index < sz) {
            if constexpr (std::is_trivially_copyable_v<T>) std::memmove(elems + index + 1, elems + index, (sz - index) * sizeof(T));
            else {
                std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::move(elems[sz - 1]));
                for (T* i = elems + sz - 1; i > elems + index; --i) *i = std::move(*(i - 1));
            }
            elems[index] = value;
        } else std::allocator_traits<Allocator>::construct(alloc, elems + index, value);
        ++sz;
        return elems + index;
    }

    constexpr iterator insert(const_iterator pos, T&& value) {
        size_type index = pos - elems;
        if (index > sz) resize(index);
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        if (index < sz) {
            if constexpr (std::is_trivially_copyable_v<T>) std::memmove(elems + index + 1, elems + index, (sz - index) * sizeof(T));
            else {
                std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::move(elems[sz - 1]));
                for (T* i = elems + sz - 1; i > elems + index; --i) *i = std::move(*(i - 1));
            }
            elems[index] = std::move(value);
        } else std::allocator_traits<Allocator>::construct(alloc, elems + index, std::move(value));
        ++sz;
        return elems + index;
    }

    constexpr iterator insert(const_iterator pos, std::size_t count, const T& value) {
        if (count == 0) return const_cast<iterator>(pos);
        size_type index = pos - elems;
        if (index > sz) resize(index);
        if (sz + count > cap) reserve(std::max(sz ? sz * VECTOR_GROW : 1, sz + count));
        if constexpr (std::is_trivially_copyable_v<T>) std::memmove(elems + index + count, elems + index, (sz - index) * sizeof(T));
        else {
            for (T* i = elems + sz + count - 1; i >= elems + index + count; --i) {
                if (i < elems + sz) *i = std::move(*(i - count));
                else std::allocator_traits<Allocator>::construct(alloc, i, std::move(*(i - count)));
            }
        }
        for (T* i = elems + index; i < elems + index + count; ++i) *i = value;
        sz += count;
        return elems + index;
    }

    template<std::input_iterator InputIt>
    constexpr iterator insert(const_iterator pos, InputIt first, InputIt last) {
        size_type index = pos - elems;
        if (index > sz) resize(index);
        if constexpr (std::forward_iterator<InputIt>) {
            if (first == last) return const_cast<iterator>(pos);
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            if (sz + count > cap) reserve(std::max(sz ? sz * VECTOR_GROW : 1, sz + count));
            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memmove(elems + index + count, elems + index, (sz - index) * sizeof(T));
                if constexpr (std::contiguous_iterator<InputIt>) std::memmove(elems + index, &*first, count * sizeof(T));
                else std::copy(first, last, elems + index);
            } else {
                for (T* i = elems + sz + count - 1; i >= elems + index + count; --i) {
                    if (i < elems + sz) *i = std::move(*(i - count));
                    else std::allocator_traits<Allocator>::construct(alloc, i, *(i - count));
                }
                for (T* i = elems + index; i < elems + index + count; ++i, ++first) *i = *first;
            }
            sz += count;
            return elems + index;
        } else {
            Vector<T, Allocator> tmp(first, last, alloc);
            return insert(pos, tmp.begin(), tmp.end());
        }
    }

    constexpr iterator insert(const_iterator pos, std::initializer_list<T> ilist) { return insert(pos, ilist.begin(), ilist.end()); }

	template<class Arg>
	constexpr iterator emplace(const_iterator pos, Arg&& arg) {
	    size_type index = pos - elems;
	    if (index > sz) resize(index);
	    if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
	    if (index == sz) {
	        std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::forward<Arg>(arg));
	    } else {
	        if constexpr (std::is_trivially_copyable_v<T>) {
	            std::memmove(elems + index + 1, elems + index, (sz - index) * sizeof(T));
	            std::allocator_traits<Allocator>::construct(alloc, elems + index, std::forward<Arg>(arg));
	        } else {
	            std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::move(elems[sz - 1]));
	            for (T* i = elems + sz - 1; i > elems + index; --i) *i = std::move(*(i - 1));
	            elems[index] = T(std::forward<Arg>(arg));
	        }
	    }
	    ++sz;
	    return elems + index;
	}

    constexpr iterator erase(const_iterator pos) {
        size_type index = pos - elems;
        if constexpr (std::is_trivially_copyable_v<T>) std::memmove(elems + index, elems + index + 1, (sz - index - 1) * sizeof(T));
        else {
            for (T* i = elems + index; i < elems + sz - 1; ++i) *i = std::move(*(i+1));
            std::allocator_traits<Allocator>::destroy(alloc, elems + sz - 1);
        }
        --sz;
        return elems + index;
    }

    constexpr iterator erase(const_iterator first, const_iterator last) {
        if (first == last) return const_cast<iterator>(first);
        size_type index = first - elems;
        std::size_t end_index = last - elems;
        if (index >= sz) return elems + sz;
        if (end_index > sz) end_index = sz;
        std::size_t count = end_index - index;
        for (T* i = elems + index; i + count < elems + sz; ++i) *i = std::move(*(i + count));
        if constexpr (!std::is_trivially_copyable_v<T>) for (T* i = elems + sz - count; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
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

    template<class Arg>
    constexpr T& emplace_back(Arg&& arg) {
        if (sz == cap) reserve(sz ? sz * VECTOR_GROW : 1);
        std::allocator_traits<Allocator>::construct(alloc, elems + sz, std::forward<Arg>(arg));
        return elems[sz++];
    }

    constexpr void pop_back() {
        if (sz > 0) {
            --sz;
            if constexpr (!std::is_trivially_copyable_v<T>) std::allocator_traits<Allocator>::destroy(alloc, elems + sz);
        }
    }

    constexpr void resize(std::size_t new_size) {
        if (new_size > cap) reserve(new_size);
        if (new_size < sz) {
            if constexpr (!std::is_trivially_copyable_v<T>) for (T* i = elems + new_size; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
            sz = new_size;
        } else if (new_size > sz) {
            for (T* i = elems + sz; i < elems + new_size; ++i) std::allocator_traits<Allocator>::construct(alloc, i, T());
            sz = new_size;
        }
    }

    constexpr void resize(std::size_t new_size, const T& value) {
        if (new_size > cap) reserve(new_size);
        if (new_size < sz) {
            if constexpr (!std::is_trivially_copyable_v<T>) for (T* i = elems + new_size; i < elems + sz; ++i) std::allocator_traits<Allocator>::destroy(alloc, i);
            sz = new_size;
        } else if (new_size > sz) {
            for (T* i = elems + sz; i < elems + new_size; ++i) std::allocator_traits<Allocator>::construct(alloc, i, value);
            sz = new_size;
        }
    }

    constexpr void swap(Vector& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value || std::allocator_traits<Allocator>::is_always_equal::value) {
        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_swap::value) std::swap(alloc, other.alloc);
        std::swap(elems, other.elems);
        std::swap(sz, other.sz);
        std::swap(cap, other.cap);
    }
};


template<class InputIt, class Allocator = std::allocator<typename std::iterator_traits<InputIt>::value_type>>
Vector(InputIt, InputIt, Allocator = Allocator()) -> Vector<typename std::iterator_traits<InputIt>::value_type, Allocator>;

template<class T, class Allocator>
constexpr bool operator==(const Vector<T, Allocator>& lhs, const Vector<T, Allocator>& rhs) {
    if (lhs.size() != rhs.size()) return false;
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template<class T, class Allocator>
constexpr auto operator<=>(const Vector<T, Allocator>& lhs, const Vector<T, Allocator>& rhs) { return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); }

template<class T, class Allocator>
constexpr void swap(Vector<T, Allocator>& lhs, Vector<T, Allocator>& rhs) noexcept(noexcept(lhs.swap(rhs))) { lhs.swap(rhs); }

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

namespace pmr {
    template<class T>
    using Vector = Vector<T, std::pmr::polymorphic_allocator<T>>;
}


template<class Allocator>
requires std::is_integral_v<typename Allocator::value_type> && std::is_unsigned_v<typename Allocator::value_type> && (!std::is_same_v<bool, typename Allocator::value_type>)
class Vector<bool, Allocator> {
public:
    using word_type = typename Allocator::value_type;
    using value_type = bool;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    class reference;
    using const_reference = bool;
    class pointer;
    using const_pointer = void;
    class iterator;
    class const_iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    static constexpr std::size_t word_bit = sizeof(word_type) * CHAR_BIT;


    class reference {
        word_type* p_;
        word_type mask_;

    public:
        reference(word_type* data, size_type offset) : p_(data + (offset / word_bit)), mask_(word_type(1) << (offset % word_bit)) {}
        reference(const reference&) = default;
        ~reference() = default;

        reference& operator=(bool x) noexcept {
            if (x) *p_ |= mask_;
            else *p_ &= ~mask_;
            return *this;
        }

        reference& operator=(const reference& rhs) noexcept {
            bool v = static_cast<bool>(rhs);
            return *this = v;
        }

        operator bool() const noexcept { return (*p_ & mask_) != 0; }

        void flip() noexcept { *p_ ^= mask_; }
    };


    class pointer {
        reference ref;
    public:
        pointer(word_type* d, size_type o) : ref(d, o) {}
        reference* operator->() { return &ref; }
        reference& operator*() { return ref; }
    };


    class iterator {
        word_type* data;
        size_type offset;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = bool;
        using pointer = Vector::pointer;
        using reference = Vector::reference;
        using iterator_category = std::random_access_iterator_tag;

        iterator() = default;
        iterator(word_type* d, size_type o) : data(d), offset(o) {}
        operator const_iterator() const noexcept { return const_iterator(data, offset); }

        reference operator*() const { return reference(data, offset); }
        pointer operator->() const { return pointer(data, offset); }

        iterator& operator++() { ++offset; return *this; }
        iterator operator++(int) { iterator tmp = *this; ++*this; return tmp; }

        iterator& operator--() { --offset; return *this; }
        iterator operator--(int) { iterator tmp = *this; --*this; return tmp; }

        iterator& operator+=(difference_type n) { offset += n; return *this; }
        iterator operator+(difference_type n) const { return iterator(data, offset + n); }
        friend iterator operator+(difference_type n, const iterator& it) { return it + n; }

        iterator& operator-=(difference_type n) { offset -= n; return *this; }
        iterator operator-(difference_type n) const { return iterator(data, offset - n); }
        difference_type operator-(const iterator& rhs) const { return offset - rhs.offset; }

        reference operator[](difference_type n) const { return *(*this + n); }

        bool operator==(const iterator& rhs) const = default;
        auto operator<=>(const iterator& rhs) const = default;
        friend bool operator==(const iterator& lhs, const const_iterator& rhs) noexcept { return lhs == static_cast<iterator>(rhs); }
        friend auto operator<=>(const iterator& lhs, const const_iterator& rhs) noexcept { return lhs <=> static_cast<iterator>(rhs); }
    };


    class const_iterator {
        const word_type* data;
        size_type offset;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = bool;
        using pointer = void;
        using reference = bool;
        using iterator_category = std::random_access_iterator_tag;

        const_iterator() = default;
        const_iterator(const word_type* d, size_type o) : data(d), offset(o) {}
        operator iterator() const noexcept { return iterator(data, offset); }

        bool operator*() const {
            size_type word = offset / word_bit;
            size_type bit = offset % word_bit;
            return (data[word] >> bit) & word_type(1);
        }

        const_iterator& operator++() { ++offset; return *this; }
        const_iterator operator++(int) { const_iterator tmp = *this; ++*this; return tmp; }

        const_iterator& operator--() { --offset; return *this; }
        const_iterator operator--(int) { const_iterator tmp = *this; --*this; return tmp; }

        const_iterator& operator+=(difference_type n) { offset += n; return *this; }
        const_iterator operator+(difference_type n) const { return const_iterator(data, offset + n); }
        friend const_iterator operator+(difference_type n, const const_iterator& it) { return it + n; }

        const_iterator& operator-=(difference_type n) { offset -= n; return *this; }
        const_iterator operator-(difference_type n) const { return const_iterator(data, offset - n); }
        difference_type operator-(const const_iterator& rhs) const { return offset - rhs.offset; }

        reference operator[](difference_type n) const { return *(*this + n); }

        bool operator==(const const_iterator& rhs) const = default;
        auto operator<=>(const const_iterator& rhs) const = default;
        friend bool operator==(const const_iterator& lhs, const iterator& rhs) noexcept { return lhs == static_cast<const_iterator>(rhs); }
        friend auto operator<=>(const const_iterator& lhs, const iterator& rhs) noexcept { return lhs <=> static_cast<const_iterator>(rhs); }
    };


private:
    Vector<word_type, Allocator> elems;
    size_type sz;
    static constexpr size_type word_index(size_type pos) noexcept { return pos / word_bit; }
    static constexpr size_type bit_index(size_type pos) noexcept { return pos % word_bit; }
    static constexpr word_type bit_mask(size_type pos) noexcept { return word_type(1) << bit_index(pos); }

public:
    constexpr Vector() noexcept(noexcept(Allocator())) : elems(), sz(0) {}

    explicit constexpr Vector(const Allocator& alloc_) noexcept : elems(alloc_), sz(0) {}

    explicit Vector(size_type count, const Allocator& alloc_ = Allocator()) : elems((count + word_bit - 1) / word_bit, alloc_), sz(count) {}

    constexpr Vector(size_type count, const bool& value, const Allocator& alloc_ = Allocator()) : elems((count + word_bit - 1) / word_bit, alloc_), sz(count) {
        if (value) {
            std::fill(elems.begin(), elems.end(), ~word_type(0));
            size_type last_bits = bit_index(sz);
            if (last_bits != 0) elems.back() &= (word_type(1) << last_bits) - 1;
        }
    }

    template<std::input_iterator InputIt>
    constexpr Vector(InputIt first, InputIt last, const Allocator& alloc_ = Allocator()) : elems(alloc_), sz(0) { for (; first != last; ++first) push_back(static_cast<bool>(*first)); }

    constexpr Vector(const Vector& other) : elems(other.elems), sz(other.sz) {}

    constexpr Vector(Vector&& other) noexcept : elems(std::move(other.elems)), sz(other.sz) { other.sz = 0; }

    constexpr Vector(const Vector& other, const Allocator& alloc_) : elems(other.elems, alloc_), sz(other.sz) {}

    constexpr Vector(Vector&& other, const Allocator& alloc_) : elems(std::move(other.elems), alloc_), sz(other.sz) { other.sz = 0; }

    Vector(std::initializer_list<bool> ilist, const Allocator& alloc_ = Allocator()) : elems(alloc_), sz(0) {
        reserve(ilist.size());
        for (bool v : ilist) push_back(v);
    }

    constexpr ~Vector() = default;

    constexpr Vector& operator=(const Vector& other) {
        elems = other.elems;
        sz = other.sz;
        return *this;
    }

    constexpr Vector& operator=(Vector&& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || std::allocator_traits<Allocator>::is_always_equal::value) {
        elems = other.elems;
        sz = other.sz;
        return *this;
    }

    constexpr Vector& operator=(std::initializer_list<bool> ilist) {
        assign(ilist);
        return *this;
    }

    constexpr void assign(std::size_t count, const bool& value) {
        if (value) {
            elems.assign((count + word_bit - 1) / word_bit, ~word_type(0));
            size_type last_bits = bit_index(count);
            if (last_bits != 0) elems.back() &= (word_type(1) << last_bits) - 1;
        } else elems.assign((count + word_bit - 1) / word_bit, word_type(0));
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

    constexpr reference at(size_type index) {
        if (index >= sz) throw std::out_of_range("Vector");
        return (*this)[index];
    }

    constexpr bool at(size_type index) const {
        if (index >= sz) throw std::out_of_range("Vector");
        return (*this)[index];
    }

    constexpr reference operator[](size_type index) { return reference(elems.data(), index); }
    constexpr bool operator[](size_type index) const { return (elems.data()[word_index(index)] >> bit_index(index)) & word_type(1); }

    constexpr reference front() { return (*this)[0]; }
    constexpr bool front() const { return (*this)[0]; }

    constexpr reference back() { return (*this)[sz - 1]; }
    constexpr bool back() const { return (*this)[sz - 1]; }

    constexpr word_type* data() noexcept { return elems.data(); } // delete
    constexpr const word_type* data() const noexcept { return elems.data(); } // delete

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
        size_type index = static_cast<size_type>(pos - cbegin());
        if (index > sz) resize(index);
        if (index == sz) {
            push_back(value);
            return iterator(elems.data(), index);
        }
        if (sz % word_bit == 0) elems.push_back(word_type(0));
        ++sz;
        std::size_t w = word_index(index);
        std::size_t b = bit_index(index);
        for (auto i = elems.end() - 1; i > elems.begin() + w; --i) {
            *i = (*i << 1) | (*(i - 1) >> (word_bit - 1));
        }
        if (b != 0) {
            word_type mask = (word_type(1) << b) - 1;
            elems[w] = (elems[w] & mask) | ((elems[w] & ~mask) << 1);
        } else elems[w] <<= 1;
        if (value) elems[w] |= (word_type(1) << b);
        else elems[w] &= ~(word_type(1) << b);
        word_type last_bits = bit_index(sz);
        if (last_bits != 0) elems.back() &= (word_type(1) << last_bits) - 1;
        return iterator(elems.data(), index);
    }

    constexpr iterator insert(const_iterator pos, bool&& value) { return insert(pos, value); }

    constexpr iterator insert(const_iterator pos, size_type count, const bool& value) {
        size_type index  = static_cast<size_type>(pos - cbegin());
        if (index > sz) resize(index);
        if (count == 0) return iterator(elems.data(), index);
        size_type new_sz = sz + count;
        size_type need_words = (new_sz + word_bit - 1) / word_bit;
        if (elems.size() < need_words) elems.resize(need_words);
        if (index == sz) {
            size_type sw = word_index(sz);
            size_type sb = bit_index(sz);
            size_type ew = word_index(new_sz);
            size_type eb = bit_index(new_sz);
            if (value) {
                if (sw == ew) {
                    elems[sw] |= ~((sb == 0) ? word_type(0) : ((word_type(1) << sb) - 1)) & ((word_type(1) << eb) - 1);
                    elems[sw] &= (word_type(1) << eb) - 1;
                } else {
                    elems[sw] |= ~((word_type(1) << sb) - 1);
                    for (size_type w = sw + 1; w < ew; ++w) elems[w] = ~word_type(0);
                    if (eb != 0) elems[ew] |= (word_type(1) << eb) - 1;
                }
            }
            sz = new_sz;
            return iterator(elems.data(), index);
        }
        size_type sw = word_index(index);
        size_type sb = bit_index(index);
        size_type end_bit = index + count;
        size_type ew = word_index(end_bit);
        size_type eb = bit_index(end_bit);
        size_type word_diff = ew - sw;
        difference_type bit_diff = eb - sb;
        if (bit_diff == 0) std::memmove(static_cast<void*>(&elems[ew]), static_cast<const void*>(&elems[sw]), ((sz + word_bit - 1) / word_bit - sw) * sizeof(word_type));
        else if (bit_diff > 0) {
            if (word_diff != 0) {
                for (auto i = elems.end() - 1; i > elems.begin() + ew; --i) {
                    *i = (*(i - word_diff) << bit_diff) | (*(i - word_diff - 1) >> (word_bit - bit_diff));
                }
                elems[ew] = elems[sw] << bit_diff;
                elems[sw] &= (sb == 0) ? word_type(0) : ((word_type(1) << sb) - 1);
            } else {
                for (auto i = elems.end() - 1; i > elems.begin() + ew; --i) {
                    *i = (*i << bit_diff) | (*(i - 1) >> (word_bit - bit_diff));
                }
                word_type mask = (sb == 0) ? word_type(0) : (word_type(1) << sb) - 1;
                elems[sw] = (elems[sw] & mask) | ((elems[sw] & ~mask) << bit_diff);
            }
        } else {
            size_type rshift = static_cast<std::size_t>(-bit_diff);
            for (auto i = elems.end() - 1; i >= elems.begin() + ew; --i) {
                *i = (*(i - word_diff) >> rshift) | (*(i - word_diff + 1) << (word_bit - rshift));
            }
            elems[sw] &= (sb == 0) ? word_type(0) : ((word_type(1) << sb) - 1);
        }
        if (value) {
            if (word_diff == 0) {
                elems[sw] |= ~((sb == 0) ? word_type(0) : (word_type(1) << sb) - 1) & ((word_type(1) << eb) - 1);
            } else {
                elems[sw] |= ~((word_type(1) << sb) - 1);
                for (size_type w = sw + 1; w < ew; ++w) elems[w] = ~word_type(0);
                if (eb != 0) elems[ew] |= (word_type(1) << eb) - 1;
            }
        } else {
            elems[sw] &= (sb == 0) ? word_type(0) : (word_type(1) << sb) - 1;
            if (sw != ew) {
               for (size_type w = sw + 1; w < ew; ++w) elems[w] = word_type(0);
               if (eb != 0) elems[ew] &= ~((word_type(1) << eb) - 1);
            }
        }
        sz = new_sz;
        word_type last_bits = bit_index(sz);
        if (last_bits != 0) elems.back() &= (word_type(1) << last_bits) - 1;
        return iterator(elems.data(), index);
    }

    template<std::input_iterator InputIt>
    constexpr iterator insert(const_iterator pos, InputIt first, InputIt last) {
        size_type index  = static_cast<size_type>(pos - cbegin());
        if (index > sz) resize(index);
        if constexpr (std::forward_iterator<InputIt>) {
            std::size_t count = static_cast<std::size_t>(std::distance(first, last));
            if (count == 0) return iterator(elems.data(), index);
            insert(pos, count, false);
            size_type index = pos - cbegin();
            size_type cur = index;
            for (; first != last; ++first, ++cur) {
                if (static_cast<bool>(*first)) elems[word_index(cur)] |= bit_mask(cur);
            }
            return iterator(elems.data(), index);
        } else {
            size_type index = pos - cbegin();
            size_type cur = iterator(elems.data(), index);
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
        size_type index = static_cast<size_type>(pos - cbegin());
        if (index > sz) {
            resize(index);
            return end();
        }
        if (index == sz) return end();
        --sz;
        size_type last_bits = bit_index(sz);
        if (index == sz) {
            if (last_bits != 0) elems.back() &= (word_type(1) << last_bits) - 1;
            else elems.pop_back();
            return iterator(elems.data(), index);
        }
        size_type w = word_index(index);
        size_type b = bit_index(index);
        if (b != 0) {
            elems[w] = (elems[w] & ((word_type(1) << b) - 1)) | ((elems[w] & ~((word_type(1) << (b + 1)) - 1)) >> 1);
        } else elems[w] >>= 1;
        elems[w] |= (elems[w + 1] & word_type(1)) << (word_bit - 1);
        for (auto i = elems.begin() + w + 1; i < elems.end() - 1; ++i) {
            *i >>= 1;
            *i |= (*(i + 1) & word_type(1)) << (word_bit - 1);
        }
        *(elems.end() - 1) >>= 1;
        if (last_bits != 0) elems.back() &= (word_type(1) << last_bits) - 1;
        else elems.pop_back();
        return iterator(elems.data(), index);
    }

    constexpr iterator erase(const_iterator first, const_iterator last) {
        size_type index  = static_cast<size_type>(first - cbegin());
        if (index > sz) {
            resize(index);
            return end();
        }
        if (index == sz) return end();
        size_type count;
        if (last > cend()) count = static_cast<std::size_t>(std::distance(first, cend()));
        else count = static_cast<std::size_t>(std::distance(first, last));
        if (count == 0) return iterator(elems.data(), index);
        sz -= count;
        size_type w = word_index(index);
        size_type b = bit_index(index);
        size_type last_word = word_index(sz - 1);
        size_type last_bits = bit_index(sz);
        if (index == sz) {
            if (last_bits != 0) elems[last_word] &= (word_type(1) << last_bits) - 1;
            if (elems.size() > last_word + 1) elems.resize(last_word + 1);
            return iterator(elems.data(), index);
        }
        count += b;
        size_type word_diff = count / word_bit;
        size_type bit_left  = count % word_bit;
        if (bit_left == b) {
            if (b != 0) {
                elems[w] = (elems[w] & ((word_type(1) << b) - 1)) | (elems[w + word_diff] & ~((word_type(1) << b) - 1));
                if (w + word_diff + 1 < elems.size()) std::memmove(static_cast<void*>(&elems[w + 1]), static_cast<const void*>(&elems[w + word_diff + 1]), (elems.size() - w - word_diff - 1) * sizeof(word_type));
            } else std::memmove(static_cast<void*>(&elems[w]), static_cast<const void*>(&elems[w + word_diff]), (elems.size() - w - word_diff) * sizeof(word_type));
        } else if (bit_left < b) {
            size_type bit_diff = b - bit_left;
            elems[w] = (elems[w] & ((word_type(1) << b) - 1)) | ((elems[w + word_diff] & ~((word_type(1) << bit_left) - 1)) << bit_diff);
            for (auto i = elems.begin() + w + 1; i <= elems.begin() + last_word; ++i) {
                *i = (*(i + word_diff - 1) >> (word_bit - bit_diff)) | (*(i + word_diff) << bit_diff);
            }
        } else {
            size_type bit_diff = bit_left - b;
            elems[w] = (elems[w] & (((b == 0) ? word_type(0) : (word_type(1) << b)) - 1)) | ((elems[w + word_diff] & ~((word_type(1) << bit_left) - 1)) >> bit_diff);
            if (w + word_diff + 1 < elems.size()) elems[w] |= elems[w + word_diff + 1] << (word_bit - bit_diff);
            for (auto i = elems.begin() + w + 1; i < elems.begin() + last_word; ++i) {
                *i = (*(i + word_diff) >> bit_diff) | (*(i + word_diff + 1) << (word_bit - bit_diff));
            }
            elems[last_word] = elems[last_word + word_diff] >> bit_diff;
            if (last_word + word_diff + 1 < elems.size()) {
                elems[last_word] |= elems[last_word + word_diff + 1] << (word_bit - bit_diff);
            }
        }
        if (last_bits != 0) elems[last_word] &= (word_type(1) << last_bits) - 1;
        if (elems.size() > last_word + 1) elems.resize(last_word + 1);
        return iterator(elems.data(), index);
    }

    constexpr void push_back(const bool& value) {
        if (sz == capacity()) reserve(sz ? sz * VECTOR_GROW : 1);
        size_type b = bit_index(sz);
        if (b == 0) elems.push_back(word_type(0));
        if (value) elems.back() |= word_type(1) << b;
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
            elems.resize((new_size + word_bit - 1) / word_bit, ~word_type(0));
            if (new_size > sz) {
                size_type last_bits = bit_index(sz);
                if (last_bits != 0) elems[word_index(sz)] |= ~((word_type(1) << last_bits) - 1);
            }
            size_type last_bits = bit_index(new_size);
            if (last_bits != 0) elems.back() &= (word_type(1) << last_bits) - 1;
            sz = new_size;
        } else resize(new_size);
    }

    constexpr void swap(Vector<bool, Allocator>& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value || std::allocator_traits<Allocator>::is_always_equal::value) {
        elems.swap(other.elems);
        std::swap(sz, other.sz);
    }

    constexpr void flip() {
        if (sz == 0) return;
        for (auto& word : elems) word = ~word;
        size_type last_bits = bit_index(sz);
        if (last_bits != 0) elems.back() &= (word_type(1) << last_bits) - 1;
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


template<class Allocator>
struct std::hash<Vector<bool, Allocator>> {
    std::size_t operator()(const Vector<bool, Allocator>& v) const noexcept {
        std::size_t h = 0xcbf29ce484222325ull;
        constexpr std::size_t prime = 0x100000001b3ull;
        using word_type = typename Allocator::value_type;
        const std::size_t word_bit = sizeof(word_type) * CHAR_BIT;
        std::size_t nwords = v.size() / word_bit;
        std::size_t leftover = v.size() % word_bit;
        const word_type* raw = v.data();
        for (std::size_t i = 0; i < nwords; ++i) {
            h ^= static_cast<std::size_t>(raw[i]);
            h *= prime;
        }
        std::size_t idx = nwords * word_bit;
        word_type tail = 0;
        for (std::size_t i = 0; i < leftover; ++i)
            if (v[idx + i]) tail |= (word_type(1) << i);
        h ^= static_cast<std::size_t>(tail);
        h *= prime;
        return h;
    }
};

