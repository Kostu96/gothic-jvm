#pragma once
#include <cstdint>

bool getFileSize(const char* filename, size_t& fileSize);
bool readFile(const char* filename, void* buffer, size_t bufferSize);

inline uint8_t parseU8(uint8_t*& ptr)
{
    return *ptr++;
}

inline uint16_t parseU16BigEndian(uint8_t*& ptr)
{
    uint16_t value = *ptr << 8 | *(ptr + 1);
    ptr += 2;
    return value;
}

inline uint32_t parseU32BigEndian(uint8_t*& ptr)
{
    uint32_t value = *ptr << 24 | *(ptr + 1) << 16 | *(ptr + 2) << 8 | *(ptr + 3);
    ptr += 4;
    return value;
}

inline uint64_t parseU64BigEndian(uint8_t*& ptr)
{
    uint64_t valueH = parseU32BigEndian(ptr);
    uint64_t valueL = parseU32BigEndian(ptr);
    return valueH << 32 | valueL;
}
