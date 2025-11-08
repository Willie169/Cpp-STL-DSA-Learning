#pragma once

#include "vector.hpp"

namespace mystd {

class _Bit_iterator;
class _Bit_const_iterator;

class _Bit_reference {
    unsigned long long* p_;
    unsigned long long mask_;
    static constexpr std::size_t word_bit = sizeof(unsigned long long) * CHAR_BIT;

public:
    _Bit_reference() = default;
    _Bit_reference(unsigned long long* data, std::size_t offset) : p_(data + (offset / word_bit)), mask_(1ULL << (offset % word_bit)) {}
    _Bit_reference(const unsigned long long* data, std::size_t offset) : p_(const_cast<unsigned long long*>(data) + (offset / word_bit)), mask_(1ULL << (offset % word_bit)) {}
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
    friend class _Bit_const_iterator;
    static constexpr std::size_t word_bit = sizeof(unsigned long long) * CHAR_BIT;

public:
    using difference_type = std::ptrdiff_t;
    using value_type = bool;
    using pointer = _Bit_reference*;
    using reference = _Bit_reference;
    using iterator_category = std::random_access_iterator_tag;

    _Bit_iterator() = default;
    _Bit_iterator(unsigned long long* d, std::size_t o) noexcept : data(d), offset(o) {}
    _Bit_iterator(const unsigned long long* d, std::size_t o) noexcept : data(const_cast<unsigned long long*>(d)), offset(o) {}
    _Bit_iterator(const _Bit_const_iterator& other) noexcept;

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
    std::strong_ordering operator<=>(const _Bit_iterator& rhs) const = default;

    friend bool operator==(const _Bit_iterator& lhs, const _Bit_const_iterator& rhs) noexcept;
    friend std::strong_ordering operator<=>(const _Bit_iterator& lhs, const _Bit_const_iterator& rhs) noexcept;
};


class _Bit_const_iterator {
    const unsigned long long* data;
    std::size_t offset;
    friend class _Bit_iterator;
    static constexpr std::size_t word_bit = sizeof(unsigned long long) * CHAR_BIT;

public:
    using difference_type = std::ptrdiff_t;
    using value_type = bool;
    using pointer = const bool*;
    using reference = bool;
    using iterator_category = std::random_access_iterator_tag;

    _Bit_const_iterator() = default;
    _Bit_const_iterator(unsigned long long* d, std::size_t o) noexcept : data(d), offset(o) {}
    _Bit_const_iterator(const unsigned long long* d, std::size_t o) noexcept : data(d), offset(o) {}
    _Bit_const_iterator(const _Bit_iterator& other) noexcept;

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

    bool operator[](std::ptrdiff_t n) const { return *(*this + n); }

    bool operator==(const _Bit_const_iterator& rhs) const = default;
    std::strong_ordering operator<=>(const _Bit_const_iterator& rhs) const = default;

    friend bool operator==(const _Bit_const_iterator& lhs, const _Bit_iterator& rhs) noexcept;
    friend std::strong_ordering operator<=>(const _Bit_const_iterator& lhs, const _Bit_iterator& rhs) noexcept;
};


_Bit_iterator::_Bit_iterator(const _Bit_const_iterator& other) noexcept : data(const_cast<unsigned long long*>(other.data)), offset(other.offset) {}
bool operator==(const _Bit_iterator& lhs, const _Bit_const_iterator& rhs) noexcept { return lhs == static_cast<_Bit_iterator>(rhs); }
std::strong_ordering operator<=>(const _Bit_iterator& lhs, const _Bit_const_iterator& rhs) noexcept { return lhs <=> static_cast<_Bit_iterator>(rhs); }
_Bit_const_iterator::_Bit_const_iterator(const _Bit_iterator& other) noexcept : data(other.data), offset(other.offset) {}
bool operator==(const _Bit_const_iterator& lhs, const _Bit_iterator& rhs) noexcept { return lhs == static_cast<_Bit_const_iterator>(rhs); }
std::strong_ordering operator<=>(const _Bit_const_iterator& lhs, const _Bit_iterator& rhs) noexcept { return lhs <=> static_cast<_Bit_const_iterator>(rhs); }


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
    vector<unsigned long long, mystd::allocator<unsigned long long>> elems;
    std::size_t sz;
    static constexpr std::size_t word_index(std::size_t pos) noexcept { return pos / word_bit; }
    static constexpr std::size_t bit_index(std::size_t pos) noexcept { return pos % word_bit; }
    static constexpr unsigned long long bit_mask(std::size_t pos) noexcept { return 1ULL << bit_index(pos); }

public:
    constexpr vector() noexcept(noexcept(Allocator())) : elems(), sz(0) {}

    explicit constexpr vector(const Allocator& alloc_) noexcept : elems(alloc_), sz(0) {}

    explicit vector(std::size_t count, const Allocator& alloc_ = Allocator()) : elems((count + word_bit - 1) / word_bit, alloc_), sz(count) {}

    constexpr vector(std::size_t count, const bool& value, const Allocator& alloc_ = Allocator()) : elems((count + word_bit - 1) / word_bit, alloc_), sz(count) {
        if (value) {
            std::fill(elems.begin(), elems.end(), ~0ULL);
            std::size_t last_bits = bit_index(sz);
            if (last_bits != 0) elems.back() &= (1ULL << last_bits) - 1;
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

    constexpr vector& operator=(vector&& other) noexcept(mystd::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || mystd::allocator_traits<Allocator>::is_always_equal::value) {
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
            elems.assign((count + word_bit - 1) / word_bit, ~0ULL);
            std::size_t last_bits = bit_index(count);
            if (last_bits != 0) elems.back() &= (1ULL << last_bits) - 1;
        } else elems.assign((count + word_bit - 1) / word_bit, 0ULL);
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
        if (sz % word_bit == 0) elems.push_back(0ULL);
        ++sz;
        std::size_t w = word_index(index);
        std::size_t b = bit_index(index);
        for (auto i = elems.end() - 1; i != elems.begin() + w; --i) {
            *i = (*i << 1) | (*(i - 1) >> (word_bit - 1));
        }
        if (b != 0) {
            unsigned long long mask = (1ULL << b) - 1;
            elems[w] = (elems[w] & mask) | ((elems[w] & ~mask) << 1);
        } else elems[w] <<= 1;
        if (value) elems[w] |= (1ULL << b);
        else elems[w] &= ~(1ULL << b);
        unsigned long long last_bits = bit_index(sz);
        if (last_bits != 0) elems.back() &= (1ULL << last_bits) - 1;
        else elems.pop_back();
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
                    elems[sw] |= ~((sb == 0) ? 0ULL : ((1ULL << sb) - 1)) & ((1ULL << eb) - 1);
                    if (eb != 0) elems[sw] &= (1ULL << eb) - 1;
                } else {
                    elems[sw] |= ~((1ULL << sb) - 1);
                    for (std::size_t w = sw + 1; w < ew; ++w) elems[w] = ~0ULL;
                    if (eb != 0) elems[ew] |= (1ULL << eb) - 1;
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
        if (bit_diff == 0) std::memmove(&elems[ew], &elems[sw], ((sz + word_bit - 1) / word_bit - sw) * sizeof(unsigned long long));
        else if (bit_diff > 0) {
            if (word_diff != 0) {
                for (auto i = elems.end() - 1; i != elems.begin() + ew; --i) {
                    *i = (*(i - word_diff) << bit_diff) | (*(i - word_diff - 1) >> (word_bit - bit_diff));
                }
                elems[ew] = elems[sw] << bit_diff;
                elems[sw] &= (sb == 0) ? 0ULL : ((1ULL << sb) - 1);
            } else {
                for (auto i = elems.end() - 1; i != elems.begin() + ew; --i) {
                    *i = (*i << bit_diff) | (*(i - 1) >> (word_bit - bit_diff));
                }
                unsigned long long mask = (sb == 0) ? 0ULL : ((1ULL << sb) - 1);
                elems[sw] = (elems[sw] & mask) | ((elems[sw] & ~mask) << bit_diff);
            }
        } else {
            std::size_t rshift = static_cast<std::size_t>(-bit_diff);
            for (auto i = elems.end() - 1; i != elems.begin() + ew - 1; --i) {
                *i = (*(i - word_diff) >> rshift) | (*(i - word_diff + 1) << (word_bit - rshift));
            }
            elems[sw] &= (sb == 0) ? 0ULL : ((1ULL << sb) - 1);
        }
        if (value) {
            if (word_diff == 0) {
                elems[sw] |= ~((sb == 0) ? 0ULL : (1ULL << sb) - 1) & ((1ULL << eb) - 1);
            } else {
                elems[sw] |= ~((1ULL << sb) - 1);
                for (std::size_t w = sw + 1; w < ew; ++w) elems[w] = ~0ULL;
                if (eb != 0) elems[ew] |= (1ULL << eb) - 1;
            }
        } else {
            elems[sw] &= (sb == 0) ? 0ULL : ((1ULL << sb) - 1);
            if (sw != ew) {
               for (std::size_t w = sw + 1; w < ew; ++w) elems[w] = 0ULL;
               if (eb != 0) elems[ew] &= ~((1ULL << eb) - 1);
            }
        }
        sz = new_sz;
        unsigned long long last_bits = bit_index(sz);
        if (last_bits != 0) elems.back() &= (1ULL << last_bits) - 1;
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
            pop_back();
            return iterator(elems.data(), index);
        }
        std::size_t w = word_index(index);
        std::size_t b = bit_index(index);
        if (b != 0) {
            elems[w] = (elems[w] & ((1ULL << b) - 1)) | ((elems[w] & ~((1ULL << (b + 1)) - 1)) >> 1);
        } else elems[w] >>= 1;
        if (w + 1 < elems.size()) elems[w] |= (elems[w + 1] & 1) << (word_bit - 1);
        for (auto i = elems.begin() + w + 1; i != elems.end() - 1; ++i) {
            *i = (*i >> 1) | (*(i + 1) & 1) << (word_bit - 1);
        }
        if (w < elems.size() - 1) *(elems.end() - 1) >>= 1;
        if (last_bits != 0) elems.back() &= (1ULL << last_bits) - 1;
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
            if (last_bits != 0) elems[last_word] &= (1ULL << last_bits) - 1;
            if (elems.size() > last_word + 1) elems.resize(last_word + 1);
            return iterator(elems.data(), index);
        }
        count += b;
        std::size_t word_diff = count / word_bit;
        std::size_t bit_left  = count % word_bit;
        if (bit_left == b) {
            if (b != 0) {
                elems[w] = (elems[w] & ((1ULL << b) - 1)) | (elems[w + word_diff] & ~((1ULL << b) - 1));
                if (w + word_diff + 1 < elems.size()) std::memmove(&elems[w + 1], &elems[w + word_diff + 1], (elems.size() - w - word_diff - 1) * sizeof(unsigned long long));
            } else std::memmove(&elems[w], &elems[w + word_diff], (elems.size() - w - word_diff) * sizeof(unsigned long long));
        } else if (bit_left < b) {
            std::size_t bit_diff = b - bit_left;
            elems[w] = (elems[w] & ((1ULL << b) - 1)) | ((elems[w + word_diff] & ~((1ULL << bit_left) - 1)) << bit_diff);
            for (auto i = elems.begin() + w + 1; i != elems.begin() + last_word + 1; ++i) {
                *i = (*(i + word_diff - 1) >> (word_bit - bit_diff)) | (*(i + word_diff) << bit_diff);
            }
        } else {
            std::size_t bit_diff = bit_left - b;
            elems[w] = (elems[w] & (((b == 0) ? 0ULL : (1ULL << b)) - 1)) | ((elems[w + word_diff] & ~((1ULL << bit_left) - 1)) >> bit_diff);
            if (w + word_diff + 1 < elems.size()) elems[w] |= elems[w + word_diff + 1] << (word_bit - bit_diff);
            for (auto i = elems.begin() + w + 1; i != elems.begin() + last_word; ++i) {
                *i = (*(i + word_diff) >> bit_diff) | (*(i + word_diff + 1) << (word_bit - bit_diff));
            }
            elems[last_word] = elems[last_word + word_diff] >> bit_diff;
            if (last_word + word_diff + 1 < elems.size()) {
                elems[last_word] |= elems[last_word + word_diff + 1] << (word_bit - bit_diff);
            }
        }
        if (last_bits != 0) elems[last_word] &= (1ULL << last_bits) - 1;
        if (elems.size() > last_word + 1) elems.resize(last_word + 1);
        return iterator(elems.data(), index);
    }

    constexpr void push_back(const bool& value) {
        if (sz == capacity()) reserve(sz ? sz * _MYSTD_VECTOR_GROW : 1);
        std::size_t b = bit_index(sz);
        if (b == 0) elems.push_back(0ULL);
        if (value) elems.back() |= 1ULL << b;
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
            elems.resize((new_size + word_bit - 1) / word_bit, ~0ULL);
            if (new_size > sz) {
                std::size_t last_bits = bit_index(sz);
                if (last_bits != 0) elems[word_index(sz)] |= ~((1ULL << last_bits) - 1);
            }
            std::size_t last_bits = bit_index(new_size);
            if (last_bits != 0) elems.back() &= (1ULL << last_bits) - 1;
            sz = new_size;
        } else resize(new_size);
    }

    constexpr void swap(vector<bool, Allocator>& other) noexcept(mystd::allocator_traits<Allocator>::propagate_on_container_swap::value || mystd::allocator_traits<Allocator>::is_always_equal::value) {
        elems.swap(other.elems);
        std::swap(sz, other.sz);
    }

    constexpr void flip() {
        if (sz == 0) return;
        for (auto& word : elems) word = ~word;
        std::size_t last_bits = bit_index(sz);
        if (last_bits != 0) elems.back() &= (1ULL << last_bits) - 1;
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
