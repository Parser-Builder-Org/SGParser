// Filename:  FileStream.h
// Content:   Adapters for file input/output.
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_GENERATOR_YACC_FILE_STREAM_H
#define INC_SGPARSER_GENERATOR_YACC_FILE_STREAM_H

#include "SGStream.h"

#include <fstream>
#include <string_view>

namespace SGParser {
namespace Generator {
namespace Yacc {

// Implementation of the SGParser input stream interface that allows to use standard file stream
class FileInputStream final : public InputStream 
{
public:
    FileInputStream() = default;
    
    FileInputStream(const FileInputStream&)                = delete;
    FileInputStream(FileInputStream&&) noexcept            = delete;
    FileInputStream& operator=(const FileInputStream&)     = delete;
    FileInputStream& operator=(FileInputStream&&) noexcept = delete;

    bool Open(std::string_view fileName) {
        fileStream.open(fileName.data(), std::fstream::in);
        return fileStream.is_open();
    }

    StreamSize Read(uint8_t* pbuffer, StreamSize numBytes) override {
        if (fileStream.is_open()) {
            fileStream.read(reinterpret_cast<char*>(pbuffer),
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

    bool Open(std::string_view fileName, Mode mode) {
        std::fstream::openmode openMode = std::fstream::out;

        switch (mode) {
            case Mode::Append:
                openMode |= std::fstream::app;
                break;
            case Mode::Truncate:
                openMode |= std::fstream::trunc;
                break;
        }

        fileStream.open(fileName.data(), openMode);
        return fileStream.is_open();
    }

    StreamSize Write(const uint8_t* pbuffer, StreamSize numBytes) override {
        if (fileStream.is_open()) {
            const auto pos = fileStream.tellp();
            fileStream.write(reinterpret_cast<const char*>(pbuffer),
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

} // namespace Yacc
} // namespace Generator
} // namespace SGParser

#endif // INC_SGPARSER_GENERATOR_YACC_FILE_STREAM_H
