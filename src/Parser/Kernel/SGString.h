// Filename:  SGString.h
// Content:   Parser string type definition
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_STRING_H
#define INC_SGPARSER_STRING_H

#include "SGDebug.h"

#include <cstdarg>
#include <cstdio>
#include <string>
#include <charconv>
#include <type_traits>

namespace SGParser {

// ***** String type definition

using CharT  = char;
using String = std::basic_string<CharT>;


// ***** String functions

// String with sprintf-like formatting and va_list argument
// Format string and string-typed arguments can be only
// a null-terminated strings of single-byte character types (UTF-8 encoded)
template <typename CompatibleChar>
String StringWithVAFormat(const CompatibleChar* format, va_list argList) {
    static_assert(std::is_integral_v<CompatibleChar> && sizeof(CompatibleChar) == 1u,
                  "String supports only single-byte character types, e.g. char or char8_t");

    va_list argListCopy;
    va_copy(argListCopy, argList);

    // Small buffer size (value should be enough for typical conversions)
    static constexpr size_t smallBufferSize = 64u;
    String result(smallBufferSize, CharT{});

    // Try to format string using a small buffer
    const auto len = std::vsnprintf(reinterpret_cast<char*>(result.data()), smallBufferSize,
                                    reinterpret_cast<const char*>(format), argList);
    SG_ASSERT(len >= 0);
    const auto length = static_cast<size_t>(len);

    // If not enough chars in the buffer, try to format again with big-enough buffer
    if (length >= smallBufferSize) {
        result.resize(length + 1u);
        [[maybe_unused]] const auto len2 = std::vsnprintf(reinterpret_cast<char*>(result.data()),
                                                          length + 1u,
                                                          reinterpret_cast<const char*>(format),
                                                          argListCopy);
        SG_ASSERT(len2 == len);
    }

    va_end(argListCopy);
    result.resize(length);

    return result;
}


// String with sprintf-like formatting and variadic arguments
// Format string and string-typed arguments can be only
// a null-terminated strings of single-byte character types (UTF-8 encoded)
template <typename CompatibleChar>
String StringWithFormat(const CompatibleChar* format...) {
    static_assert(std::is_integral_v<CompatibleChar> && sizeof(CompatibleChar) == 1u,
                  "String supports only single-byte character types, e.g. char or char8_t");

    va_list argList;
    va_start(argList, format);
    const auto result = StringWithVAFormat(format, argList);
    va_end(argList);

    return result;
}


// Non-throwing conversion from arithmetic type
// Returns an empty string in case of conversion error
template<typename Number>
String StringFromNumber(Number value) noexcept {
    static_assert(std::is_arithmetic_v<Number>, "Arithmetic type is required");

    // Should be enough for representation of any arithmetic type
    static constexpr size_t strSize = 24u;
    char str[strSize];

    if (const auto [ptr, ec] = std::to_chars(str, str + strSize, value); ec == std::errc{})
        return String{reinterpret_cast<const CharT*>(str), reinterpret_cast<const CharT*>(ptr)};
    SG_ASSERT(false);
    return String{};
}


// Non-throwing conversion to arithmetic type
// Returns a default-constructed Number value in case of conversion error
template <typename Number>
Number StringToNumber(const String& str) noexcept {
    static_assert(std::is_arithmetic_v<Number>, "Arithmetic type is required");

    Number result;
    const auto from = reinterpret_cast<const char*>(str.data());
    const auto to   = reinterpret_cast<const char*>(str.data() + str.size());

    if (const auto [_, ec] = std::from_chars(from, to, result); ec == std::errc{})
        return result;
    SG_ASSERT(false);
    return Number{};
}

} // namespace SGParser

#endif // INC_SGPARSER_STRING_H
