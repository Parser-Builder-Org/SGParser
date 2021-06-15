// Filename:  FileOutputStream.h
// Content:   Adapters for file output.
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_GENERATOR_FILE_OUTPUT_STREAM_H
#define INC_SGPARSER_GENERATOR_FILE_OUTPUT_STREAM_H

#include "SGStream.h"

#include <fstream>
#include <filesystem>

namespace SGParser
{
namespace Generator
{

// Implementation of the SGParser output stream interface that allows to use standard file stream
class FileOutputStream final : public OutputStream
{
public:
    enum class Mode 
    {
        Append,
        Truncate
    };

public:
    FileOutputStream() = default;
    
    FileOutputStream(const FileOutputStream&)                = delete;
    FileOutputStream(FileOutputStream&&) noexcept            = delete;
    FileOutputStream& operator=(const FileOutputStream&)     = delete;
    FileOutputStream& operator=(FileOutputStream&&) noexcept = delete;

    bool Open(const std::filesystem::path& fileName, Mode mode) {
        std::fstream::openmode openMode = std::fstream::out;

        switch (mode) {
            case Mode::Append:
                openMode |= std::fstream::app;
                break;
            case Mode::Truncate:
                openMode |= std::fstream::trunc;
                break;
        }

        fileStream.open(fileName.string(), openMode);
        return fileStream.is_open();
    }

    StreamSize Write(const uint8_t* pBuffer, StreamSize numBytes) override {
        if (fileStream.is_open()) {
            const auto pos = fileStream.tellp();
            fileStream.write(reinterpret_cast<const char*>(pBuffer),
                             static_cast<std::streamsize>(numBytes));
            return static_cast<StreamSize>(fileStream.tellp() - pos);
        }
        return 0;
    }

    bool Close() override {
        fileStream.close();
        return !fileStream.is_open(); 
    }

private:
    std::ofstream fileStream;
};

} // namespace Generator
} // namespace SGParser

#endif // INC_SGPARSER_GENERATOR_FILE_OUTPUT_STREAM_H
