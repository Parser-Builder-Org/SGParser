// Filename:  Tokenizer.h
// Content:   Tokenizer class header file with declarations
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_TOKENIZER_H
#define INC_SGPARSER_TOKENIZER_H

#include "TokenizerBase.h"

#include <map>
#include <vector>

namespace SGParser
{

// ***** Token and TokenStream

// Base token class, all tokens must be derived from this class
struct TokenCode
{
    using CodeType = unsigned;

    // Reserved token codes
    enum TCode : CodeType
    {
        TokenError,  // Error token, in case an error occurred
        TokenEOF,    // End of file (input) token
        TokenFirstID // First user-assigned token id
    };

    CodeType Code;   // Code value

    // *** Token interface functions

    // Default EOF constructor must always be supported
    TokenCode(CodeType code = TokenEOF) : Code{code} {}
};


// TokenStream is a one-way input stream
// It is impossible to backup or re-read tokens from steam
// The stream will return EOF if there are no more tokens left
template <class Token = TokenCode>
class TokenStream
{
public:
    virtual ~TokenStream() = default;

    // Fills in the read in token the passed token reference
    // is returned back. Users should test token code for EOF
    virtual Token& GetNextToken(Token& token) = 0;
};


// Line / Offset position tracker in the stream
struct LineOffsetPosTracker final
{
    // Line in file and offset in line
    size_t Line   = 0u;
    size_t Offset = 0u;

    void AdvanceByte() noexcept {}

    void AdvanceChar(unsigned ch) noexcept {
        if (ch == '\n') {
            ++Line;
            Offset = 0u;
        } else if (ch == '\t')
            Offset += 4u;
        else
            ++Offset;
    }

    void Clear() noexcept {
        Line   = 0u;
        Offset = 0u;
    }
};


// A no-op position tracker that incurs no overhead
struct NullPosTracker final
{
    void AdvanceByte() noexcept         {}
    void AdvanceChar(unsigned) noexcept {}
    void Clear() noexcept               {}
};


// ***** Token Character Reader

// Character reader class used to read both the input
// characters and for re-scanning the final token string
// characters. This class can be substituted to allow
// for Unicode / UTF8 character decoding
template <class ByteReader, class PosTracker>
class TokenCharReaderBase final
{
public:
    TokenCharReaderBase(const ByteReader& reader, PosTracker& pos)
        : Reader{reader},
          Pos{pos} {
        Character = reader.GetByte();
    }

    bool Advance() {
        Pos.AdvanceChar(Character);
        Pos.AdvanceByte();
        const bool hasAdvanced = Reader.Advance();
        Character              = hasAdvanced ? Reader.GetByte() : 0u;
        return hasAdvanced;
    }

    bool     IsEOF() const noexcept   { return Reader.IsEOF(); }
    unsigned GetChar() const noexcept { return Character; }

private:
    ByteReader  Reader;
    PosTracker& Pos;
    unsigned    Character;
};


// ***** Tokenizer

// Converts StreamInput into Token stream
template <class Token>
class TokenizerImpl : public TokenStream<Token>, public TokenizerBase
{
public:
    using PosTracker      = typename Token::PosTracker;
    using InputCharReader = typename Token::InputCharReader;
    using TokenCharReader = typename Token::TokenCharReader;

    // *** Constructors

    TokenizerImpl() = default;
    explicit TokenizerImpl(InputStream* pinputStream) { SetInputStream(pinputStream); }

    // Set input stream (true for success)
    bool SetInputStream(InputStream* pinputStream) {
        // Reset the tracking data.
        TailPos.Clear();
        HeadPos.Clear();
        return TokenizerBase::SetInputStream(pinputStream);
    }

    // *** Query Token Data (returns information about current token)

    // Return a character reader for the last token
    TokenCharReader GetTokenCharReader() const {
        return TokenCharReader{BufferRangeByteReader(GetHeadPos(), GetTailPos()), NullPos};
    }
    // Get first character from the token, if this all we need
    unsigned GetTokenChar() const {
        TokenCharReader creader{BufferRangeByteReader(GetHeadPos(), GetTailPos()), NullPos};
        return creader.GetChar();
    }

    // Returns position in file
    PosTracker GetTokenPos() const noexcept { return HeadPos; }

protected:
    // Track the line and column
    PosTracker HeadPos;
    PosTracker TailPos;

    // Null position, used for token string scanning
    mutable NullPosTracker NullPos;
};


// ***** Generic token

// Simple token containing the code, string and position
struct GenericToken final : TokenCode
{
    using PosTracker      = LineOffsetPosTracker;
    using InputCharReader = TokenCharReaderBase<TokenizerBase::ByteReader, PosTracker>;
    using TokenCharReader = TokenCharReaderBase<TokenizerBase::BufferRangeByteReader, NullPosTracker>;
    using Tokenizer       = TokenizerImpl<GenericToken>;

    String Str;
    size_t Line   = 0u;
    size_t Offset = 0u;

    // Read-in from tokenizer function
    void CopyFromTokenizer(CodeType code, const Tokenizer& tokenizer) {
        Code            = code;
        const auto& pos = tokenizer.GetTokenPos();
        Line            = pos.Line;
        Offset          = pos.Offset;

        // Copy the token string
        auto creader    = tokenizer.GetTokenCharReader();

        Str.clear();
        while (!creader.IsEOF()) {
            Str += CharT(creader.GetChar());
            creader.Advance();
        }
    }
};

} // namespace SGParser

#endif // INC_SGPARSER_TOKENIZER_H
