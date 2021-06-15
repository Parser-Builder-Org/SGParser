// Filename:  DFATokenizer.h
// Content:   DFATokenizer class header file with declarations
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_DFATOKENIZER_H
#define INC_SGPARSER_DFATOKENIZER_H

#include "Tokenizer.h"
#include "LexemeInfo.h"
#include "DFA.h"

#include <vector>

namespace SGParser
{

// ***** DFA-Based tokenizer used for general parsing

template <class Token>
class DFATokenizer : public TokenizerImpl<Token> 
{
public:
    // Default constructor
    DFATokenizer() = default;

    // Initialization constructor
    explicit DFATokenizer(const DFA* pdfa, InputStream* pinputStream = nullptr) {
        Create(pdfa, pinputStream);
    }

    // Initializes the tokenizer to use a specified DFA and input stream
    bool Create(const DFA* pdfa, InputStream* pinputStream = nullptr);

    // *** Token Stream interface

    // Return next token / EOF Token
    Token& GetNextToken(Token& token) override;

private:
    using CodeType = typename Token::CodeType;

    using typename TokenizerBase::ByteReader;
    using typename TokenizerImpl<Token>::InputCharReader;
    using typename TokenizerImpl<Token>::BufferPos;
    using typename TokenizerImpl<Token>::PosTracker;
    using TokenizerImpl<Token>::SetInputStream;
    using TokenizerImpl<Token>::HeadPos;
    using TokenizerImpl<Token>::TailPos;
    using TokenizerImpl<Token>::GetHeadPos;
    using TokenizerImpl<Token>::GetTailPos;
    using TokenizerImpl<Token>::SetTailPos;
    using TokenizerImpl<Token>::AdjustHead;

    const DFA* pDFA               = nullptr;
    unsigned   ExpressionStackTop = DFA::EmptyTransition;

    // Expression stack, controls starting state
    std::vector<unsigned> ExpressionStack;
};

// *** DFA Tokenizer implementation

template <class Token>
bool DFATokenizer<Token>::Create(const DFA* pdfa, InputStream* pinputStream) {
    if (!SetInputStream(pinputStream))
        return false;
    pDFA = pdfa;

    // Starting with expressions 0 in dfa
    ExpressionStackTop = 0u;
    ExpressionStack.clear();
    return true;
}

// Gets next token, return TokenCode
template <class Token>
Token& DFATokenizer<Token>::GetNextToken(Token& token) {
    CodeType        code;
    InputCharReader charReader{ByteReader{*this}, TailPos};

    do {
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

        // Try to find the longest matching lexeme
        // Keep going through the DFA until we hit an error condition,
        // that is, we get stuck and have nowhere to go

        // The initial state of a DFA is always the zeroth state
        unsigned state        = pDFA->GetExpressionStartState(ExpressionStackTop);
        unsigned lastLexemeID = DFA::EmptyTransition;
        // Last recognized Tail
        BufferPos lastTail;
        // Save start head, so that we can see if we moved at all
        BufferPos startHead{GetHeadPos()};

        // Save the last tail line and column position
        PosTracker lastTailPos{TailPos};

        unsigned accept;

        do {
            // Get the next state out of the table
            state = pDFA->GetTransitionState(state, charReader.GetChar());

            // EmptyTransition means error condition
            if (state == pDFA->EmptyTransition)
                break;

            // If it's an accepting state, record the pTail position and
            // lexeme ID so we can return them if this turns out to be the longest lexeme
            if (accept = pDFA->GetAcceptState(state); accept != 0u) {
                lastLexemeID = accept;
                accept       = charReader.Advance() ? 1u : 0u;
                lastTailPos  = TailPos;
                lastTail     = GetTailPos();
            } else
                accept = charReader.Advance() ? 1u : 0u;

        } while (accept != 0u);

        // If we didn't find a valid lexeme, raise an error, unless we've
        // got an empty lexeme. In this case, there are simply no more characters
        // remaining in the input stream
        if (lastLexemeID == DFA::EmptyTransition) {
            // If we didn't move, and its the end of file, report EOF
            if (charReader.IsEOF() && startHead == GetTailPos())
                code = TokenCode::TokenEOF;
            else {
                // Error - nothing recognized
                SetTailPos(startHead);
                TailPos = HeadPos;
                code    = TokenCode::TokenError;
                if (!charReader.IsEOF())
                    charReader.Advance();
            }
            token.CopyFromTokenizer(code, *this);
            return token;
        }

        // Put the pTail at the end of the lexeme found
        SetTailPos(lastTail);
        // Set the line/offset values to the end of the last lexeme found
        TailPos = lastTailPos;

        // Otherwise, return the lexeme
        // Should be [*pHead, *LastTailPos]
        const auto& lexinfo = pDFA->GetLexemeInfo(lastLexemeID);
        code                = lexinfo.TokenCode;

        // Perform an action, if any
        switch (lexinfo.Action & LexemeInfo::ActionMask) {
            case LexemeInfo::ActionPush:
                ExpressionStack.push_back(ExpressionStackTop);
                // Fall through to goto
                [[fallthrough]];
                
            case LexemeInfo::ActionGoto:
                ExpressionStackTop = lexinfo.Action & LexemeInfo::ActionValueMask;
                break;

            case LexemeInfo::ActionPop:
                if (!ExpressionStack.empty()) {
                    ExpressionStackTop = ExpressionStack.back();
                    ExpressionStack.pop_back();
                } else { // Empty stack, error
                    ExpressionStackTop = 0u;
                    token.CopyFromTokenizer(TokenCode::TokenError, *this);
                    return token;
                }
                break;
        }

        // if code == 0, then ignore token
    } while (code == 0u);

    token.CopyFromTokenizer(code, *this);
    return token;
}

} // namespace SGParser

#endif // INC_SGPARSER_DFATOKENIZER_H
