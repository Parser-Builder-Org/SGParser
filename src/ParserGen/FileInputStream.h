// Filename:  FileInputStream.h
// Content:   Adapters for file input.
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_GENERATOR_FILE_INPUT_STREAM_H
#define INC_SGPARSER_GENERATOR_FILE_INPUT_STREAM_H

#include "SGStream.h"

#include <fstream>
#include <filesystem>

namespace SGParser
{
namespace Generator
{

// Implementation of the SGParser input stream interface that allows to use standard file stream
class FileInputStream final : public InputStream
{
public:
    FileInputStream() = default;

    FileInputStream(const FileInputStream&)                = delete;
    FileInputStream(FileInputStream&&) noexcept            = delete;
    FileInputStream& operator=(const FileInputStream&)     = delete;
    FileInputStream& operator=(FileInputStream&&) noexcept = delete;

    bool Open(const std::filesystem::path& fileName) {
        fileStream.open(fileName.string(), std::fstream::in);
        return fileStream.is_open();
    }

    StreamSize Read(uint8_t* pBuffer, StreamSize numBytes) override {
        if (fileStream.is_open()) {
            fileStream.read(reinterpret_cast<char*>(pBuffer),
                            static_cast<std::streamsize>(numBytes));
            return static_cast<StreamSize>(fileStream.gcount());
        }
        return 0;
    }

    bool Close() override {
        fileStream.close();
        return !fileStream.is_open();
    }

private:
    std::ifstream fileStream;
};

} // namespace Generator
} // namespace SGParser

#endif // INC_SGPARSER_GENERATOR_FILE_INPUT_STREAM_H
