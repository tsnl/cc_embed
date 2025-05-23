#include <tsnl/log.hpp>

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <regex>
#include <string_view>
#include <utility>
#include <vector>

using namespace tsnl;
using namespace std::literals;

constexpr auto bytes_per_row = 16;
static_assert(bytes_per_row >= 1);

constexpr auto header_template = R"cpp(// Auto-generated by embed_cpp, do not edit
#pragma once

#include <span>

#include <cstddef>

namespace tsnl::cc_embed {{

extern std::span<const std::byte> {0};

}} // namespace tsnl::cc_embed
)cpp"sv;

constexpr auto source_template_prefix = R"cpp(// Auto-generated by embed_cpp, do not edit
#include <array>
#include <span>

#include <cstddef>
#include <cstdint>

namespace tsnl::cc_embed {{

// std::byte does not admit integer literals, so we use std::array<std::uint8_t, N> instead and reinterpret_cast the 
// data to std::byte* later.
static_assert(sizeof(std::byte) == sizeof(uint8_t));
static auto {0}_storage = std::to_array<uint8_t>({{
)cpp"sv;

constexpr auto source_template_suffix = R"cpp(
}});

std::span<const std::byte> {0}(reinterpret_cast<const std::byte*>({0}_storage.data()), {0}_storage.size());

}} // namespace tsnl::cc_embed
)cpp";

auto main(int argc, char const* argv[]) -> int {
    if (argc != 6) {
        log::fatal(
        ) << "Incorrect CLI args: "
          << "usage: " << argv[0]
          << " <input_file> <variable_name> <output_header_file> <output_source_file> <append_null_terminator>";
        std::unreachable();
    }

    std::filesystem::path input_file = argv[1];
    std::string var_name = argv[2];
    std::filesystem::path output_header_file = argv[3];
    std::filesystem::path output_source_file = argv[4];
    bool append_null_terminator = strcmp(argv[5], "1") == 0;

    // Validate variable name:
    {
        std::regex var_name_regex(R"([a-zA-Z_][a-zA-Z0-9_]*)");
        if (!std::regex_match(var_name, var_name_regex)) {
            log::fatal() << "Invalid variable name: " << var_name;
            std::unreachable();
        }
    }

    // Read binary file:
    std::vector<std::byte> bin_data;
    {
        std::ifstream bin(input_file);
        if (!bin) {
            log::fatal() << "Failed to open input file: " << input_file;
            std::unreachable();
        }

        // Determine bin size:
        bin.seekg(0, std::ios::end);
        auto bin_size = bin.tellg();
        bin.seekg(0, std::ios::beg);

        // Account for null terminator in size if requested:
        if (append_null_terminator) {
            bin_size += 1;
        }

        // Read bin data:
        bin_data.resize(bin_size);
        bin.read(reinterpret_cast<char*>(bin_data.data()), bin_size);
        if (append_null_terminator) {
            bin_data.back() = std::byte{0};
        }
    }

    // Write C++ header file:
    {
        std::filesystem::create_directories(output_header_file.parent_path());
        std::ofstream hdr(output_header_file);
        if (!hdr) {
            log::fatal() << "Failed to open output header file: " << output_header_file;
            std::unreachable();
        }

        hdr << std::vformat(header_template, std::make_format_args(var_name));
    }

    // Write C++ source file:
    {
        std::filesystem::create_directories(output_source_file.parent_path());
        std::ofstream src(output_source_file);
        if (!src) {
            log::fatal() << "Failed to open output source file: " << output_source_file;
            std::unreachable();
        }

        src << std::vformat(source_template_prefix, std::make_format_args(var_name));

        for (size_t i = 0; i < bin_data.size(); ++i) {
            src << "0x" << std::hex << std::setw(2) << std::setfill('0') << std::to_integer<int>(bin_data[i]);
            if (i != bin_data.size() - 1) {
                if (i % bytes_per_row == bytes_per_row - 1) {
                    src << ",\n";
                } else {
                    src << ", ";
                }
            }
        }

        src << std::vformat(source_template_suffix, std::make_format_args(var_name));
    }

    return 0;
}
