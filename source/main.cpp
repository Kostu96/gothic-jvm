#include "runtime/vm.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <print>

/*
 * args <main_class> [<class_path_entry>...]
 */
int main(int argc, char** argv) {
    try {
        const char* main_class = argc >= 2 ? argv[1] : "HG";

        VM vm;

        if (argc >= 3) {
            for (int i = 2; i < argc; ++i) {
                vm.class_loader().add_classpath_entry(argv[i]);
            }
        }
        else {
            vm.class_loader().add_classpath_entry(std::filesystem::current_path() / "gothic3thebeginning");
        }

        vm.run(main_class);
    }
    catch (const std::exception& e) {
        std::println(stderr, "{}", e.what());
        return 1;
    }

    return 0;
}
