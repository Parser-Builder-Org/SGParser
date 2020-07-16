// Filename:  NFA.h
// Content:   NFA declarations
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_GENERATOR_NFA_H
#define INC_SGPARSER_GENERATOR_NFA_H

#include "SGString.h"

#include <set>
#include <vector>

namespace SGParser {
namespace Generator {

// ***** Nondeterministic Finite Automaton (NFA) declaration

// *** NFANode

// Represents a state in NFA
struct NFANode final
{
    // Static Epsilon value
    static constexpr unsigned Epsilon = 0u;

    // Node ID
    unsigned              Id;
    // 0 if not accepting, Lexeme ID # > 0 if accepting
    unsigned              AcceptingState;
    // A list of transitions to different nodes for a particular character
    // Epsilon (empty link) is character value 0
    std::vector<unsigned> LinkChar;
    std::vector<NFANode*> LinkPtr;
};


// *** NFA class

class NFA final
{
public:
    // Flags for computing Kleene '*', '+' or '?' operator on the NFA
    enum class KleeneType
    {
        ConnectEmpty,
        ConnectBack,
        ConnectBoth
    };

    // Const for invalid lexeme representation
    static constexpr unsigned InvalidLexeme = unsigned(-1);

public:
    // *** Constructors & destructor

    // Constructs a completely empty NFA
    NFA() = default;

    // Just constructs an empty NFA
    // Rarely used since the other constructors are much more useful
    explicit NFA(unsigned lexemeID)
        : LexemeId{lexemeID} { 
    }

    // Construct a NFA that accepts the one character String
    NFA(unsigned c, unsigned lexemeId) {
        Create(c, lexemeId); 
    }

    // Construct a NFA that accepts any one of the given characters
    NFA(const std::vector<unsigned>& c, unsigned lexemeId) {
        Create(c, lexemeId);
    }

    // Copy NFA n
    // Optionally assign a new lexeme id
    NFA(const NFA& nfa, unsigned newLexemeId = 0u)
        : pStartState{nullptr} {
        Create(nfa, newLexemeId);
    }

    // No copy/move allowed
    NFA(const NFA&)                = delete;
    NFA(NFA&&) noexcept            = delete;
    NFA& operator=(const NFA&)     = delete;
    NFA& operator=(NFA&&) noexcept = delete;

    // Destructor
    ~NFA() {
        Destroy();
    }

    // Construct a NFA that accepts the one character String
    bool     Create(unsigned c, unsigned lexemeID);
    // Construct a NFA that accepts any one of the given characters
    bool     Create(const std::vector<unsigned>& c, unsigned lexemeID);
    // Construct a NFA from another NFA (n)
    // Assigns a new lexeme id if newLexemeId != 0
    bool     Create(const NFA& nfa, unsigned newLexemId = 0u);

    // Destroy the NFA and reset it to empty
    void     Destroy();

    // Returns whether or not the NFA is valid
    bool     IsValid() const noexcept { return (pStartState != nullptr); }

    // *** Utility Functions

    // NOTE: Contents of parameter NFAs are emptied
    // for MoveData, Concat, Or, and Kleene

    // Moves NFA's data to this NFA
    void     MoveData(NFA& source);

    // Concatenates another NFA to this one
    void     Concat(NFA& nfa);

    // Or's two NFAs and stores the result in this NFA
    void     Or(NFA& nfa);

    // Computes Kleene '*', '+' or '?' operator on the NFA
    void     Kleene(KleeneType type = KleeneType::ConnectBoth);

    // Combines a set of NFAs into one by Or'ing them all together
    void     CombineNFAs(const std::vector<NFA*>& nfaList);

    // Get the lexeme ID of the NFA
    unsigned GetLexemeID() const noexcept { return LexemeId; }

    // Debugging print routines
    void     PrintFA() const;
    void     PrintFADotty(String& str) const;

    // Assigns ID numbers to each node, mostly for debugging
    void     NumberFA() const;

private:
    friend class DFAGen;

    using NodeSet = std::set<NFANode*>;

    // ID to identify our particular final states when combined with another NFA
    // Can not be 0 (0 means Epsilon)
    unsigned              LexemeId    = InvalidLexeme;

    // The start state of the NFA
    NFANode*              pStartState = nullptr;
    // A list of all final states
    // Redundant with the tree, but necessary for efficiency
    std::vector<NFANode*> FinalState;

    // *** Internal functions

    // Allocate a new node
    // If accepting, sets it to accept LexemeID & adds it to FinalState
    NFANode* NewState(unsigned accepting);

    // Form a new link between two existing nodes on character c
    void     AddLink(NFANode* pmodify, unsigned c, NFANode* ptarget);

    // Traverses the graph and constructs a list of all nodes reachable from seed in visited
    void     TraverseGraph(NFANode* pseed, NodeSet& visited) const;
};

} // namespace Generator
} // namespace SGParser

#endif // INC_SGPARSER_GENERATOR_NFA_H
