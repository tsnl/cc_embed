#include <tsnl/cc_embed/hello_binary_512.hpp>
#include <tsnl/cc_embed/hello_world.hpp>

#include <fstream>

#include <gtest/gtest.h>

TEST(hello_world, test) {
    std::string s(reinterpret_cast<char const*>(tsnl::cc_embed::hello_world.data()));
    EXPECT_EQ(s, "hello world");
    EXPECT_EQ(tsnl::cc_embed::hello_world.back(), std::byte{0});
}

TEST(hello_binary_512, test) {
    std::ifstream reference_file("data/hello_binary_512.bin", std::ios::binary);
    EXPECT_TRUE(bool(reference_file));

    reference_file.seekg(0, std::ios::end);
    auto file_size = reference_file.tellg();
    reference_file.seekg(0, std::ios::beg);
    EXPECT_EQ(file_size, tsnl::cc_embed::hello_binary_512.size());

    for (auto b : tsnl::cc_embed::hello_binary_512) {
        EXPECT_EQ(b, std::byte(reference_file.get()));
    }
}
