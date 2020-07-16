// Filename:  TokenizerBase.h
// Content:   Tokenizer base class providing buffer support
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_TOKENIZERBASE_H
#define INC_SGPARSER_TOKENIZERBASE_H

#include "SGStream.h"

namespace SGParser {

// ***** Tokenizer's Data Buffer

// Linked list of buffers is used, to allow for arbitrary length lexemes
struct TokenizerBuffer 
{
    static constexpr size_t BufferSize  = 8192u;

    // Pointer to the first character after the buffer
    // pBufferTail has to be first element in a structure!!!
    char*            pBufferTail        = nullptr;
    // Pointer to next buffer, if any
    TokenizerBuffer* pNext              = nullptr;
    // Character data
    char             Buffer[BufferSize] = {};
};


// Tokenizer base class that implements buffer support
// This can be shared among all template tokenizer instances
class TokenizerBase 
{
public:
    // Default constructor
    TokenizerBase() = default;

    // No copy/move allowed
    TokenizerBase(const TokenizerBase&)                = delete;
    TokenizerBase(TokenizerBase&&) noexcept            = delete;
    TokenizerBase& operator=(const TokenizerBase&)     = delete;
    TokenizerBase& operator=(TokenizerBase&&) noexcept = delete;

    // Destructor
    ~TokenizerBase() { FreeAllBuffers(); }

    // *** Internal routines

    // Set input stream (1 for success)
    bool SetInputStream(InputStream* pinputStream);

    // Returns false for EOF
    // Must be called only after successful call to SetInputStream(),
    // since this requires pTailBuffer to be non-null
    bool AdvanceTail() {
        SG_ASSERT(pTailBuffer);
        ++pTail;
        return pTail >= pTailBuffer->pBufferTail ? ReloadBuffer() : true;
    }

    // Creates a new buffer and loads the block from input stream
    // Returns nullptr for EOF
    TokenizerBuffer* LoadNewBuffer(bool freeOnEmpty = true);
    // Reloads buffers & positions tail
    // Must be called only after successful call to SetInputStream(),
    // since this requires pTailBuffer to be non-null
    // Returns nullptr for EOF
    bool ReloadBuffer();
    // Free's all the buffers and resets the pointers
    void FreeAllBuffers() noexcept;

    // Move the head to the tail and free the used buffers
    // Must be called only after successful call to SetInputStream(),
    // since this requires pHeadBuffer and pTailBuffer to be non-null
    void AdjustHead() noexcept;

    // Byte reader implementations for both initial token character reading token 
    // string re-scanning. We always start out at the first byte available
    // (unless IsEOF() is true) and can call Advance() for more
    class ByteReader final
    {
    public:
        explicit ByteReader(TokenizerBase& tok) noexcept : Tok{tok} {}

        uint8_t  GetByte() const noexcept { return static_cast<uint8_t>(*Tok.pTail); }
        bool     IsEOF() const noexcept   { return Tok.pTail >= Tok.pTailBuffer->pBufferTail; }
        bool     Advance()                { return Tok.AdvanceTail(); }

    private:
        TokenizerBase& Tok;
    };

    // Get location in the buffer; can represent either head or tail
    struct BufferPos final
    {
        char*            pChar   = nullptr;
        TokenizerBuffer* pBuffer = nullptr;

        bool operator==(const BufferPos& other) const noexcept {
            return (pChar == other.pChar) && (pBuffer == other.pBuffer);
        }
    };

    BufferPos GetHeadPos() const noexcept { return {pHead, pHeadBuffer}; }
    BufferPos GetTailPos() const noexcept { return {pTail, pTailBuffer}; }

    // Readjust tail buffer; useful when we overshoot the tail
    // end of the token due to lexical scanning look-ahead
    void      SetTailPos(const BufferPos& src) noexcept {
        pTail       = src.pChar; 
        pTailBuffer = src.pBuffer;
    }

    // Read in bytes between the specified Head and Tail positions
    // Useful for obtaining strings of characters
    class BufferRangeByteReader final
    {
    public:
        BufferRangeByteReader(const BufferPos& head, const BufferPos& tail) 
            : Head{head},
              Tail{tail} {
        }

        uint8_t GetByte() const noexcept { return static_cast<uint8_t>(*Head.pChar); }
        bool    IsEOF() const noexcept   { return Head.pChar == Tail.pChar; }

        bool    Advance() noexcept {
            ++Head.pChar;
            if (Head.pChar == Head.pBuffer->pBufferTail) {
                Head.pBuffer = Head.pBuffer->pNext;
                if (Head.pBuffer == nullptr)
                    return false;
                Head.pChar = &Head.pBuffer->Buffer[0u];
            }
            return true;
        }

    private:
        BufferPos Head;
        BufferPos Tail;
    };

private:
    // Head and its buffer (points to beginning of lexeme)
    char*            pHead        = nullptr;
    TokenizerBuffer* pHeadBuffer  = nullptr;
    // Tail and its buffer (seeks forward as lexeme is being recognized)
    char*            pTail        = nullptr;
    TokenizerBuffer* pTailBuffer  = nullptr;

    // Used for optimized performance
    TokenizerBuffer* pFreeBuffer  = nullptr;

    // The input we're tokenizing, returns data in bytes
    InputStream*     pInputStream = nullptr;
};

} // namespace SGParser

#endif // INC_SGPARSER_TOKENIZERBASE_H
