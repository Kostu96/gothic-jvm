#include "utils/binary_reader.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>
#include <vector>

TEST(BinaryReader, ReadsU8) {
    std::vector<std::byte> data{ std::byte{0x42} };
    util::BinaryReader reader(data);

    EXPECT_EQ(reader.read_u8(), 0x42u);
}

TEST(BinaryReader, ReadsU16BigEndian) {
    std::vector<std::byte> data{ std::byte{0x12}, std::byte{0x34} };
    util::BinaryReader reader(data);

    EXPECT_EQ(reader.read_u16(), 0x1234u);
}

TEST(BinaryReader, ReadsU32BigEndian) {
    std::vector<std::byte> data{
        std::byte{0xCA}, std::byte{0xFE}, std::byte{0xBA}, std::byte{0xBE}
    };
    util::BinaryReader reader(data);

    EXPECT_EQ(reader.read_u32(), 0xCAFEBABEu);
}

TEST(BinaryReader, ReadsStringOfGivenLength) {
    std::vector<std::byte> data{
        std::byte{'H'}, std::byte{'i'}, std::byte{'!'}
    };
    util::BinaryReader reader(data);

    EXPECT_EQ(reader.read_string(3), "Hi!");
}

TEST(BinaryReader, ReadBytesReturnsOwnedCopyAndAdvances) {
    std::vector<std::byte> data{
        std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}
    };
    util::BinaryReader reader(data);

    auto first_two = reader.read_bytes(2);
    ASSERT_EQ(first_two.size(), 2u);
    EXPECT_EQ(first_two[0], std::byte{0x01});
    EXPECT_EQ(first_two[1], std::byte{0x02});

    EXPECT_EQ(reader.read_u16(), 0x0304u);
}

TEST(BinaryReader, SkipAdvancesOffset) {
    std::vector<std::byte> data{
        std::byte{0xAA}, std::byte{0xBB}, std::byte{0xCC}, std::byte{0xDD}
    };
    util::BinaryReader reader(data);

    reader.skip(2);

    EXPECT_EQ(reader.read_u16(), 0xCCDDu);
}

TEST(BinaryReader, OverReadThrowsOutOfRange) {
    std::vector<std::byte> data{ std::byte{0x00} };
    util::BinaryReader reader(data);

    EXPECT_THROW((void)reader.read_u16(), std::out_of_range);
}

TEST(BinaryReader, SequentialReadsCoverEntireBuffer) {
    std::vector<std::byte> data{
        std::byte{0xCA}, std::byte{0xFE}, std::byte{0xBA}, std::byte{0xBE},
        std::byte{0x00}, std::byte{0x34}
    };
    util::BinaryReader reader(data);

    EXPECT_EQ(reader.read_u32(), 0xCAFEBABEu);
    EXPECT_EQ(reader.read_u16(), 0x0034u);
    EXPECT_THROW((void)reader.read_u8(), std::out_of_range);
}
