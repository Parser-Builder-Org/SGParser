// Filename:  SGString.h
// Content:   Parser string type definition
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_STRING_H
#define INC_SGPARSER_STRING_H

#include "SGDebug.h"

#include <cstdarg>
#include <cstdio>
#include <cinttypes>
#include <string>
#include <type_traits>

namespace SGParser
{

// ***** String type definition

using CharT  = char;
using String = std::basic_string<CharT>;


// ***** String functions

// String with sprintf-like formatting and va_list argument
// Format string and string-typed arguments can be only
// a null-terminated strings of single-byte character types (UTF-8 encoded)
template <typename CompatibleChar>
inline String StringWithVAFormat(const CompatibleChar* format, va_list argList) {
    static_assert(std::is_integral_v<CompatibleChar> && sizeof(CompatibleChar) == 1u,
                  "String supports only single-byte character types, e.g. char or char8_t");

    // Determine required buffer-size:
    va_list argList2;
    va_copy(argList2, argList);
    const auto ilen = std::vsnprintf(nullptr, 0u, reinterpret_cast<const char*>(format), argList2);
    SG_ASSERT(ilen >= 0);  // if this fails, vsnprintf() discovered an error - possibly format-error
    va_end(argList2);

    if (ilen == 0)
        return String{};

    // Write it:
    String result(static_cast<size_t>(ilen) + 1u, CharT{});
    static_assert(sizeof(result.front()) == 1u, "String supports only single-byte character, e.g., char or char8_t");
    [[maybe_unused]] const auto ilen2 = std::vsnprintf(reinterpret_cast<char*>(result.data()),
                                                       result.size(),
                                                       reinterpret_cast<const char*>(format),
                                                       argList);
    SG_ASSERT(ilen2 == ilen);

    result.resize(static_cast<size_t>(ilen2));
    return result;
}


// String with sprintf-like formatting and variadic arguments
// Format string and string-typed arguments can be only
// a null-terminated strings of single-byte character types (UTF-8 encoded)
template <typename CompatibleChar>
inline String StringWithFormat(const CompatibleChar* format...) {
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
inline String StringFromNumber(Number value) noexcept {
    static_assert(std::is_arithmetic_v<Number>, "Arithmetic type is required");
    return std::to_string(value);
}


// Non-throwing conversion to arithmetic type
// Returns a default-constructed Number value in case of conversion error
template <typename Number>
inline Number StringToNumber(const String& str) noexcept {
    static_assert(std::is_arithmetic_v<Number>, "Arithmetic type is required");
    if constexpr (std::is_floating_point_v<Number>)
        return static_cast<Number>(std::strtold(str.data(), nullptr));
    else if constexpr (std::is_signed_v<Number>)
        return static_cast<Number>(std::strtoimax(str.data(), nullptr, 10));
    else
        return static_cast<Number>(std::strtoumax(str.data(), nullptr, 10));
}

} // namespace SGParser

#endif // INC_SGPARSER_STRING_H
