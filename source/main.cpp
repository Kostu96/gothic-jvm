#include "runtime/class.hpp"

#include <cstdio>
#include <format>
#include <fstream>
#include <print>

int main()
{
    Class hg_class("gothic3thebeginning/HG.class");

    std::ofstream out("HG.class.txt");
    if (!out) {
        std::println(stderr, "Failed to open output file");
        return 1;
    }

    out << std::format("Class name: {}\n", hg_class.name());
    out << std::format("Super class name: {}\n", hg_class.super_name());

    out << "Methods:\n";
    for (const auto& m : hg_class.methods()) {
        out << std::format("  {}{}\n", m.name, m.descriptor);

        if (!m.code.empty()) {
            out << std::format("      max_stack       = {}\n", m.max_stack);
            out << std::format("      max_locals      = {}\n", m.max_locals);
            out << std::format("      code_length     = {}\n", m.code.size());
            out << "      code            =";
            for (size_t k = 0; k < m.code.size(); ++k) {
                if (k % 16 == 0) {
                    out << "\n        ";
                }
                out << std::format(" {:02X}", std::to_integer<uint8_t>(m.code[k]));
            }
            out << '\n';
            out << std::format("      exception_table = {} entry(ies)\n", m.exception_table.size());
            for (const auto& e : m.exception_table) {
                out << std::format("        start_pc={} end_pc={} handler_pc={} catch_type={}\n",
                    e.start_pc, e.end_pc, e.handler_pc, e.catch_type);
            }
        }
    }

    return 0;
}
