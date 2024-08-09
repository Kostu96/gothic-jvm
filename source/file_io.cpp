#include "file_io.hpp"

#include <cstdio>

bool getFileSize(const char* filename, size_t& fileSize)
{
    FILE* file;
    fopen_s(&file, filename, "rb");
    if (!file) {
        fileSize = 0;
        return false;
    }

    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);

    fclose(file);
    return true;
}

bool readFile(const char* filename, void* buffer, size_t bufferSize)
{
    FILE* file;
    fopen_s(&file, filename, "rb");
    if (!file) {
        return false;
    }

    fread(buffer, 1, bufferSize, file);

    fclose(file);
    return true;
}
