// Filename:  DFA.cpp
// Content:   DFA class implementation source file
// Provided AS IS under MIT License; see LICENSE file in root folder.
//
// Should be possible to make it yet much faster by using sets of characters for states.

#include "DFA.h"

using namespace SGParser;

// ***** Deterministic Finite Automaton

// Creates a DFA using a static DFA structure
// Table data is cleared before the creation and will remain
// clear in case of exceptions (basic exception safety is provided)
void DFA::Create(const StaticDFA& staticDFA) {
    // Free the existing data
    Destroy();

    // Try to reserve needed space
    AcceptStates.reserve(staticDFA.TableHeight);
    LexemeInfos.reserve(staticDFA.LexemeCount);
    ExpressionStartStates.reserve(staticDFA.ExpressionCount);

    // Temporary container for swap-initialization of TransitionTable
    decltype(TransitionTable) newTransitionTable;
    newTransitionTable.reserve(staticDFA.TableHeight);
    for (size_t i = 0u; i < staticDFA.TableHeight; ++i) {
        newTransitionTable.emplace_back();
        newTransitionTable.back().reserve(staticDFA.TableWidth);
        const auto src = staticDFA.pTransitionTable + staticDFA.TableWidth * i;
        newTransitionTable.back().assign(src, src + staticDFA.TableWidth);
    }

    // Temporary container for swap-initialization of CharTable
    decltype(CharTable) newCharTable;
    // Static CharTable stored as unsigned {index, value} pairs
    auto pChar = staticDFA.pCharTable;
    for (size_t i = 0u; i < staticDFA.CharCount; ++i, pChar += 2u)
        newCharTable.SetValue(pChar[0u], pChar[1u]);

    // From this point we can (safely) initialize the actual data

    // Swap-initialize the transition table
    TransitionTable.swap(newTransitionTable);
    // Initialize the accept states
    AcceptStates.assign(staticDFA.pAcceptState, staticDFA.pAcceptState + staticDFA.TableHeight);
    // Swap-initialize the character table
    CharTable.swap(newCharTable);
    // Initialize the lexeme infos
    LexemeInfos.assign(staticDFA.pLexemeInfos, staticDFA.pLexemeInfos + staticDFA.LexemeCount);
    // Initialize the expressions
    ExpressionStartStates.assign(staticDFA.pExpressionStartState,
                                 staticDFA.pExpressionStartState + staticDFA.ExpressionCount);
}


// Destroy the DFA data and reset its variables
void DFA::Destroy() noexcept {
    CharTable.clear();
    TransitionTable.clear();
    AcceptStates.clear();
    ExpressionStartStates.clear();
    LexemeInfos.clear();
}


// Debugging routine to print a DFA to a string suitable
// for display with the UC Berkeley dotty program
void DFA::PrintDFADotty(String& str) const {
    String dest = "digraph G {\n";

    for (unsigned state = 0u; state < unsigned(GetStateCount()); ++state) {
        const auto acceptState = GetAcceptState(state);
        if (acceptState)
            dest += StringWithFormat("n%u [label=\"n%u: a%u\" peripheries=2]\n",
                                     state, state, acceptState);

        for (unsigned i = 0u; i < unsigned(GetCharCount()); ++i) {
            const auto transState = GetTransitionState(state, i);
            if (transState == EmptyTransition)
                continue;

            String label;
            if (i >= 32u && i <= 126u)
                label = StringWithFormat("'%c'", char(i));
            else
                label = StringWithFormat("%u", i);

            dest += StringWithFormat("n%u -> n%u [label=\"%s\"]\n",
                                     state, transState, label.data());
        }
    }
    dest += "}\n";

    str.swap(dest);
}
