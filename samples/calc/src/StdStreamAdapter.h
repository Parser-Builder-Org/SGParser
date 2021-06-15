// Filename:  StdStreamAdapter.h
// Content:   Standard stream adapter definition
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_SAMPLE_CALC_STD_STREAM_ADAPTER_H
#define INC_SGPARSER_SAMPLE_CALC_STD_STREAM_ADAPTER_H

#include "SGStream.h"

#include <istream>

namespace Calc 
{

using namespace SGParser;

// Implementation of the parser input stream interface that allows to use standard input stream
class StdStreamAdapter final : public InputStream 
{
public:
    explicit StdStreamAdapter(std::istream& stream) : stream{stream} {}

    StreamSize Read(uint8_t* pbuffer, StreamSize numBytes) override {
        stream.read(reinterpret_cast<char*>(pbuffer), static_cast<std::streamsize>(numBytes));
        return static_cast<StreamSize>(stream.gcount());
    }

    // Does nothing because we don't own the stream
    bool Close() override { return false; }

private:
    std::istream& stream;
};

} // namespace Calc

#endif // INC_SGPARSER_SAMPLE_CALC_STD_STREAM_ADAPTER_H
