// Filename:  RegExpTokenizer.h
// Content:   RegExpTokenizer class header file with declarations
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_GENERATOR_REGEXPTOKENIZER_H
#define INC_SGPARSER_GENERATOR_REGEXPTOKENIZER_H

#include "Tokenizer.h"

namespace SGParser {
namespace Generator {

// *** Simple Regular Expression Tokenizer

template <class Token>
class RegExpTokenizer final : public TokenizerImpl<Token> 
{
public:
    // *** Constructors

    RegExpTokenizer() = default;

    explicit RegExpTokenizer(InputStream* pinputStream)
        : TokenizerImpl<Token>{pinputStream} {
    }

    // *** Token Stream interface

    // Gets next token. Return Token Code / EOF Token
    Token& GetNextToken(Token& token) override {
        InputCharReader charReader{ByteReader{*this}, TailPos};

        // If we are tracking the position
        // than adjust the head values
        HeadPos = TailPos;

        // Move the head to tail
        AdjustHead();

        // If tail's passed the end, EOF
        if (charReader.IsEOF()) {
            token.CopyFromTokenizer(TokenCode::TokenEOF, *this);
            return token;
        }

        // Use a hard coded tokenizer designed for
        // tokenizing regular expression strings
        const auto ch = charReader.GetChar();
        CodeType   code;

        // If it's not an operator used for constructing regular expressions, put
        // it down as ch character constant. Otherwise, set the code equal to the
        // value of the operator's character
        if (ch != '+' && ch != '.' && ch != '|' && ch != '*' && ch != '(' &&
            ch != ')' && ch != '[' && ch != ']' && ch != '{' && ch != '}' && 
            ch != '-' && ch != '^' && ch != '?')
            code = CodeType('c') + TokenCode::TokenFirstID;
        else
            code = CodeType(ch) + TokenCode::TokenFirstID;

        // Increment the line and column values if tracking is enabled
        charReader.Advance();

        token.CopyFromTokenizer(code, *this);
        return token;
    }

 private:
    using CodeType = typename Token::CodeType;

    using typename TokenizerBase::ByteReader;
    using typename TokenizerImpl<Token>::InputCharReader;
    using TokenizerImpl<Token>::HeadPos;
    using TokenizerImpl<Token>::TailPos;
    using TokenizerImpl<Token>::AdjustHead;
};

} // namespace Generator
} // namespace SGParser

#endif // INC_SGPARSER_GENERATOR_REGEXPTOKENIZER_H
