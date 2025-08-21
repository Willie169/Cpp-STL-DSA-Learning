#include <iostream>
#include <vector>
#include <typeinfo>
#include <cxxabi.h>
int main() {
    int status;
    std::cout << *(abi::__cxa_demangle(typeid(std::vector<bool, std::allocator<int>>::allocator_type).name(), 0, 0, &status)) << "\n";
    return 0;
}
