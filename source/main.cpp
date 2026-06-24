#include "runtime/vm.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <print>

int main(int argc, char** argv) {
    try {
        const char* main_class = argc >= 2 ? argv[1] : "gothic3thebeginning/HG";

        VM vm(main_class);

        if (argc >= 3) {
            for (int i = 2; i < argc; ++i) {
                vm.add_classpath_entry(argv[i]);
            }
        }
        else {
            vm.add_classpath_entry(std::filesystem::current_path());
        }

        vm.run();
    }
    catch (const std::exception& e) {
        std::println(stderr, "VM error: {}", e.what());
        return 1;
    }

    return 0;
}
