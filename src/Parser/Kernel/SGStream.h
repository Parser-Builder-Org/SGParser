// Filename:  SGStream.h
// Content:   Header for a general-purpose stream
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_STREAM_H
#define INC_SGPARSER_STREAM_H

#include "SGString.h"

#include <cerrno>
#include <cstring>
#include <algorithm>
#include <type_traits>

namespace SGParser
{

// ***** Input stream

// Signed type for representing size of buffer to read/write
// and return number of bytes read/written
using StreamSize = std::make_signed_t<size_t>;

class InputStream
{
public:
    virtual ~InputStream() = default;

    // Blocking read, will read in the given number of bytes or less from the stream
    // Returns : -1 for error
    //           Otherwise number of bytes read,
    //           if 0 or < numBytes, no more bytes available,
    //           end of file or the other side of stream is closed
    virtual StreamSize Read(uint8_t* pBuffer, StreamSize numBytes)        = 0;
    // Closes the stream
    // This stream object can't be used again after being closed
    virtual bool       Close()                                            = 0;
};


// ***** Output stream

class OutputStream
{
public:
    virtual ~OutputStream() = default;

    // Blocking write, will write in the given number of bytes to the stream
    // Returns : -1 for error
    //           Otherwise number of bytes read
    virtual StreamSize Write(const uint8_t* pBuffer, StreamSize numBytes) = 0;
    // Closes the stream
    // One the Output stream is closed, the read on a corresponding
    // The stream object can't be used again after being closed
    virtual bool       Close()                                            = 0;
};


// ***** Text output stream

// Delegated stream for writing chars from a user-provided string buffer (non-owning)
// Instance of OutputStream class should be provided as an actual output stream object
class TextOutputStream final : public OutputStream
{
public:
    // Constructor
    explicit TextOutputStream(OutputStream& outputStream) : stream{outputStream} {}

    // Deleted constructors
    TextOutputStream()                            = delete;
    TextOutputStream(const TextOutputStream&)     = delete;
    TextOutputStream(TextOutputStream&&) noexcept = delete;

    // Outputs a 'size' number of chars from string buffer (the data can include '0' values)
    // Negative 'size' means output a string up to '0' value (suitable for
    // output an entire null-terminated string)
    StreamSize WriteText(const char* str, StreamSize size = -1) {
        if (!str)
            return 0;
        size = size < 0 ? static_cast<StreamSize>(std::strlen(str)) : size;
        return Write(reinterpret_cast<const uint8_t*>(str), size);
    }

    // Outputs a 'size' number of chars from String
    // Negative 'size' value means output an entire string
    StreamSize WriteText(const String& str, StreamSize size = -1) {
        if (static_cast<size_t>(size) > str.size())
            size = static_cast<StreamSize>(str.size());
        return Write(reinterpret_cast<const uint8_t*>(str.data()), size);
    }

    bool Close() override { return stream.Close(); }

private:
    OutputStream& stream;

    StreamSize Write(const uint8_t* pBuffer, StreamSize numBytes) override {
        return stream.Write(pBuffer, numBytes);
    }
};


// ***** MemBufferInputStream

// Reads data from user-provided buffer (non-owning)
// The data can include '0' values
class MemBufferInputStream final : public InputStream
{
public:
    // Default constructor (empty stream)
    MemBufferInputStream() noexcept = default;

    // Constructor from String
    explicit MemBufferInputStream(const String& bufferString) noexcept
        : MemBufferInputStream{bufferString.data(), bufferString.size()}
    {}

    // Constructor from the buffer of known size
    template <typename CompatibleType>
    MemBufferInputStream(const CompatibleType* buffer, size_t bufferSize) noexcept
        : buf{reinterpret_cast<const uint8_t*>(buffer)},
          size{bufferSize},
          pos{0u} {
        static_assert(sizeof(CompatibleType) == 1u,
                      "MemBufferInputStream supports only buffer with single-byte elements");
    }

    // Deleted constructors
    MemBufferInputStream(const MemBufferInputStream&)     = delete;
    MemBufferInputStream(MemBufferInputStream&&) noexcept = delete;

    // Destructor
    ~MemBufferInputStream() override { Close(); }

    // Set string as input buffer
    void SetInputString(const String& bufferString) noexcept {
        SetInputBuffer(bufferString.data(), bufferString.size());
    }

    // Set input buffer of known size
    template <typename CompatibleType>
    void SetInputBuffer(const CompatibleType* buffer, size_t bufferSize) noexcept {
        static_assert(sizeof(CompatibleType) == 1u,
                      "MemBufferInputStream supports only buffer with single-byte elements");
        buf  = reinterpret_cast<const uint8_t*>(buffer);
        size = bufferSize;
        pos  = 0u;
    }

    // *** InputStream overrides

    StreamSize Read(uint8_t* pBuffer, StreamSize numBytes) override {
        if (buf) {
            const size_t length = std::min(size - pos, static_cast<size_t>(numBytes));
            if (length > 0u) {
                std::copy(buf + pos, buf + pos + length, pBuffer);
                pos += length;
            }
            return static_cast<StreamSize>(length);
        }
        return 0;
    }

    bool Close() noexcept override {
        buf  = nullptr;
        size = 0u;
        pos  = 0u;
        return true;
    }

private:
    const uint8_t* buf  = nullptr;
    size_t         size = 0u;
    size_t         pos  = 0u;
};

} // namespace SGParser

#endif // INC_SGPARSER_STREAM_H
