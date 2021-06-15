// Filename:  Lexeme.h
// Content:   Lexeme structure
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_LEXEME_H
#define INC_SGPARSER_LEXEME_H

#include "SGString.h"
#include "LexemeInfo.h"

namespace SGParser
{

// *** Lexeme structure

// Lexeme data from file
struct Lexeme final
{
    // Name of the lexeme, or it's Token label
    // If empty, the lexeme's token is to be ignored
    String     Name;
    // Regular expressions corresponding to lexemes
    String     RegularExpression;
    // Actions
    LexemeInfo Info;
    // Action parameter expression name String, if any
    String     ActionParam;

    void SetLexeme(const String& name, const String& regExpr, unsigned tokenCode = 0u) {
        Name              = name;
        RegularExpression = regExpr;
        Info              = {tokenCode, LexemeInfo::ActionNone};
        ActionParam.clear();
    }
};

} // namespace SGParser

#endif // INC_SGPARSER_LEXEME_H
