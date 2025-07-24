#include <iostream>
#include <typeinfo>
#include "Vector.hpp"

void test_vector() {
    using std::cout;

    Vector<int> v;
    cout << "Default constructed, empty: " << v.empty() << '\n';

    for (int i = 0; i < 5; ++i) v.push_back(i * 10);
    cout << "After push_back: ";
    for (auto x : v) cout << x << " ";
    cout << '\n';

    cout << "at(2): " << v.at(2) << ", front: " << v.front() << ", back: " << v.back() << '\n';
    try {
        auto five = v.at(5);
    } catch (const std::exception& e) {
	cout << "at(5): exception type: " << typeid(e).name() << ", what(): " << e.what() << '\n';
    }

    Vector<int> v2 = v;
    cout << "Copied vector: ";
    for (auto x : v2) cout << x << " ";
    cout << '\n';

    Vector<int> v3= std::move(v2);
    cout << "Moved vector: ";
    for (auto x : v3) cout << x << " ";
    cout << '\n';

    Vector<int> v4 = {100, 200, 300};
    cout << "Initializer list: ";
    for (auto x : v4) cout << x << " ";
    cout << '\n';

    v4.assign(4, 42);
    cout << "Assign 4x42: ";
    for (auto x : v4) cout << x << " ";
    cout << '\n';

    int arr[] = {5, 6, 7, 8};
    v4.assign(arr, arr + 4);
    cout << "Assign range: ";
    for (auto x : v4) cout << x << " ";
    cout << '\n';

    v4.resize(6, 99);
    cout << "Resize to 6 (fill 99): ";
    for (auto x : v4) cout << x << " ";
    cout << '\n';

    v4.resize(3);
    cout << "Resize to 3: ";
    for (auto x : v4) cout << x << " ";
    cout << '\n';

    v4.reserve(20);
    cout << "Reserve 20, cap = " << v4.capacity() << '\n';
    v4.shrink_to_fit();
    cout << "Shrink to fit, cap = " << v4.capacity() << '\n';

    v4.insert(v4.begin() + 1, 88);
    cout << "Insert 88 at pos 1: ";
    for (auto x : v4) cout << x << " ";
    cout << '\n';

    v4.insert(v4.begin() + 2, 2, 77);
    cout << "Insert 2x77 at pos 2: ";
    for (auto x : v4) cout << x << " ";
    cout << '\n';

    v4.insert(v4.begin(), arr, arr + 2);
    cout << "Insert range at begin: ";
    for (auto x : v4) cout << x << " ";
    cout << '\n';

    v4.insert(v4.end(), {111, 222});
    cout << "Insert init list at end: ";
    for (auto x : v4) cout << x << " ";
    cout << '\n';

    v4.emplace(v4.begin() + 1, 333);
    v4.emplace_back(444);
    cout << "After emplace and emplace_back: ";
    for (auto x : v4) cout << x << " ";
    cout << '\n';

    v4.erase(v4.begin() + 1);
    cout << "After erase pos 1: ";
    for (auto x : v4) cout << x << " ";
    cout << '\n';

    v4.erase(v4.begin() + 2, v4.begin() + 4);
    cout << "After erase range [2,4): ";
    for (auto x : v4) cout << x << " ";
    cout << '\n';

    v4.pop_back();
    cout << "After pop_back: ";
    for (auto x : v4) cout << x << " ";
    cout << '\n';

    v4.clear();
    cout << "After clear, empty: " << v4.empty() << '\n';

    Vector<int> a = {1, 2, 3}, b = {9, 8};
    a.swap(b);
    cout << "After swap, a: ";
    for (auto x : a) cout << x << " ";
    cout << " b: ";
    for (auto x : b) cout << x << " ";
    cout << '\n';

    Vector<int> v5 = {1, 2, 3};
    cout << "Iteration: ";
    for (int i : v5) cout << i;
    cout << "\nReverse Iteration: ";
    for (auto i = v5.rbegin(); i < v5.rend(); ++i) cout << *i;
    cout << "\n";
}

int main() {
    test_vector();
    return 0;
}

