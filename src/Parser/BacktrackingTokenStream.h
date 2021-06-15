// Filename:  BacktrackingTokenStream.h
// Content:   BacktrackingTokenStream class header file with declarations
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_BACKTRACKINGTOKENSTREAM_H
#define INC_SGPARSER_BACKTRACKINGTOKENSTREAM_H

#include "Tokenizer.h"

#include <map>

namespace SGParser
{

// ***** Backtracking token stream

// Will read tokens from source stream, and allow to backtrack in them based on markers
template <class Token = TokenCode>
class BacktrackingTokenStream final : public TokenStream<Token> 
{
public:
    // Const for representation of an invalid token index
    static constexpr size_t InvalidIndex = size_t(-1);

public:
    // Constructors
    BacktrackingTokenStream();
    explicit BacktrackingTokenStream(TokenStream<Token>* pSourceStream,
                                     size_t rememberLength = 1u);

    // No copy/move allowed
    BacktrackingTokenStream(const BacktrackingTokenStream&)                = delete;
    BacktrackingTokenStream(BacktrackingTokenStream&&) noexcept            = delete;
    BacktrackingTokenStream& operator=(const BacktrackingTokenStream&)     = delete;
    BacktrackingTokenStream& operator=(BacktrackingTokenStream&&) noexcept = delete;

    // Destructor
    ~BacktrackingTokenStream() override;

    // Resets all buffers, and sets source stream
    void   ResetStream(TokenStream<Token>* pSourceStream, size_t rememberLength = 1u) noexcept;

    // *** Marker & backtracking management

    // Returns token index (current position from the very beginning)
    // Should be used to set markers
    size_t GetTokenIndex() const noexcept;

    // Releases all markers
    void   ResetMarkers() noexcept;

    // Sets 'tracking' mark, so we can backtrack to this position if needed
    // Return true for success, false for fail (already lost that index)
    bool   SetMarker(size_t makerIndex = InvalidIndex);

    // Frees tracking index (we may not be able to backtrack here any more)
    // Return false if no marker was defined for that index
    bool   ReleaseMarker(size_t markerIndex);

    // Returns number of tokens buffered after the marker
    size_t GetBufferedLength(size_t markerIndex) const;

    // Backtracks to a certain marker
    bool   BacktrackToMarker(size_t markerIndex, size_t streamLength = MaxSize);

    // Backtracks several items back from current position (may be limited by RememberLength)
    bool   SeekBack(size_t count) noexcept;
    
    // Backtracks to absolute position
    bool   SeekTo(size_t index) noexcept;

    // Advances to buffer end and resets stream length
    void   AdvanceToBufferEnd(size_t streamLength = MaxSize) noexcept;

    // Sets stream length (of how many characters will be reported from that point on)
    // After stream length is hit, it will automatically report EOF
    void   SetMaxStreamLength(size_t streamLength = MaxSize) noexcept;

    // *** Token stream implementation

    // Returns next token
    Token& GetNextToken(Token& token) override;

private:
    // Internal const for representation of the maximum input stream size
    static constexpr size_t MaxSize = size_t(-1);

    // Internal block caching system
    struct StreamBlock final
    {
        static constexpr size_t BufferSize = 512u;

        Token        Tokens[BufferSize];
        size_t       Index = 0u;        // Index of first token
        size_t       Count = 0u;        // Number of tokens
        StreamBlock* pNext = nullptr;
    };

    TokenStream<Token>* pSourceStream;  // Token source, if any
    StreamBlock*        pFirstBlock;    // First block
    StreamBlock*        pThisBlock;     // Current position
    size_t              ThisPos;
    size_t              RememberLength; // Number of previous tokens to remember
    size_t              LengthLeft;     // Number of tokens left to report
    size_t              Pos;            // Current position (furthest position in source stream)
    bool                SourceEOFFlag;  // Flag set, if source is at EOF

    // Tracked starting positions (mark which elements we have to remember)
    std::map<size_t, StreamBlock*> Markers;

    // *** Utility functions

    // Releases extra buffers that are no longer needed
    void ReleaseExtraBuffers() noexcept;
};


// *** BacktrackingTokenStream implementation

// Default constructor
template <class Token>
BacktrackingTokenStream<Token>::BacktrackingTokenStream() 
    : pSourceStream{nullptr},
      pFirstBlock{new StreamBlock}, // Allocate 1 stream block
      pThisBlock{pFirstBlock},
      ThisPos{0u},
      RememberLength{1u},
      LengthLeft{0u},
      Pos{0u},
      SourceEOFFlag{false} {
}

// Initialization constructor
template <class Token>
BacktrackingTokenStream<Token>::BacktrackingTokenStream(TokenStream<Token>* psourceStream,
                                                        size_t rememberLength)
    : pSourceStream{psourceStream},
      pFirstBlock{new StreamBlock}, // Allocate 1 stream block
      pThisBlock{pFirstBlock},
      ThisPos{0u},
      RememberLength{rememberLength},
      LengthLeft{psourceStream ? MaxSize : 0u},
      Pos{0u},
      SourceEOFFlag{false} {
}

// Destructor
template <class Token>
BacktrackingTokenStream<Token>::~BacktrackingTokenStream() {
    // Free all allocated blocks
    while (pFirstBlock)
        delete std::exchange(pFirstBlock, pFirstBlock->pNext);
}

// Resets all buffers, and sets source stream
template <class Token>
void BacktrackingTokenStream<Token>::ResetStream(TokenStream<Token>* psourceStream,
                                                 size_t rememberLength) noexcept {
    // Release all markers
    Markers.clear();
    // Release all but one buffer
    while (pFirstBlock->pNext)
        delete std::exchange(pFirstBlock, pFirstBlock->pNext);

    pThisBlock        = pFirstBlock;
    pThisBlock->Index = 0u;
    ThisPos           = 0u;
    // Reset variables
    pSourceStream     = psourceStream;
    RememberLength    = rememberLength;
    LengthLeft        = psourceStream ? MaxSize : 0u;
    Pos               = 0u;
    SourceEOFFlag     = false;
}

// *** Utility functions

// Releases extra buffers that are no longer needed
template <class Token>
void BacktrackingTokenStream<Token>::ReleaseExtraBuffers() noexcept {
    // Last valid index we have to keep track of
    // Specifically the smaller of:
    //   - the distance from RememberLength to Pos (or 0 if RememberLength > Pos);
    //   - if Markers has any keys, the first key in Markers.
    size_t lastIndex = Pos > RememberLength ? Pos - RememberLength : 0u;

    // Account for earliest marker
    if (!Markers.empty())
        lastIndex = std::min(lastIndex, Markers.cbegin()->first);

    // Free all blocks before lastIndex
    while (pFirstBlock->Index < lastIndex && pFirstBlock != pThisBlock &&
           pFirstBlock->pNext && pFirstBlock->pNext->Index <= lastIndex)
        delete std::exchange(pFirstBlock, pFirstBlock->pNext);
}

// *** Marker & backtracking management

// Releases all markers
template <class Token>
void BacktrackingTokenStream<Token>::ResetMarkers() noexcept {
    Markers.clear();
    ReleaseExtraBuffers();
}

// Returns token index (current position from the very beginning)
// Should be used to set markers
template <class Token>
size_t BacktrackingTokenStream<Token>::GetTokenIndex() const noexcept {
    SG_ASSERT(ThisPos < StreamBlock::BufferSize);
    return ThisPos + pThisBlock->Index;
}

// Sets 'tracking' mark, so we can backtrack to this position if needed
// Return true for success, false for fail (already lost that index)
template <class Token>
bool BacktrackingTokenStream<Token>::SetMarker(size_t markerIndex) {
    if (markerIndex < pFirstBlock->Index || markerIndex > Pos)
        return false;

    StreamBlock* pblock;

    // Find a block marker would belong to
    if (pThisBlock->Index <= markerIndex &&
        (pThisBlock->Index + pThisBlock->Count > markerIndex ||
         (pThisBlock->Index + pThisBlock->Count == markerIndex &&
          pThisBlock->Count != StreamBlock::BufferSize)))
        pblock = pThisBlock;
    else {
        pblock = pFirstBlock;
        while (pblock->pNext && pblock->pNext->Index <= markerIndex)
            pblock = pblock->pNext;
    }
    // Add marker
    Markers.insert_or_assign(markerIndex, pblock);
    return true;
}

// Frees tracking index (we may not be able to backtrack here any more)
// Return false if no marker was defined for that index
template <class Token>
bool BacktrackingTokenStream<Token>::ReleaseMarker(size_t markerIndex) {
    const auto iMap = Markers.find(markerIndex);
    // If not found, we fail
    if (iMap == Markers.end())
        return false;
    // Remove one marker of this value
    if (iMap == Markers.begin()) {
        Markers.erase(iMap);
        ReleaseExtraBuffers();
    } else
        Markers.erase(iMap);
    return true;
}

// Returns number of tokens buffered after the marker
template <class Token>
size_t BacktrackingTokenStream<Token>::GetBufferedLength(size_t markerIndex) const {
    // If not found, we fail
    if (Markers.find(markerIndex) == Markers.end())
        return 0u;
    return Pos - markerIndex;
}

// Backtracks to a certain marker
template <class Token>
bool BacktrackingTokenStream<Token>::BacktrackToMarker(size_t markerIndex, size_t streamLength) {
    const auto iMap = Markers.find(markerIndex);
    // If not found, we fail
    if (iMap == Markers.end())
        return false;

    pThisBlock = iMap->second;
    ThisPos    = markerIndex - pThisBlock->Index;
    LengthLeft = pSourceStream ? streamLength : 0u;
    return true;
}

// Backtracks several items back from current position (may be limited by RememberLength)
template <class Token>
bool BacktrackingTokenStream<Token>::SeekBack(size_t count) noexcept {
    SG_ASSERT(ThisPos < StreamBlock::BufferSize);
    return SeekTo(pThisBlock->Index + ThisPos - count);
}

// Backtracks to absolute position
template <class Token>
bool BacktrackingTokenStream<Token>::SeekTo(size_t index) noexcept {
    // if this is not in our current block
    if (!(pThisBlock->Index <= index &&
          pThisBlock->Index + pThisBlock->Count <= index &&
          pThisBlock->Count != StreamBlock::BufferSize)) {
        // Otherwise seek from beginning
        // Check bounds
        if (index < pFirstBlock->Index || index > Pos)
            return false;
        // And seek till block that contains our index
        pThisBlock = pFirstBlock;
        while (index >= pThisBlock->Index + pThisBlock->Count && pThisBlock->pNext)
            pThisBlock = pThisBlock->pNext;
    }

    // Update index
    ThisPos = index - pThisBlock->Index;
    return true;
}

// Advances to buffer end and resets stream length
template <class Token>
void BacktrackingTokenStream<Token>::AdvanceToBufferEnd(size_t streamLength) noexcept {
    // Advance to last block
    while (pThisBlock->Count == StreamBlock::BufferSize)
        pThisBlock = pThisBlock->pNext;
    // Set position, and length
    ThisPos    = pThisBlock->Count;
    LengthLeft = pSourceStream ? streamLength : 0u;
}

// Sets stream length (of how many characters will be reported from that point on)
// After stream length is hit, it will automatically report EOF
template <class Token>
void BacktrackingTokenStream<Token>::SetMaxStreamLength(size_t streamLength) noexcept {
    LengthLeft = pSourceStream ? streamLength : 0u;
}

// *** Token stream implementation

// Returns next token
template <class Token>
Token& BacktrackingTokenStream<Token>::GetNextToken(Token& token) {
    // Check if we are in the correct state, i.e.
    // there is a StreamBlock to save the next token
    SG_ASSERT(ThisPos != StreamBlock::BufferSize);

    // Zero length - return EOF Token
    if (LengthLeft == 0u) {
        token = Token{};
        return token;
    }

    // If we are behind the end (backtracked)
    // return next consecutive token
    if (ThisPos < pThisBlock->Count) {
        token = pThisBlock->Tokens[ThisPos];
        ++ThisPos;
        if (ThisPos == StreamBlock::BufferSize) {
            ThisPos    = 0u;
            pThisBlock = pThisBlock->pNext;
        }
        return token;
    }

    // Read next token from original stream
    pSourceStream->GetNextToken(token);
    pThisBlock->Tokens[ThisPos] = token;

    // If source ended earlier, just return EOF value
    if (SourceEOFFlag)
        return token;
    if (token.Code == TokenCode::TokenEOF)
        SourceEOFFlag = true;

    // Increase pointer
    ++ThisPos;
    ++pThisBlock->Count;
    // Final position in stream
    ++Pos;
    if (ThisPos == StreamBlock::BufferSize) {
        // Enforce basic exception safety
        // If new StreamBlock fails then ThisPos will stay equal
        // to StreamBlock::BufferSize and next call to GetNextToken
        // will fail (but BacktrackingTokenStream will stay in safe-to-delete state)
        const auto newBlock = new StreamBlock;
        ThisPos             = 0u;
        pThisBlock->pNext   = newBlock;
        pThisBlock          = pThisBlock->pNext;
        pThisBlock->Index   = Pos;
        // Check for freeing any extra buffers
        ReleaseExtraBuffers();
    }
    return token;
}

} // namespace SGParser

#endif // INC_SGPARSER_BACKTRACKINGTOKENSTREAM_H
