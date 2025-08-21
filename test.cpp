#include "Vector.hpp"
#include <bits/stdc++.h>
using namespace std;

// --- Utilities ---------------------------------------------------------------

#define CHECK(expr) do { \
    if(!(expr)) { \
        cerr << "CHECK failed: " #expr " at " << __FILE__ << ":" << __LINE__ << "\n"; \
        std::terminate(); \
    } \
} while(0)

template<class VB>
static bool equal_to_std(const VB& v) {
    std::vector<bool> s;
    s.reserve(v.size());
    for (size_t i = 0; i < v.size(); ++i) s.push_back(static_cast<bool>(v[i]));
    if (s.size() != v.size()) return false;
    for (size_t i = 0; i < v.size(); ++i) if (s[i] != static_cast<bool>(v[i])) return false;
    // also test const_iterator view
    size_t i = 0;
    for (auto it = v.cbegin(); it != v.cend(); ++it, ++i) {
        if (static_cast<bool>(*it) != s[i]) return false;
    }
    return true;
}

template<class VB>
static void assert_all_zero(const VB& v) {
    for (size_t i = 0; i < v.size(); ++i) CHECK(static_cast<bool>(v[i]) == false);
}

template<class VB>
static void assert_all_one(const VB& v) {
    for (size_t i = 0; i < v.size(); ++i) CHECK(static_cast<bool>(v[i]) == true);
}

template<class VB>
static void fill_pattern(VB& v, size_t n, int pat) {
    v.clear();
    v.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        bool b = false;
        if (pat == 0) b = false;                               // all 0
        else if (pat == 1) b = true;                           // all 1
        else if (pat == 2) b = (i % 2);                        // 0101...
        else if (pat == 3) b = ((i / 3) % 2);                  // 000111000111...
        else if (pat == 4) b = (i % 7 == 0);                   // sparse
        else {                                                 // random but deterministic
            static std::mt19937_64 rng(123456789);
            b = std::uniform_int_distribution<int>(0,1)(rng);
        }
        v.push_back(b);
    }
}

template<class Word>
using VB = Vector<bool, std::allocator<Word>>;

template<class VB>
static void test_basic_ctor_and_size() {
    VB v;
    CHECK(v.size() == 0);
    CHECK(v.empty());
    v.reserve(1);
    CHECK(v.capacity() >= 1);
    VB v2(13);
    CHECK(v2.size() == 13);
    assert_all_zero(v2);

    VB v3(17, true);
    CHECK(v3.size() == 17);
    assert_all_one(v3);

    VB v4{false,true,true,false,true,false};
    CHECK(v4.size() == 6);
    CHECK(static_cast<bool>(v4[0]) == false);
    CHECK(static_cast<bool>(v4[1]) == true);
}

template<class VB>
static void test_proxy_and_pointer() {
    VB v(5, false);
    // reference proxy via operator[]
    v[0] = true;
    CHECK(static_cast<bool>(v[0]) == true);
    // copy-assign from reference
    v[1] = v[0];
    CHECK(static_cast<bool>(v[1]) == true);
    // flip() on reference
    v[1].flip();
    CHECK(static_cast<bool>(v[1]) == false);
    // pointer proxy (iterator->)
    auto it = v.begin();
    auto p = it.operator->();
    p->flip(); // flips bit 0
    CHECK(static_cast<bool>(v[0]) == false);
}

template<class VB>
static void test_front_back_at() {
    VB v{false,true,false};
    CHECK(static_cast<bool>(v.front()) == false);
    CHECK(static_cast<bool>(v.back())  == false);
    v.back() = true;
    CHECK(static_cast<bool>(v.back())  == true);

    // at() bounds
    CHECK(v.at(0) == false);
    CHECK(v.at(1) == true);
    CHECK(v.at(2) == true);
    bool threw = false;
    try { (void)v.at(3); } catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);
}

template<class VB>
static void test_iterators_and_random_access_ops() {
    VB v;
    for (int i = 0; i < 50; ++i) v.push_back(i%3==0);
    static_assert(std::random_access_iterator<typename VB::iterator>);
    static_assert(std::random_access_iterator<typename VB::const_iterator>);

    auto b = v.begin();
    auto e = v.end();
    CHECK((e - b) == static_cast<typename VB::difference_type>(v.size()));
    CHECK(*(b + 3) == (v[3]));
    CHECK(*(e - 1) == v.back());

    // const_iterator conversion
    typename VB::const_iterator cb = v.begin();
    CHECK(*cb == static_cast<bool>(v[0]));

    // reverse iterators
    auto rb = v.rbegin();
    CHECK(static_cast<bool>(*rb) == static_cast<bool>(v.back()));
}

template<class VB>
static void test_push_pop_resize_shrink() {
    VB v;
    for (int i = 0; i < 100; ++i) v.push_back(i&1);
    CHECK(v.size() == 100);
    CHECK(static_cast<bool>(v.back()) == true);

    v.pop_back();
    CHECK(v.size() == 99);
    CHECK(static_cast<bool>(v.back()) == false);

    auto cap_before = v.capacity();
    v.shrink_to_fit();
    CHECK(v.capacity() >= v.size()); // not strictly required to shrink below size

    v.resize(150, true);
    CHECK(v.size() == 150);
    for (size_t i = 100; i < 150; ++i) CHECK(static_cast<bool>(v[i]) == true);

    v.resize(37);
    CHECK(v.size() == 37);
}

template<class VB>
static void test_assign_overloads() {
    VB v;
    v.assign(25, true);
    assert_all_one(v);
    CHECK(v.size() == 25);

    std::vector<int> src(17);
    for (int i = 0; i < 17; ++i) src[i] = (i%4==0);
    v.assign(src.begin(), src.end());
    CHECK(v.size() == 17);
    for (int i = 0; i < 17; ++i) CHECK(static_cast<bool>(v[i]) == (i%4==0));

    v.assign({0,1,1,0,1,0,0,1});
    CHECK(v.size() == 8);
    CHECK(static_cast<bool>(v[1]) == true);
    CHECK(static_cast<bool>(v[0]) == false);
}

template<class VB>
static void test_insert_single_all_positions() {
    VB v;
    for (int i = 0; i < 10; ++i) v.push_back(i&1); // 0101010101
    // insert at beginning
    v.insert(v.begin(), true); // 1 0101010101
    CHECK(v.size() == 11);
    CHECK(static_cast<bool>(v[0]) == true);
    // insert in middle
    v.insert(v.begin() + 5, false);
    CHECK(static_cast<bool>(v[5]) == false);
    // insert at end
    v.insert(v.end(), true);
    CHECK(static_cast<bool>(v.back()) == true);
}

template<class VB>
static void test_insert_count_edges() {
    VB v;
    // make sure we cross word boundaries
    const size_t WB = VB::word_bit;
    const size_t N = WB*2 + 7;
    v.assign(N, false);

    // insert true block exactly at boundary
    v.insert(v.begin() + WB, WB, true);
    CHECK(v.size() == N + WB);
    for (size_t i = WB; i < WB*2; ++i) CHECK(static_cast<bool>(v[i]) == true);

    // insert zero count is no-op
    auto before = v.size();
    v.insert(v.begin()+3, 0, true);
    CHECK(v.size() == before);
}

template<class VB>
static void test_insert_range_and_ilist() {
    VB v;
    v.assign(8, false);
    std::vector<int> src = {1,0,1,1,0};
    v.insert(v.begin()+3, src.begin(), src.end());
    CHECK(static_cast<bool>(v[3]) == true);
    CHECK(static_cast<bool>(v[4]) == false);
    CHECK(static_cast<bool>(v[5]) == true);
    CHECK(static_cast<bool>(v[6]) == true);
    CHECK(static_cast<bool>(v[7]) == false);

    v.insert(v.end(), {1,1,0,0});
    CHECK(static_cast<bool>(v[v.size()-4]) == true);
    CHECK(static_cast<bool>(v[v.size()-1]) == false);
}

template<class VB>
static void test_emplace_and_emplace_at() {
    VB v{false,false,false};
    v.emplace_back(true);
    CHECK(static_cast<bool>(v.back()) == true);
    v.emplace(v.begin()+1, false); // should behave like insert(pos, value)
    CHECK(static_cast<bool>(v[1]) == false);
}

template<class VB>
static void test_erase_single_and_range() {
    VB v;
    for (int i = 0; i < 70; ++i) v.push_back(i % 3 == 0);
    const size_t WB = VB::word_bit;
    // Erase single
    for (size_t offset = 0; offset < WB; ++offset) {
        if (v.size() > WB + offset) {
            bool next = static_cast<bool>(v[WB + offset]);
            v.erase(v.begin() + (WB - 1 + offset));
            CHECK(static_cast<bool>(v[WB - 1 + offset]) == next);
        }
    }
    // Erase range
    size_t old_size = v.size();
    auto it = v.erase(v.begin() + 5, v.begin() + 33);
    CHECK(v.size() == old_size - (33 - 5));
    CHECK((it - v.begin()) == 5);
    // Erase to end
    v.erase(v.begin() + 10, v.end());
    CHECK(v.size() == 10);
    // Erase ranges of various sizes and alignment
    v = {0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 ,1, 0 ,1, 1, 1, 1, 1};
    std::vector<std::pair<size_t, size_t>> ranges = {
        {3, 5}, {1, 7}, {3, 8}, {0, 8}, {0, 9}, {3, 12}, {3, 9}, {10, 23}, {21, 22}
    };
    for (auto [start, end] : ranges) {
        auto t = v;
        std::vector<bool> u(v.begin(), v.end());
        auto it = t.erase(t.begin() + start, t.begin() + end);
        u.erase(u.begin() + start, u.begin() + end);
        CHECK(t.size() == u.size());
        for (size_t i = 0; i < t.size(); ++i) {
            CHECK(static_cast<bool>(t[i]) == static_cast<bool>(u[i]));
        }
    }
}

template<class VB>
static void test_clear_swap_copy_move() {
    VB a{0,1,1,0,1};
    VB b{1,0};
    a.swap(b);
    CHECK(b.size() == 5 && a.size() == 2);
    CHECK(static_cast<bool>(b[0]) == 0 && static_cast<bool>(b[1]) == 1);

    VB c = b;                  // copy
    CHECK(equal_to_std(c));
    VB d = std::move(b);       // move
    CHECK(equal_to_std(d));

    c.clear();
    CHECK(c.size() == 0 && c.empty());
}

template<class VB>
static void test_flip_and_masking_tail() {
    // size not a multiple of word_bit -> tail bits must be masked after flip
    const size_t WB = VB::word_bit;
    VB v(WB + 5, false);
    // set some
    v[0] = true; v[WB] = true; v[WB+4] = true;
    v.flip();
    // flipping should invert only first (WB+5) bits
    CHECK(static_cast<bool>(v[0]) == false);
    CHECK(static_cast<bool>(v[WB]) == false);
    CHECK(static_cast<bool>(v[WB+4]) == false);
    // and all others up to size are true now
    for (size_t i = 0; i < v.size(); ++i) {
        if (i==0 || i==WB || i==WB+4) continue;
        CHECK(static_cast<bool>(v[i]) == true);
    }
}

template<class VB>
static void test_static_reference_swap() {
    VB v{0,1,0};
    auto r0 = v[0];
    auto r1 = v[1];
    Vector<bool, typename VB::allocator_type>::swap(r0, r1);
    CHECK(static_cast<bool>(v[0]) == true);
    CHECK(static_cast<bool>(v[1]) == false);
}

template<class VB>
static void test_hash_basic_changes() {
    VB v;
    for (int i=0;i<100;++i) v.push_back((i%5)==0);
    std::hash<VB> H;
    auto h1 = H(v);
    v[3] = !static_cast<bool>(v[3]);
    auto h2 = H(v);
    // it's possible but unlikely to collide; at least assert determinism
    CHECK(h1 != H(v)); // after reverting…
    v[3].flip();       // revert
    CHECK(H(v) == h1);
}

template<class VB>
static void test_at_randomized_equivalence() {
    VB v;
    const size_t WB = VB::word_bit;
    std::vector<bool> s;
    std::mt19937 rng(42);
    for (int t = 0; t < 10000; ++t) {
        int op = rng() % 9;
        std::string op_name;
        size_t idx = 0;
        size_t idx1 = 0;
        bool val = false;
        size_t cnt = 0;
        if (op == 0) { // push_back
            op_name = "push_back";
            val = rng() & 1;
            v.push_back(val);
            s.push_back(val);
        }
        else if (op == 1 && !s.empty()) { // pop_back
            op_name = "pop_back";
            v.pop_back();
            s.pop_back();
        }
        else if (op == 2 && !s.empty()) { // set at idx
            op_name = "set";
            idx = rng() % s.size();
            val = rng() & 1;
            v[idx] = val;
            s[idx] = val;
        }
        else if (op == 3) { // insert single
            idx = s.empty() ? 0 : (rng() % (s.size() + 1));
            val = rng() & 1;
            v.insert(v.begin() + idx, val);
            s.insert(s.begin() + idx, val);
            op_name = "insert_single_" + to_string(idx) + "_" + to_string(val);
        }
        else if (op == 4 && !s.empty()) { // erase single
            op_name = "erase_single";
            idx = rng() % s.size();
            v.erase(v.begin() + idx);
            s.erase(s.begin() + idx);
        }
        else if (op == 5) { // resize
            op_name = "resize";
            size_t newn = rng() % 150;
            val = rng() & 1;
            v.resize(newn, val);
            s.resize(newn, val);
        }
        else if (op == 6) { // flip
            op_name = "flip";
            v.flip();
            for (auto&& x : s) x = !x;
        }
        else if (op == 7) { // insert count
            op_name = "insert_count";
            idx = s.empty() ? 0 : (rng() % (s.size() + 1));
            cnt = rng() % 40;
            val = rng() & 1;
            v.insert(v.begin() + idx, cnt, val);
            s.insert(s.begin() + idx, cnt, val);
        } else if (!s.empty()) { // erase range
            op_name = "erase_range";
            idx = rng() % s.size();
            idx1 = rng() % s.size();
            if (idx1 < idx) {
                size_t tmp = idx;
                idx = idx1;
                idx1 = tmp;
            }
            v.erase(v.begin() + idx, v.begin() + idx1);
            s.erase(s.begin() + idx, s.begin() + idx1);
        }
        // verify equality
        if (v.size() != s.size()) {
            std::cerr << "Size mismatch after operation " << op_name << "\n";
            std::cerr << "v.size() = " << v.size() << ", s.size() = " << s.size() << "\n";
            std::terminate();
        }
        for (size_t i = 0; i < s.size(); ++i) {
            if (static_cast<bool>(v[i]) != static_cast<bool>(s[i])) {
                std::cerr << "Value mismatch after operation " << op_name << " at index " << i << ": v[" << i << "] = " << static_cast<bool>(v[i]) << ", s[" << i << "] = " << s[i] << ", v.size() = " << v.size() << ", s.size() = " << s.size() << "\n";
                std::terminate();
            }
        }
        for (size_t i = s.size(); i < (v.size() + WB - 1) / WB * WB; ++i) CHECK(v[i] == 0);
    }
}

// --- Compile-time constraints smoke test ------------------------------------

template<class Alloc>
concept GoodAllocator =
    std::is_integral_v<typename Alloc::value_type> &&
    std::is_unsigned_v<typename Alloc::value_type> &&
    !std::is_same_v<bool, typename Alloc::value_type>;

// Dummy allocators to probe the requires-clause:
template<class T> struct UnsignedOKAlloc {
    using value_type = T;
    UnsignedOKAlloc() = default;
    template<class U> UnsignedOKAlloc(const UnsignedOKAlloc<U>&) {}
    T* allocate(std::size_t n) { return std::allocator<T>{}.allocate(n); }
    void deallocate(T* p, std::size_t n) { std::allocator<T>{}.deallocate(p,n); }
};
static_assert(GoodAllocator<UnsignedOKAlloc<unsigned char>>);
static_assert(GoodAllocator<std::allocator<unsigned int>>);

// Uncommenting the following should fail substitution (as intended):
// using Bad1 = Vector<bool, std::allocator<signed int>>;
// using Bad2 = Vector<bool, std::allocator<bool>>;

// --- Runner for one word_type -----------------------------------------------

template<class Word>
static void run_suite_for_word() {
    using V = VB<Word>;
    cout << "Running suite for word_type = " << typeid(Word).name()
         << " (word_bit=" << V::word_bit << ")\n";

    test_basic_ctor_and_size<V>();
    cout << "✔ test_basic_ctor_and_size passed\n";

    test_proxy_and_pointer<V>();
    cout << "✔ test_proxy_and_pointer passed\n";

    test_front_back_at<V>();
    cout << "✔ test_front_back_at passed\n";

    test_iterators_and_random_access_ops<V>();
    cout << "✔ test_iterators_and_random_access_ops passed\n";

    test_push_pop_resize_shrink<V>();
    cout << "✔ test_push_pop_resize_shrink passed\n";

    test_assign_overloads<V>();
    cout << "✔ test_assign_overloads passed\n";

    test_insert_single_all_positions<V>();
    cout << "✔ test_insert_single_all_positions passed\n";

    test_insert_count_edges<V>();
    cout << "✔ test_insert_count_edges passed\n";

    test_insert_range_and_ilist<V>();
    cout << "✔ test_insert_range_and_ilist passed\n";

    test_emplace_and_emplace_at<V>();
    cout << "✔ test_emplace_and_emplace_at passed\n";

    test_erase_single_and_range<V>();
    cout << "✔ test_erase_single_and_range passed\n";

    test_clear_swap_copy_move<V>();
    cout << "✔ test_clear_swap_copy_move passed\n";

    test_flip_and_masking_tail<V>();
    cout << "✔ test_flip_and_masking_tail passed\n";

    test_static_reference_swap<V>();
    cout << "✔ test_static_reference_swap passed\n";

    test_hash_basic_changes<V>();
    cout << "✔ test_hash_basic_changes passed\n";

    test_at_randomized_equivalence<V>();
    cout << "✔ test_at_randomized_equivalence passed\n";

    // final sanity
    V v;
    fill_pattern(v, V::word_bit * 3 + 5, 5);
    CHECK(equal_to_std(v));
    cout << "✔ final sanity check passed\n";
}

// --- main --------------------------------------------------------------------

int main() {
    // Run the full suite for multiple word sizes to catch boundary bugs.
    run_suite_for_word<unsigned char>();       // 8-bit
    cout << "✔ run_suite_for_word<unsigned char> passed\n";
    run_suite_for_word<unsigned short>();      // 16-bit (usually)
    cout << "✔ run_suite_for_word<unsigned short> passed\n";
    run_suite_for_word<unsigned int>();        // 32-bit (usually)
    cout << "✔ run_suite_for_word<unsigned int> passed\n";
    run_suite_for_word<unsigned long long>();  // 64-bit (usually)
    cout << "✔ run_suite_for_word<unsigned long long> passed\n";

    cout << "All tests passed ✅\n";
    return 0;
}

