// Filename:  LexemeInfo.h
// Content:   LexemeInfo structure
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_LEXEMEINFO_H
#define INC_SGPARSER_LEXEMEINFO_H

namespace SGParser
{

// *** LexemeInfo structure

// Information about lexeme
// This will be stored in the DFA
struct LexemeInfo final
{
    // Action
    enum ActionValue : unsigned
    {
        ActionNone      = 0x0000'0000u,
        ActionGoto      = 0x1000'0000u,
        ActionPush      = 0x2000'0000u,
        ActionPop       = 0x3000'0000u,
        ActionMask      = 0xF000'0000u,
        ActionValueMask = 0x0FFF'FFFFu
    };

    // Token code, 0 for ignore
    unsigned TokenCode  = 0u;
    // Actions to perform on lexeme
    unsigned Action     = ActionNone;
};

} // namespace SGParser

#endif // INC_SGPARSER_LEXEMEINFO_H
