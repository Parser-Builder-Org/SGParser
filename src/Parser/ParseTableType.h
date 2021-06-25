// Filename:  ParseTableType.h
// Content:   ParseTableType enumeration
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_PARSETABLETYPE_H
#define INC_SGPARSER_PARSETABLETYPE_H

namespace SGParser
{

// Types of parse tables supported
enum class ParseTableType
{
    // Set initially, no table
    None,
    // LR(1) parsing table
    LR,
    // LALR(1) parsing table
    LALR,
    // Compacted LR(1) parsing table (very similar to LALR,
    // but won't generate erroneous reduce-reduce conflicts)
    CLR
};

} // namespace SGParser

#endif // INC_SGPARSER_PARSETABLETYPE_H
