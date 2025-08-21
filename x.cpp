#include <iostream>
#include <vector>
#include <typeinfo>
#include <cxxabi.h>

int main() {
    int status;
    
    auto print_type = [&status](const char* desc, const std::type_info& ti) {
        char* realname = abi::__cxa_demangle(ti.name(), 0, 0, &status);
        std::cout << desc << ": " << realname << "\n";
        std::free(realname);
    };
    
    std::cout << "=== std::vector<bool> ===\n";
    print_type("value_type", typeid(std::vector<bool>::value_type));
    print_type("allocator_type", typeid(std::vector<bool>::allocator_type));
    print_type("reference", typeid(std::vector<bool>::reference));
    print_type("const_reference", typeid(std::vector<bool>::const_reference));
    print_type("pointer", typeid(std::vector<bool>::pointer));
    print_type("const_pointer", typeid(std::vector<bool>::const_pointer));
    
    std::cout << "\n=== std::vector<int> ===\n";
    print_type("value_type", typeid(std::vector<int>::value_type));
    print_type("allocator_type", typeid(std::vector<int>::allocator_type));
    print_type("reference", typeid(std::vector<int>::reference));
    print_type("const_reference", typeid(std::vector<int>::const_reference));
    print_type("pointer", typeid(std::vector<int>::pointer));
    print_type("const_pointer", typeid(std::vector<int>::const_pointer));
    
    return 0;
}
