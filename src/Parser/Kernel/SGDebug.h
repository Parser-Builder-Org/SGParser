// Filename:  SGDebug.h
// Content:   General purpose debugging support
// Provided AS IS under MIT License; see LICENSE file in root folder.
//
// SG Debug functionality is enabled if and only if SG_BUILD_DEBUG is defined.

#ifndef INC_SGPARSER_DEBUG_H
#define INC_SGPARSER_DEBUG_H

#ifdef SG_BUILD_DEBUG

#include <cassert>
#include <cstdio>

// Simple assert
#define SG_ASSERT(p)                  assert(p)

// Conditional warning with "SG Warning: " prefix
#define SG_DEBUG_WARNING(cond, str)   { if (cond) { std::printf("SG Warning: %s", str); } }

// Conditional error with "SG Error: " prefix
#define SG_DEBUG_ERROR(cond, str)     { if (cond) { std::printf("SG Error: %s", str); } }

// Conditional message with no prefix
#define SG_DEBUG_MESSAGE(cond, str)   { if (cond) { std::printf("%s", str); } }

#else // SG_BUILD_DEBUG

// If not in debug build, macros do nothing
#define SG_ASSERT(p)
#define SG_DEBUG_WARNING(cond, str)
#define SG_DEBUG_ERROR(cond, str)
#define SG_DEBUG_MESSAGE(cond, str)

#endif // SG_BUILD_DEBUG

#endif // INC_SGPARSER_DEBUG_H
