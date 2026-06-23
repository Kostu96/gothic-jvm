#pragma once
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace util {

class BinaryReader {
public:
    explicit BinaryReader(std::span<const std::byte> buffer) :
        buffer_(buffer), offset_(0) {}

    uint8_t read_u8();
    uint16_t read_u16();
    uint32_t read_u32();

    std::string read_string(size_t length);

    std::vector<std::byte> read_bytes(size_t length);

    void skip(size_t count);
private:
    void check_avaiable(size_t count) const;

    std::span<const std::byte> buffer_;
    size_t offset_;
};

}
