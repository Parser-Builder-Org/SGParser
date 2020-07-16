// Filename:  LexemeInfo.h
// Content:   LexemeInfo structure
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_LEXEMEINFO_H
#define INC_SGPARSER_LEXEMEINFO_H

#include <cstdint>

namespace SGParser {

// *** LexemeInfo structure

// Information about lexeme
// This will be stored in the DFA
struct LexemeInfo final
{
    // Action
    enum ActionValue : unsigned 
    {
        ActionNone      = 0x0000'0000,
        ActionGoto      = 0x1000'0000,
        ActionPush      = 0x2000'0000,
        ActionPop       = 0x3000'0000,
        ActionMask      = 0xF000'0000,
        ActionValueMask = 0x0FFF'FFFF
    };

    // Token code, 0 for ignore
    unsigned TokenCode  = 0u;
    // Actions to perform on lexeme
    unsigned Action     = ActionNone;
};

} // namespace SGParser

#endif // INC_SGPARSER_LEXEMEINFO_H
