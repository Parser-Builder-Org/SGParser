// Filename:  DFAGen.cpp
// Content:   DFA Generator class implementation source file
// Provided AS IS under MIT License; see LICENSE file in root folder.
//
// Has a DFAGen class that constructs a DFA table from a NFA.
// Should be possible to make it yet much faster by using sets of characters for states.

#include "DFAGen.h"
#include "Tokenizer.h"

namespace SGParser
{
namespace Generator
{

// ***** Deterministic Finite Automaton Generator

// Constructs a DFA from a NFA with 'subset construction' algorithm
// The resulting DFA is represented as a transition table
bool DFAGen::Create(const NFA& nfa, const std::vector<Lexeme>& lexemes,
                    [[maybe_unused]] unsigned maxChar) {
    // Make sure the DFA is not valid
    if (IsValid())
        return false;

    // A set of states (every element contains the NFA array subset)
    std::vector<std::vector<NFANode*>*> dfaStates;

    // Construct a list of all characters used in the NFA
    std::set<NFANode*> tempList;
    std::set<unsigned> charSet;

    nfa.TraverseGraph(nfa.pStartState, tempList);

    // Search for maximum character used (to determine the width of the table)
    // And also store all characters used in the char set
    for (const auto infaSet : tempList)
        charSet.insert(infaSet->LinkChar.begin(), infaSet->LinkChar.end());

    for (const auto icharSet : charSet)
        CharTable.SetValue(icharSet, unsigned(GetCharCount()));

    // Put e-closure(pnfa.initialState) in for the first state of dfaStates
    auto peclosure = new std::vector<NFANode*>;
    peclosure->push_back(nfa.pStartState);
    EpsilonClosure(*peclosure);
    dfaStates.push_back(peclosure);

    // Next state std::vector, always empty
    auto pnextState = new std::vector<NFANode*>;

    // And allocate the transition table for the first state
    TransitionTable.resize(1u);
    TransitionTable[0u].resize(GetCharCount(), EmptyTransition);

    // Create subset states
    for (size_t state = 0u; state < dfaStates.size(); ++state) {
        std::map<unsigned, std::vector<NFANode*>> charLinks;

        // Calculate Move on all characters
        const auto& dfaState = *dfaStates[state];

        // Go through and insert all character links with their destination
        for (const auto it : dfaState)
            for (size_t j = 0u; j < it->LinkChar.size(); ++j)
                if (const auto ch = it->LinkChar[j]; ch != 0u)
                    charLinks[ch].push_back(it->LinkPtr[j]);

        // Go through all characters
        for (const auto& [ch, nodes] : charLinks) {
            // There can never be empty node sets, because they wouldn't have been added
            // Copy next std::vector
            *pnextState = nodes;

            // Sort states so that we can use a linear search
            std::sort(pnextState->begin(), pnextState->end());

            // Add all states reachable through epsilon-links
            EpsilonClosure(*pnextState);

            // If U is not a state in dfaStates, add it
            // Linear search is ok, because NFA states are sorted
            size_t j = 0u;
            for (; j < dfaStates.size(); ++j)
                if (*dfaStates[j] == *pnextState)
                    break;

            // if not found
            if (j == dfaStates.size()) {
                // Add a new state
                dfaStates.push_back(pnextState);
                // And allocate transition table for the state
                const auto size = TransitionTable.size();
                TransitionTable.resize(size + 1u);
                TransitionTable[size].resize(GetCharCount(), EmptyTransition);
                // Allocate std::vector for next state
                pnextState = new std::vector<NFANode*>;
            }

            // Set transition for this state on a given character 'chars[i]'
            // To a new state j (made out of move for that character)
            SetTransitionState(unsigned(state), ch, unsigned(j));
        }
    }

    // Delete remaining next state std::vector
    delete pnextState;

    // Check for a note and report if needed
    const auto checkForNoteAndReport = [&](const char* message...) {
        if (Messages.GetMessageFlags() & ParseMessageBuffer::MessageNote) {
            va_list argList;
            va_start(argList, message);
            const auto messageStr = StringWithVAFormat(message, argList);
            va_end(argList);

            const ParseMessage msg{ParseMessage::NoteMessage, "", messageStr};
            Messages.AddMessage(msg);
        }
    };

    // Make an array that tells us which states are accepting states.
    // Zero means non-accepting, nonzero means accepting. The particular
    // nonzero value is the lowest lexeme ID number associated with the nodes
    AcceptStates.resize(TransitionTable.size(), StateType(0u));

    for (size_t i = 0u; i < TransitionTable.size(); ++i) {
        auto& acceptState = AcceptStates[i];
        // Does this state contain an accepting node?
        for (const auto& it: *dfaStates[i]) {
            const auto astate = StateType(it->AcceptingState);
            if (astate) {
                // Set pAcceptState[i] equal to the lexeme ID if either:
                //   1. It's the only non-zero lexeme ID we've seen
                //   2. Or it's appears the latest in file - so it has the lowest lexeme ID
                if (acceptState == 0u)
                    acceptState = astate;
                else if (acceptState < astate) {
                    // NOTE: Lexeme %s takes precedence over %s on state %d
                    checkForNoteAndReport("Lexeme '%s' takes precedence over '%s' on state %zu",
                                          lexemes[astate].Name.data(),
                                          lexemes[acceptState].Name.data(), i);
                    acceptState = astate;
                } else if (acceptState > astate)
                    // NOTE: Lexeme %s takes precedence over %s on state %d (other way around)
                    checkForNoteAndReport("Lexeme '%s' takes precedence over '%s' on state %zu",
                                          lexemes[acceptState].Name.data(),
                                          lexemes[astate].Name.data(), i);
            }
        }
    }

    // Clean out the allocated vectors in dfaStates
    for (const auto st: dfaStates)
        delete st;
    dfaStates.clear();

    // Create a lexeme information structure
    LexemeInfos.resize(lexemes.size() + 2u);

    // First 2 elements are default tokens
    LexemeInfos[0u].TokenCode = TokenCode::TokenError;
    LexemeInfos[0u].Action    = LexemeInfo::ActionNone;
    LexemeInfos[1u].TokenCode = TokenCode::TokenEOF;
    LexemeInfos[1u].Action    = LexemeInfo::ActionNone;

    // And copy data to it
    for (size_t i = 0u; i < lexemes.size(); ++i)
        LexemeInfos[i + 2u] = lexemes[i].Info;

    // Only one expression created
    ExpressionStartStates.push_back(StateType(0u));

    return true;
}


// Combines another DFA into this one, and assigns it a next expression state
// Source DFA is emptied
bool DFAGen::Combine(DFAGen& dfa) {
    // Both DFAs have to be valid
    if (!IsValid() || !dfa.IsValid())
        return false;

    // Both DFAs have to have the same Lexeme set & terminal size
    if (LexemeInfos.size() != dfa.GetLexemeCount())
        return false;

    // Concatenate tables, accept states and expression start tables
    // Create a new list of all the characters

    const auto stateCount       = TransitionTable.size();
    const auto sourceStateCount = dfa.TransitionTable.size();

    const auto newStartState    = StateType(stateCount);
    const auto newStateCount    = stateCount + sourceStateCount;

    const auto oldCharCount     = GetCharCount();

    // Add all new chars to the Chart table
    for (const auto& [pos, value]: dfa.CharTable)
        if (!CharTable.HasValue(pos))
            CharTable.SetValue(pos, unsigned(GetCharCount()));

    // Resize each state within the transition table
    const auto newCharCount = GetCharCount();
    if (oldCharCount != newCharCount)
        for (size_t i = 0u; i < stateCount; ++i)
            TransitionTable[i].resize(newCharCount, EmptyTransition);

    // Append the source DFAs transition table and accept states
    AcceptStates.resize(newStateCount);
    TransitionTable.resize(newStateCount);
    for (size_t i = 0u; i < sourceStateCount; ++i) {
        // Append each accept state
        AcceptStates[stateCount + i] = dfa.AcceptStates[i];
        // Relocate the source states values into this ones (check every character)
        TransitionTable[stateCount + i].resize(newCharCount, EmptyTransition);
        // Traverse the source character list and add a new state for all characters
        for (const auto& [pos, _]: dfa.CharTable) {
            // Get the transition state for the specific character
            const auto state = StateType(dfa.GetTransitionState(unsigned(i), unsigned(pos)));
            // If the state is not empty than insert it into the table
            if (state != EmptyTransition)
                TransitionTable[stateCount + i][GetCharIndex(unsigned(pos))] =
                    state + newStartState;
        }
    }

    // *** Expression

    // Convert source expression start states
    const auto exprCount       = ExpressionStartStates.size();
    const auto sourceExprCount = dfa.GetExpressionStartStateCount();
    ExpressionStartStates.resize(exprCount + sourceExprCount);
    for (size_t i = 0u; i < sourceExprCount; ++i)
        ExpressionStartStates[exprCount + i] =
            dfa.ExpressionStartStates[i] + newStartState;

    // Free data in source
    dfa.Destroy();

    // We are done
    return true;
}


// *** Internal functions

// TestString -- tests a string against the DFA
// Return LexemeID for match, 0 otherwise
unsigned DFAGen::TestString(const String& str) const {
    unsigned state = 0u;

    for (const auto ch: str) {
        // Test for character out of bounds, if it is - match fails
        if (!CharTable.HasValue(ch))
            return 0u;
        // If the transition state is empty than match fails
        state = GetTransitionState(state, ch);
        if (state == EmptyTransition)
            return 0u;
    }
    return GetAcceptState(state);
}


size_t DFAGen::Compress([[maybe_unused]] unsigned tableType, unsigned compressType) {
    size_t result = 0u;

    // *** Combine Duplicate rows

    if (compressType & CT_CombineDuplicate) {
        // Map the indexes from the old set to the new one
        // (keep inverse map for new table creation)
        std::map<StateType, StateType>      match;
        std::map<StateType, StateType>      invmatch;
        // New transition table and character index set
        std::vector<std::vector<StateType>> newTable;
        std::set<StateType>                 newSet;

        for (StateType icharSet = 0u; icharSet < StateType(TransitionTable[0u].size());
             ++icharSet) {
            if (match.find(icharSet) != match.end())
                continue;

            // Add new transition set and set the match states
            const auto newSetCount = StateType(newSet.size());
            match[icharSet]        = newSetCount;
            invmatch[newSetCount]  = icharSet;
            newSet.insert(icharSet);

            for (const auto& [_, value]: CharTable) {
                const auto index = StateType(value);

                if (index <= icharSet || match.find(index) != match.end())
                    continue;

                // Test to see if the two transition tables are the same
                size_t istate = 0u;
                bool same = true;
                while (istate < TransitionTable.size() && same)
                    if (TransitionTable[istate][icharSet] != TransitionTable[istate][index])
                        same = false;
                    else
                        ++istate;

                // If they are the same than store a match
                if (same)
                    match[index] = newSetCount;
            }
        }

        // Initialize the new table
        newTable.resize(TransitionTable.size());
        for (size_t istate = 0u; istate < TransitionTable.size(); ++istate) {
            newTable[istate].resize(newSet.size());
            for (size_t icharSet = 0u; icharSet < newSet.size(); ++icharSet)
                newTable[istate][icharSet] =
                    TransitionTable[istate][invmatch[StateType(icharSet)]];
        }

        // Store the character
        for (auto& [_, value]: CharTable)
            value = unsigned(match[StateType(value)]);

        // Update the Transition table to the new (compressed) table
        TransitionTable.swap(newTable);
        result = newSet.size();
    }

    // *** Remove Empty

    if (compressType & CT_RemoveEmpty) {
        size_t elist = 0u;

        for (auto& row: TransitionTable) {
            // Check to see if the state is empty
            size_t k = 0u;
            bool empty = true;
            while (k < row.size() && empty) {
                if (row[k] != EmptyTransition)
                    empty = false;
                ++k;
            }

            // If so then clear it
            if (empty) {
                row.resize(0u);
                ++elist;
            }
        }

        EmptyStateCount = elist;
        result = 1u;
    }

    return result;
}


// Create a static DFA structure
bool DFAGen::CreateStaticDFA(String& str, const String& name, const String& namespaceName) const {
    // The DFA must be valid
    if (!IsValid())
        return false;

    static constexpr size_t asRowCount = 10u;

    const auto tableWidth      = TransitionTable[0u].size();
    const auto tableHeight     = TransitionTable.size();
    const auto lexemeCount     = LexemeInfos.size();
    const auto expressionCount = ExpressionStartStates.size();

    String dest;

    // *** Add the includes

    dest += "#include \"ParseTableType.h\"\n\n#include <cstdint>\n\n";

    // *** Add namespace declaration if needed

    if (!namespaceName.empty())
        dest += "namespace " + namespaceName + "\n{\n\n";

    // *** Add the Transition static data

    dest += StringWithFormat("static uint16_t %s_TransitionTable[%zu][%zu] =\n{",
                             name.data(), tableHeight, tableWidth);

    // Go through all the transitions and add them
    auto sepV = "\n";
    for (const auto& row: TransitionTable) {
        dest += sepV + String{"    {"};
        auto sepH = "";
        for (const unsigned state: row) {
            dest += sepH + StringWithFormat("0x%04X", state);
            sepH = ", ";
        }
        dest += "}";
        sepV = ",\n";
    }
    dest += "\n};\n\n";

    // *** Add the AcceptStates static data

    dest += StringWithFormat("static uint16_t %s_AcceptStates[%zu] =\n{",
                             name.data(), tableHeight);
    // Go through all the accept states and add them
    size_t i = 0u;
    auto sep = "";
    for (const unsigned state: AcceptStates) {
        dest += sep;
        if (i % asRowCount == 0u && i != tableHeight)
            dest += "\n    ";
        dest += StringWithFormat("0x%04X", state);
        sep = ", ";
        ++i;
    }
    dest += "\n};\n\n";

    // *** Add the Lexeme Info static data

    dest += StringWithFormat("static SGParser::LexemeInfo %s_LexemeInfos[%zu] =\n{",
                             name.data(), lexemeCount);

    // Go through all the accept states and add them
    sep = "";
    for (const auto& [tokenCode, action]: LexemeInfos) {
        dest += sep + StringWithFormat("\n    {%u, SGParser::LexemeInfo::", tokenCode);

        switch (action & LexemeInfo::ActionMask) {
            case LexemeInfo::ActionNone: dest += "ActionNone";  break;
            case LexemeInfo::ActionGoto: dest += "ActionGoto";  break;
            case LexemeInfo::ActionPush: dest += "ActionPush";  break;
            case LexemeInfo::ActionPop:  dest += "ActionPop";   break;
            default:                     dest += "ActionError"; break;
        }

        const auto actionValue = action & LexemeInfo::ActionValueMask;
        if (actionValue > 0u)
            dest += StringWithFormat(" | 0x%08X", actionValue);

        dest += "}";
        sep = ",";
    }
    dest += "\n};\n\n";

    // *** Add the Expression static data

    dest += StringWithFormat("static uint16_t %s_ExpressionStartStates[%zu] =\n{\n    ",
                             name.data(), expressionCount);

    // Go through all the accept states and add them
    sep = "";
    for (const unsigned state: ExpressionStartStates) {
        dest += sep + StringWithFormat("%u", state);
        sep = ", ";
    }
    dest += "\n};\n\n";

    // *** Character table

    dest += StringWithFormat("static uint8_t %s_CharTable[%zu][2] =\n{\n",
                             name.data(), CharTable.size());

    // Create an ordered map from (unordered) CharTable only
    // for the sake of pretty looking output
    std::map<size_t, decltype(CharTable)::value_type> sortedCharTable{CharTable.begin(),
                                                                      CharTable.end()};
    sep = "";
    for (const auto& [pos, value]: sortedCharTable) {
        dest += sep + StringWithFormat("    {%zu, %u}", pos, value);
        sep = ",\n";
    }
    dest += "\n};\n\n";

    // *** Add the StaticDFA structure

    dest += StringWithFormat("static SGParser::StaticDFA %s =\n"
                "{\n"
                "    %zuu,\n"
                "    %zuu,\n"
                "    %s_TransitionTable[0u],\n"
                "    %s_AcceptStates,\n"
                "    %zuu,\n"
                "    %s_CharTable[0u],\n"
                "    %zuu,\n"
                "    %s_LexemeInfos,\n"
                "    %zuu,\n"
                "    %s_ExpressionStartStates\n"
                "};\n",
                name.data(),
                tableWidth,
                tableHeight,
                name.data(),
                name.data(),
                CharTable.size(),
                name.data(),
                lexemeCount,
                name.data(),
                expressionCount,
                name.data()
    );

    // Close namespace declaration of needed
    if (!namespaceName.empty())
        dest += "\n} // namespace " + namespaceName + "\n";

    str.swap(dest);

    return true;
}


size_t DFAGen::GetTableSize(TableType type) const noexcept {
    switch (type) {
        case TableType::TransitionTable:
            return ((TransitionTable.size() - EmptyStateCount) * TransitionTable[0u].size() *
                    sizeof(EmptyStateCount) + TransitionTable.size() *
                    sizeof(std::vector<StateType>));

        case TableType::AcceptStates:
            return AcceptStates.size() * sizeof(StateType);
    }

    SG_ASSERT(false);
    return 0u;
}


// Computes the epsilon closure of a set of nodes. This is a set of nodes that
// can be reached with epsilon (empty) links alone
void DFAGen::EpsilonClosure(std::vector<NFANode*>& setOfStates) {
    // For all not yet expanded states
    for (size_t pos = 0u; pos < setOfStates.size(); ++pos) {
        const auto node = setOfStates[pos];

        // For epsilon all character links
        for (size_t i = 0u; i < node->LinkChar.size(); ++i)
            if (node->LinkChar[i] == NFANode::Epsilon) {
                const auto pLinkNode = node->LinkPtr[i];

                // Is node already added? If not added, add node
                if (std::find(setOfStates.begin(), setOfStates.end(), pLinkNode) ==
                    setOfStates.end())
                    setOfStates.push_back(pLinkNode);
            }
    }
}

} // namespace Generator
} // namespace SGParser
