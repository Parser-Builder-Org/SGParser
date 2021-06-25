// Filename:  DFA.h
// Content:   DFA declarations
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_DFA_H
#define INC_SGPARSER_DFA_H

#include "SGString.h"
#include "LexemeInfo.h"
#include "MappedTable.h"

#include <vector>

namespace SGParser
{

// ***** Deterministic Finite Automaton (DFA)

// *** Static DFA structure declaration

// Used for hard-coded DFA structures
struct StaticDFA final
{
    size_t      TableWidth;
    size_t      TableHeight;
    uint16_t*   pTransitionTable;
    uint16_t*   pAcceptState;
    size_t      CharCount;
    uint8_t*    pCharTable;   // table of {index, value}
    size_t      LexemeCount;
    LexemeInfo* pLexemeInfos;
    size_t      ExpressionCount;
    uint16_t*   pExpressionStartState;
};


// ***** DFA Class declaration

class DFA
{
public:
    using StateType = uint16_t;

    static constexpr StateType EmptyTransition = StateType(-1);

public:
    // Creates an empty DFA
    DFA() = default;

    // Initializes the DFA using a static DFA structure
    explicit DFA(const StaticDFA& staticDFA) {
        Create(staticDFA);
    }

    // Creates a DFA using a static DFA structure
    // Table data is cleared before the creation and will remain
    // clear in case of exceptions (basic exception safety is provided)
    void     Create(const StaticDFA& staticDFA);

    // Destroy the DFA and reset it to empty
    void     Destroy() noexcept;

    // Determines whether the DFA is valid
    bool     IsValid() const noexcept                        { return !TransitionTable.empty(); }

    // Returns the transition state
    unsigned GetTransitionState(unsigned state, unsigned ch) const {
        const auto index = GetCharIndex(ch);
        return index < TransitionTable[state].size() ? TransitionTable[state][index]
                                                     : EmptyTransition;
    }

    // Return the accept state
    unsigned GetAcceptState(unsigned state) const            { return AcceptStates[state]; }

    // Return the expression start state for a given state
    unsigned GetExpressionStartState(unsigned state) const {
        return ExpressionStartStates[state];
    }

    // Return table size
    size_t   GetStateCount() const noexcept                  { return TransitionTable.size(); }

    // Return the number of characters currently indexed
    size_t   GetCharCount() const noexcept                   { return CharTable.size(); }

    // Return the number of expression start states
    size_t   GetExpressionStartStateCount() const noexcept {
        return ExpressionStartStates.size();
    }

    // Return the number of lexemes (lexeme infos)
    size_t   GetLexemeCount() const noexcept                 { return LexemeInfos.size(); }

    // Returns the lexeme info
    const LexemeInfo& GetLexemeInfo(unsigned lexemeId) const { return LexemeInfos[lexemeId]; }

    // PrintDFADotty is a debugging routine to print a DFA to
    // a string suitable for display with the UC Berkeley dotty program
    void     PrintDFADotty(String& str) const;

protected:
    using CharTableType = MappedTable<unsigned, unsigned(-1)>;

    std::vector<std::vector<StateType>> TransitionTable;
    std::vector<StateType>              AcceptStates;

    // Lexeme infos for each lexeme
    std::vector<LexemeInfo>             LexemeInfos;
    // A DFA can be a set of different DFAs, representing lexeme sets - expression
    // Each expression has a separate starting state
    std::vector<StateType>              ExpressionStartStates;

    CharTableType                       CharTable;

    // Return the transition state for a given character and state
    unsigned GetCharIndex(unsigned ch) const {
        return CharTable.GetValue(ch);
    }
};

} // namespace SGParser

#endif // INC_SGPARSER_DFA_H
