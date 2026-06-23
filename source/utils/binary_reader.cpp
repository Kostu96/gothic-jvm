#include "binary_reader.hpp"

#include <stdexcept>

namespace util {

uint8_t BinaryReader::read_u8() {
    check_avaiable(1);

    return std::to_integer<uint8_t>(buffer_[offset_++]);
}

uint16_t BinaryReader::read_u16() {
    check_avaiable(2);

    uint16_t value =
        (std::to_integer<uint16_t>(buffer_[offset_]) << 8) |
        std::to_integer<uint16_t>(buffer_[offset_ + 1]);

    offset_ += 2;

    return value;
}

uint32_t BinaryReader::read_u32() {
    check_avaiable(4);

    uint32_t value =
        (std::to_integer<uint32_t>(buffer_[offset_]) << 24) |
        (std::to_integer<uint32_t>(buffer_[offset_ + 1]) << 16) |
        (std::to_integer<uint32_t>(buffer_[offset_ + 2]) << 8) |
        std::to_integer<uint32_t>(buffer_[offset_ + 3]);

    offset_ += 4;

    return value;
}

std::string BinaryReader::read_string(size_t length) {
    check_avaiable(length);

    std::string str(reinterpret_cast<const char*>(buffer_.data() + offset_), length);
    offset_ += length;

    return str;
}

std::vector<std::byte> BinaryReader::read_bytes(size_t length) {
    check_avaiable(length);

    std::vector<std::byte> bytes(buffer_.begin() + offset_, buffer_.begin() + offset_ + length);
    offset_ += length;

    return bytes;
}

void BinaryReader::skip(size_t count) {
    check_avaiable(count);

    offset_ += count;
}

void BinaryReader::check_avaiable(size_t count) const {
    if (offset_ + count > buffer_.size()) {
        throw std::out_of_range("BinaryReader: not enough data to read");
    }
}

}
