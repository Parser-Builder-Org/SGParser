// Filename:  Parser.h
// Content:   Parser class header file with declarations
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_PARSER_H
#define INC_SGPARSER_PARSER_H

#include "DFATokenizer.h"
#include "BacktrackingTokenStream.h"
#include "ParseTable.h"
#include "ProductionMask.h"

#include <type_traits>
#include <algorithm>

namespace SGParser
{

// ***** Parse Stack Element

// StackElements are stored on parse stack
// It is possible to define any stack entry, provided that
// it is derived from ParseStackElement

// Stack Element base class
template <class Token = TokenCode>
struct ParseStackElement
{
    // Type of token, for parser to refer to
    using TokenType = Token;

    // Parser State
    unsigned State          = ParseTable::InvalidState;
    // Starting Terminal marker (index) for this state, if not InvalidIndex
    size_t   TerminalMarker = BacktrackingTokenStream<TokenType>::InvalidIndex;

protected:
    // This class-hierarchy uses a zero-overhead form of non-virtual pseudo-overriding
    // Think of these as if they are virtual, but with some added constraints/disciplines:
    //   - your derived class will need public versions of all 3 of these functions
    //   - if you want a "do nothing" behavior, either redefine them as {} or use `using`,
    //     e.g., `using ParseStackElement::Cleanup;` in the public part of your derived class
    // They aren't public in the base class because they aren't virtual:
    //   - making them protected prevents them from being called via a base-class ref/ptr
    //   - see https://isocpp.org/wiki/faq/strange-inheritance#redefining-nonvirtuals

    // Redefine this function to store token data
    // For error, the token data will be for first valid token after the error
    void ShiftToken([[maybe_unused]] TokenType& tok,
                    [[maybe_unused]] TokenStream<TokenType>& stream) {}

    // This gets called (after ShiftToken) to allow for access to skipped error tokens
    // Token Buffer stream constrains error tokens skipped
    void SetErrorData([[maybe_unused]] TokenType& tok,
                      [[maybe_unused]] TokenStream<TokenType>& stream) {}

    // Redefine this function to delete the appropriate data on stack error
    void Cleanup() {}
};


// Generic parse stack entry
struct ParseStackGenericElement final : public ParseStackElement<GenericToken>
{
    // User-defined data
    String Str;
    size_t Line   = 0u;
    size_t Offset = 0u;

    using ParseStackElement::SetErrorData;
    using ParseStackElement::Cleanup;

    // Redefined function to store token data
    void ShiftToken(TokenType& tok, [[maybe_unused]] TokenStream<TokenType>& stream) {
        Str    = tok.Str;
        Line   = tok.Line;
        Offset = tok.Offset;
    }
};


// ***** Parse Callback

// Forward declaration
template <class> class Parse;

template <class StackElement>
class ParseHandler
{
public:
    virtual ~ParseHandler() = default;
    virtual bool Reduce(Parse<StackElement>& parse, unsigned productionID) = 0;
};


// ***** Parse Class

// Parse class parses the user input by obtaining tokens from Tokenizer
// and reporting Reduce actions as productions are reduced.
// StackElement must derive from ParseStackElement
template <class StackElement>
class Parse final
{
public:
    // Token type, taken from stack element template
    using TokenType = typename StackElement::TokenType;

    // Type for indexing the stack elements
    using IndexType = std::make_signed_t<size_t>;

public:
    // *** Constructors & destructor

    // Create parser, and set its parse table
    explicit Parse(const ParseTable* ptable = nullptr, size_t stackSize = DefaultStackSize) {
        Create(ptable, stackSize);
    }

    // Create & initialize parser
    Parse(const ParseTable* ptable, TokenStream<TokenType>* ptokenStream,
          size_t stackSize = DefaultStackSize) {
        Create(ptable, ptokenStream, stackSize);
    }

    // No copy/move allowed
    Parse(const Parse&)                = delete;
    Parse(Parse&&) noexcept            = delete;
    Parse& operator=(const Parse&)     = delete;
    Parse& operator=(Parse&&) noexcept = delete;

    // Destructor
    ~Parse() {
        Destroy();
    }

    // Create and set the parse table (no tokenizer)
    bool Create(const ParseTable* ptable, size_t stackSize = DefaultStackSize);
    // Create and initialize the parser
    bool Create(const ParseTable* ptable, TokenStream<TokenType>* ptokenStream,
                size_t stackSize = DefaultStackSize);
    // Destroy the parser data
    void Destroy();

    // *** Utility functions

    // Set/get parse table
    // Caller retains ownership and responsibility to delete
    // Can be nullptr
    void              SetParseTable(const ParseTable* pparseTable);
    const ParseTable* GetParseTable() const noexcept { return pParseTable; }

    // Change/Set token stream
    // Caller retains ownership and responsibility to delete
    // Can be nullptr
    void                    SetTokenStream(TokenStream<TokenType>* ptokenStream);
    // Return the internal tokenizer
    TokenStream<TokenType>* GetTokenStream() noexcept { return pTokenizer; }

    // Resets parser (flushes stack)
    void ResetParse();
    // Cleans up all the elements in the parse stack
    // calls ParseStackElement::Destroy for every element
    void CleanupParseStack(size_t tillPos = 0u);

    // Return true if ready to parse (both tokenizer & parse table are set & valid)
    bool IsValid() const noexcept { return TopState != InvalidState; }

    // *** Parsing

    // Sets starting production, should be called before parsing
    bool SetStartingProduction(unsigned nonTerminal);

    // Returns either accept or error
    bool DoParse(ParseHandler<StackElement>& parseHandler);

    // Can be called on reduce to change reduce nonterminal
    // Should only be done for user-controlled reductions
    // In "A | B -> 'a';" you can force a reduce to B instead of A (default)
    // Return false if failed (change not allowed for this state/nonterminal)
    bool SetReduceNonterminal(unsigned nonTerminal) noexcept;

    // *** Stack access

    size_t GetStackPosition() const noexcept  { return StackPosition; }

    // One greater than max allowed index in (*this)[n]
    // Result varies during parsing (since StackPosition varies).
    IndexType GetMaxAllowedIndex() const noexcept {
        return IndexType(StackSize - StackPosition);
    }

    // On reduce, this can be used to access production directly
    // On shift, points to the value just shifted
    // Negative indexes are allowed, but you should be careful
    StackElement&       operator[](IndexType index) {
        SG_ASSERT(-index <= IndexType(StackPosition));
        SG_ASSERT(index < GetMaxAllowedIndex());
        return pStack[StackPosition + index];
    }

    const StackElement& operator[](IndexType index) const {
        SG_ASSERT(-index <= IndexType(StackPosition));
        SG_ASSERT(index < GetMaxAllowedIndex());
        return pStack[StackPosition + index];
    }

    // One greater than max allowed index in (*this)[n]
    // Result varies during parsing (since StackPosition varies)
    size_t size() const noexcept { return StackSize - StackPosition; }

    // *** Debugging functions

    // Displays stack in form of text
    void             PrintStack(String& str) const;

    const TokenType& GetLastToken() const noexcept        { return Token; }
    int              GetLastErrorState() const noexcept   { return LastErrorState; }
    const String&    GetErrorStackString() const noexcept { return ErrorStackStr; }

private:
    // Default stack size
    static constexpr size_t   DefaultStackSize = 2048u;
    // Min stack size
    static constexpr size_t   MinStackSize     = 128u;

    // Invalid marker const
    static constexpr size_t   InvalidIndex     = BacktrackingTokenStream<TokenType>::InvalidIndex;
    // Ivalid state const
    static constexpr unsigned InvalidState     = ParseTable::InvalidState;

    // *** Parse Tokenizer & Table

    // Parse Table to be used (non-owning pointer to a single object)
    const ParseTable*       pParseTable = nullptr;
    // Tokenizer being used (non-owning pointer to a single object)
    TokenStream<TokenType>* pTokenizer  = nullptr;
    // Backtracking stream
    BacktrackingTokenStream<TokenType> Stream;

    // *** Stack

    // Current stack pointer (starts at 0)
    size_t        StackPosition  = 0u;
    // Parser Stack (don't save in ExtractParserState)
    size_t        StackSize      = 0u;
    StackElement* pStack         = nullptr;

    // *** Runtime variables

    // Value to store on top of stack on next ParseStep callback
    // Note: TopState == InvalidState means the parser is not initialized properly
    unsigned      TopState       = InvalidState;
    // Set if next token should be obtained
    bool          NextTokenFlag  = false;
    // Previous position of error (stored so that we can see if the input was advanced)
    size_t        PrevTokenIndex = InvalidIndex;
    // Token code & data being processed
    TokenType     Token;
    // Left value, for reduce
    unsigned      ReduceLeft     = 0u;
    // Set of stack positions for valid tokens, used in error recovery
    size_t*       pValidTokenStackPositions  = nullptr;
    // Error Marker
    size_t        ErrorMarker    = InvalidIndex;
    // Last error state, for debug reporting
    unsigned      LastErrorState = InvalidState;
    String        ErrorStackStr;

    // *** Utility functions

    TokenType& GetNextToken(TokenType& token) { return Stream.GetNextToken(token); }
    bool       AdvancedInput() const noexcept { return Stream.GetTokenIndex() > PrevTokenIndex; }
};

// *** Initialization

// Create and set the parse table (no tokenizer)
template <class StackElement>
bool Parse<StackElement>::Create(const ParseTable* ptable, size_t stackSize) {
    // Make sure the parsing process is not started
    // IsValid() is reversed in this context: `this` is valid for Create
    // only if IsValid() is false
    if (IsValid())
        return false;

    // Allocate stack
    // Basic exception safety is provided
    const auto newStackSize = std::max(MinStackSize, stackSize);
    auto       newStack     = new StackElement[newStackSize];

    // From this point we can (safely) initialize the actual data

    delete[] std::exchange(pValidTokenStackPositions, nullptr);

    pParseTable = ptable;
    pTokenizer  = nullptr;
    TopState    = InvalidState;
    ErrorMarker = InvalidIndex;

    // If new stack allocation is successful then modify the existing stack
    delete[] std::exchange(pStack, newStack);
    StackSize     = newStackSize;
    StackPosition = 0u;

    return true;
}

// Create and initialize the parser
template <class StackElement>
bool Parse<StackElement>::Create(const ParseTable* ptable, TokenStream<TokenType>* ptokenStream,
                                 size_t stackSize) {
    // Create and set the parse table (no tokenizer)
    if (!Create(ptable, stackSize))
        return false;

    // And set tokenizer
    SetTokenStream(ptokenStream);

    return true;
}

// Destroy the parser data
template <class StackElement>
void Parse<StackElement>::Destroy() {
    // Delete the parse stack
    CleanupParseStack();

    delete[] std::exchange(pStack, nullptr);
    delete[] std::exchange(pValidTokenStackPositions, nullptr);

    TopState = InvalidState;
}

// Change/set parse table
// Caller retains ownership and responsibility to delete
// Can be nullptr
template <class StackElement>
void Parse<StackElement>::SetParseTable(const ParseTable* pparseTable) {
    pParseTable = pparseTable;
    ResetParse();
}

// Change/Set Tokenizer
// Caller retains ownership and responsibility to delete
// Can be nullptr
template <class StackElement>
void Parse<StackElement>::SetTokenStream(TokenStream<TokenType>* ptokenStream) {
    pTokenizer = ptokenStream;
    ResetParse();
}

// Resets parser (flushes stack)
template <class StackElement>
void Parse<StackElement>::ResetParse() {
    // Token set might have been a different size so it must be reallocated
    // Basic exception safety is provided
    size_t* newValidTokenSet = pParseTable && pParseTable->IsValid()
                                   ? new size_t[pParseTable->GetTerminalCount()]
                                   : nullptr;

    // From this point we can (safely) change the existing data

    // Delete the parse stack
    CleanupParseStack();
    Stream.ResetStream(pTokenizer);

    // Reset the data
    StackPosition  = 0u;
    PrevTokenIndex = Stream.GetTokenIndex();

    // If the parse table and tokenizer are valid then reinitialize the data
    if (pParseTable && pTokenizer && pParseTable->IsValid()) {
        // Set the top and stack state to the initial parse table state
        TopState         = pParseTable->GetInitialState();
        pStack[0u].State = TopState;
        SG_ASSERT(TopState != InvalidState);

        // If the state is recording then initialize the terminal marker
        if (pParseTable->StateInfos[TopState].Record)
            Stream.SetMarker(pStack[0u].TerminalMarker = Stream.GetTokenIndex());
        else
            pStack[0u].TerminalMarker = InvalidIndex;

        delete[] std::exchange(pValidTokenStackPositions, newValidTokenSet);
    }
    // Otherwise, set them to empty
    else {
        TopState         = InvalidState;
        pStack[0u].State = InvalidState;
    }

    NextTokenFlag = true;
}

// Delete parse stack
template <class StackElement>
void Parse<StackElement>::CleanupParseStack(size_t tillPos) {
    // Call the destroy function for all stack elements (except 0 - special element)
    for (auto i = StackPosition; i > tillPos; --i) {
        if (pStack[i].TerminalMarker != InvalidIndex)
            Stream.ReleaseMarker(pStack[i].TerminalMarker);
        pStack[i].Cleanup();
    }
    // And reset position to last 'valid' element - usually 0
    StackPosition = tillPos;
}

// *** Parsing Code

// Sets starting production, should be called before parsing
template <class StackElement>
bool Parse<StackElement>::SetStartingProduction(unsigned nonTerminal) {
    // Check to make sure the parser is valid
    if (StackPosition != 0u || pStack[0u].State == InvalidState || TopState == InvalidState)
        return false;
    // Get the start state of the nonterminal
    const auto state = pParseTable->GetStartState(nonTerminal);
    if (state == InvalidState)
        return false;
    // Adjust the stack state and top state
    pStack[0u].State = state;
    TopState         = state;
    SG_ASSERT(TopState != InvalidState);
    // If the state is recording then set the terminal market to the current token index
    if (pParseTable->StateInfos[state].Record) {
        pStack[0u].TerminalMarker = Stream.GetTokenIndex();
        Stream.SetMarker(pStack[0u].TerminalMarker);
    } else
        pStack[0u].TerminalMarker = InvalidIndex;
    return true;
}

// Callback parser implementation
template <class StackElement>
bool Parse<StackElement>::DoParse(ParseHandler<StackElement>& parseHandler) {
    unsigned errorCode;

    // Continue to parse until we encounter an error or accept
    while (true) {
        // Check the top state to make sure it is valid
        if (TopState == InvalidState)
            return false;

        // Store result from the previous step
        pStack[StackPosition].State = TopState;

        // Get the token code if needed
        if (NextTokenFlag)
            GetNextToken(Token);

    try_next_action:
        // Keep shifting as long as 'Shift' action is selected
        auto actionEntry = pParseTable->GetAction(pStack[StackPosition].State, Token.Code);
        while (actionEntry & ParseTable::ShiftMask) {
            ++StackPosition;
            SG_ASSERT(StackPosition < StackSize);
            pStack[StackPosition].State = actionEntry & ParseTable::ExtractMask;

            // User callback (to get at token data)
            pStack[StackPosition].ShiftToken(Token, Stream);
            if (pParseTable->Terminals[Token.Code].ErrorTerminal) {
                const auto marker = Token.Code == TokenCode::TokenError
                                        ? ErrorMarker
                                        : pStack[StackPosition - 1u].TerminalMarker;
                const auto offset = Stream.GetTokenIndex();
                Stream.BacktrackToMarker(marker, Stream.GetBufferedLength(marker));
                pStack[StackPosition].SetErrorData(Token, Stream);
                // If it's a backtracking error, backtrack & retry the whole thing
                if (pParseTable->StateInfos[pStack[StackPosition].State].BacktrackOnError)
                    Stream.BacktrackToMarker(pStack[StackPosition - 1u].TerminalMarker);
                else
                    Stream.SeekTo(offset);
                Stream.SetMaxStreamLength();
            }

            // Start recording if needed
            if (pParseTable->StateInfos[pStack[StackPosition].State].Record)
                Stream.SetMarker(pStack[StackPosition].TerminalMarker = Stream.GetTokenIndex());
            else
                pStack[StackPosition].TerminalMarker = InvalidIndex;

            // Get next token
            GetNextToken(Token);
            // And get next action
            actionEntry = pParseTable->GetAction(pStack[StackPosition].State, Token.Code);
        }

        // Do Reduce
        if (actionEntry & ParseTable::ReduceMask) {
            // See if we have accepted
            if (actionEntry == ParseTable::AcceptValue)
                return true;

            // On reduce, this is the production Id
            const auto ReducedProd = actionEntry & ParseTable::ExtractMask;
            const auto rprod       = pParseTable->GetReduceProduction(ReducedProd);

            // Release all markers
            for (size_t i = 0u; i < size_t(rprod.Length); ++i)
                if (pStack[StackPosition - i].TerminalMarker != InvalidIndex)
                    Stream.ReleaseMarker(pStack[StackPosition - i].TerminalMarker);

            // Pop the production (size-1), (point to the top element
            // so it can be accessed with [])
            StackPosition = StackPosition + 1u - size_t(rprod.Length);
            // Get next state (consult goto)
            TopState      = pParseTable->GetLeftReduceState(pStack[StackPosition - 1u].State,
                                                            rprod.Left);
            if (TopState == InvalidState) {
                // Cleanup the stack, including [StackPosition].
                for (size_t i = size_t(rprod.Length + 1u); i > 0u; --i)
                    pStack[StackPosition + i - 1u].Cleanup();
                // Revert to previous stack position.
                SG_ASSERT(StackPosition > 0u);
                --StackPosition;
                // Set Token.Code to '%error', which will allow try_next_action to process
                // appropriately, with either shift or reduce.
                Token.Code     = TokenCode::TokenError;
                // Backtrack by one token so that the first valid token can be re-consumed
                // properly following an error.
                PrevTokenIndex = Stream.GetTokenIndex();
                Stream.SeekBack(1u);
                goto try_next_action;
            }
            NextTokenFlag = false;
            ReduceLeft    = rprod.Left;

            // If the Stack Position advanced we must make sure to set the terminal marker
            if (rprod.Length == 0u) {
                // Start recording if needed
                if (pParseTable->StateInfos[TopState].Record)
                    Stream.SetMarker(pStack[StackPosition].TerminalMarker =
                        Stream.GetTokenIndex());
                else
                    pStack[StackPosition].TerminalMarker = InvalidIndex;
            }

            // If the reduce function fails than return an error
            if (!rprod.NotReported && !parseHandler.Reduce(*this, ReducedProd))
                goto step_error;

            // Cleanup the stack after [StackPosition].
            // Leave the element at stack position unchanged since it holds the Reduce result.
            for (size_t i = size_t(rprod.Length); i > 0u; --i)
                pStack[StackPosition + i].Cleanup();

            // See if this production has to throw a named error
            if (rprod.ErrorTerminalFlag) {
                errorCode = ReducedProd | (ReduceLeft << 16u);
                if (const auto it = pParseTable->ProductionErrorTerminals.find(errorCode);
                    it != pParseTable->ProductionErrorTerminals.end()) {
                    errorCode = it->second;
                    errorCode &= ProductionMask::TerminalValue;
                    goto handle_error;
                }
            }
            continue;
        }

        // An Error has occurred, deal with it
        errorCode = TokenCode::TokenError;

    handle_error:
        // Note that if we haven't advanced input, it's the same error as before
        if (AdvancedInput() || Stream.GetBufferedLength(ErrorMarker) == 0u) {
            if (ErrorMarker != InvalidIndex)
                Stream.ReleaseMarker(ErrorMarker);
            ErrorMarker = Stream.GetTokenIndex() - 1u;
            Stream.SetMarker(ErrorMarker);
        }

        // Calculate valid token set
        bool       errorProdFound    = false;
        bool       nextActionValid   = false;
        size_t     nextStackPosition = 0u;
        const auto maxToken          = pParseTable->GetTerminalCount();
        std::fill(pValidTokenStackPositions, pValidTokenStackPositions + maxToken, InvalidIndex);

        // Search stack until a state with action on 'error' is found
        for (size_t i = 0u; i <= StackPosition; ++i) {
            const auto sp = StackPosition - i;
            actionEntry   = pParseTable->GetAction(pStack[sp].State, errorCode);
            if (actionEntry & (ParseTable::ShiftMask | ParseTable::ReduceMask)) {
                // If the action is reduce, we have to try reducing until we
                // finally shift the error token
                // Once we shift, we can look for the next valid token
                auto pos            = sp;
                auto actionVal      = actionEntry & ParseTable::ExtractMask;
                bool needNextAction = true;

                while (actionEntry & ParseTable::ReduceMask) {
                    // Check special case (Accept)
                    const auto length = pParseTable->GetReduceActionPopSize(actionVal);
                    if (length == 0u) {
                        needNextAction = false;
                        break;
                    }
                    pos -= length - 1u;
                    SG_ASSERT(pos > 0u && pos <= StackSize);
                    // Check special case (reduce state for action is invalid)
                    const auto state = pParseTable->GetReduceState(pStack[pos - 1u].State, actionVal);
                    if (state == InvalidState) {
                        needNextAction = false;
                        break;
                    }
                    actionEntry = pParseTable->GetAction(state, errorCode);
                    actionVal   = actionEntry & ParseTable::ExtractMask;
                }

                // We should be shifting next, so actionVal is a state that'll be
                // on stack once we shift error
                errorProdFound = true;

                if (needNextAction) {
                    const auto nextActionAfterError = pParseTable->GetAction(actionVal, Token.Code);
                    if ((nextActionAfterError & (ParseTable::ShiftMask | ParseTable::ReduceMask)) != 0) {
                        nextActionValid   = true;
                        nextStackPosition = sp;
                        break;
                    }
                }

                // Go through all symbols, and collect allowed tokens
                for (unsigned j = 0u; j < unsigned(maxToken); ++j)
                    if (pValidTokenStackPositions[j] == InvalidIndex)
                        if (pParseTable->GetAction(actionVal, j) & ParseTable::ActionMask)
                            pValidTokenStackPositions[j]  = sp;
            }
        }

        if (!errorProdFound) {
            LastErrorState = pStack[StackPosition].State;
            PrintStack(ErrorStackStr);
            goto step_error;
        }

        if (nextActionValid) {
            // If there are reductions we can do on 'error' lookahead, do them first
            if ((pParseTable->GetAction(pStack[StackPosition].State, errorCode) &
                 ParseTable::ReduceMask) == 0u) {
                // Flush the remainder of stack symbols (this will also set StackPosition=sp)
                CleanupParseStack(nextStackPosition);
            }
        } else {
            // If an error production was found up the stack, we try to recover. This involves:
            //  1. Skipping all tokens, potentially including the last token that triggered the error,
            //     until a valid token is found.
            //       - No tokens may be skipped at all if the offending token is actually valid directly
            //         after an error. If you wrote "(5 + )" instead of "(5 + 5)" your original error
            //         token may be ')' but it doesn't actually need to be skipped.
            //         In this case, pValidTokenStackPositions[Token.Code] is valid up the stack, so no skipping
            //         takes place.
            //  2. Rolling back the stack, which includes StackPosition to a state that accepts
            //     an error followed by our next token. This is done by
            //         CleanupParseStack(pValidTokenStackPositions[Token.Code])
            //  3. Backtracking by one token and setting Token.Code to 'errorCode' to continue parsing.
            //         This will cause '%error' to be shifted onto the stack and allow things
            //         to move on with the next valid token after it.
            // (1.) Skip all 'unacceptable' tokens until a valid token, if any.
            SG_ASSERT(pValidTokenStackPositions[Token.Code] == InvalidIndex);
            TokenType tmpToken = Token;
            do {
                if (tmpToken.Code == TokenCode::TokenEOF) {
                    // If EOF was not in a valid look-ahead following %error, and we have hit
                    // the end of file, there is nothing left to do.
                    goto step_error;
                }
                if (tmpToken.Code == TokenCode::TokenError) {
                    // There are two reasons we can end up here:
                    //   a) Tokenizer generated TokenError because it saw unexpected characters,
                    //      which it had no reg-exp for. If this is the case, just skip them.
                    //   b) This is a second time through the error-handling loop for the same token.
                    //      We've already set Token.Code = errorCode for this token and tried
                    //      to recover, but for some reason this didn't work and we are back here.
                    //      Fail if this is the case.
                    if (PrevTokenIndex >= Stream.GetTokenIndex())
                        goto step_error;
                }
                GetNextToken(tmpToken);
            } while (pValidTokenStackPositions[tmpToken.Code] == InvalidIndex);
            // If there are reductions we can do on 'error' lookahead, do them first
            if ((pParseTable->GetAction(pStack[StackPosition].State, errorCode) &
                 ParseTable::ReduceMask) == 0u) {
                // (2.) Flush the remainder of stack symbols (this will also set StackPosition=sp)
                CleanupParseStack(pValidTokenStackPositions[tmpToken.Code]);
            }
        }
        // Set Token.Code to '%error', which will allow try_next_action to process appropriately,
        // with either shift or reduce. Also backtrack by one token so that the first valid token
        // can be re-consumed properly following an error.
        PrevTokenIndex = Stream.GetTokenIndex();
        Stream.SeekBack(1u);
        Token.Code = errorCode;
        goto try_next_action;
    }

step_error:
    // Clean up the parse stack by freeing all the elements
    CleanupParseStack();
    return false;
}

// Can be called on reduce to change reduce nonterminal
// Should only be done for user-controlled reductions
// In "A | B -> 'a';" you can force a reduce to B instead of A (default)
// Return false if failed (change not allowed for this state/nonterminal)
template <class StackElement>
bool Parse<StackElement>::SetReduceNonterminal(unsigned nonTerminal) noexcept {
    if (!pParseTable || size_t(nonTerminal) >= pParseTable->GetNonTerminalCount())
        return false;

    const auto topState = pParseTable->GetLeftReduceState(pStack[StackPosition - 1u].State,
                                                          nonTerminal);
    if (topState == InvalidState)
        return false;

    ReduceLeft = nonTerminal;
    TopState   = topState;

    return true;
}

// *** Debugging

template <class StackElement>
void Parse<StackElement>::PrintStack(String& str) const {
    str = StringWithFormat("%zu: ", StackPosition);
    for (size_t i = 0u; i <= StackPosition; ++i)
        str += StringWithFormat("[s%u]", pStack[i].State);
    str += "\n";
}

} // namespace SGParser

#endif
