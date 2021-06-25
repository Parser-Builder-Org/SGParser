// Filename:  DFAGen.h
// Content:   DFA Generator declarations
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_GENERATOR_DFAGEN_H
#define INC_SGPARSER_GENERATOR_DFAGEN_H

#include "NFA.h"
#include "DFA.h"
#include "ParseMessage.h"
#include "Lexeme.h"
#include "MappedTable.h"

#include <vector>

namespace SGParser
{
namespace Generator
{

// ***** Generic DFA syntax tree node used to construct a DFA from a syntax tree

template <class TreeNode>
class DFASyntaxTree final
{
public:
    unsigned               LexemeId = 0u;
    // The root node of the syntax tree
    TreeNode*              pRoot    = nullptr;
    // A list of all the character nodes (used for efficiency, non-owning)
    std::vector<TreeNode*> CharNodes;

    DFASyntaxTree() = default;

    // No copy/move allowed
    DFASyntaxTree(const DFASyntaxTree&)                = delete;
    DFASyntaxTree(DFASyntaxTree&&) noexcept            = delete;
    DFASyntaxTree& operator=(const DFASyntaxTree&)     = delete;
    DFASyntaxTree& operator=(DFASyntaxTree&&) noexcept = delete;

    ~DFASyntaxTree()              { delete pRoot; }

    bool IsValid() const noexcept { return pRoot != nullptr; }
};


// ***** DFAGen Class declaration

class DFAGen final : public DFA
{
public:
    enum class TableType
    {
        TransitionTable,
        AcceptStates
    };

    static constexpr unsigned CT_RemoveEmpty      = 0x01;
    static constexpr unsigned CT_CombineDuplicate = 0x02;

public:
    // Creates an empty DFA
    DFAGen() = default;

    // Initializes the DFA using an NFA (pnfa)
    DFAGen(const NFA& nfa, const std::vector<Lexeme>& lexemes, unsigned maxChar) {
        // Create the DFA
        Create(nfa, lexemes, maxChar);
    }

    // Creates a DFA from an NFA
    bool     Create(const NFA& nfa, const std::vector<Lexeme>& lexemes, unsigned maxChar = 0u);

    // Compress the DFA data
    // Returns the new set size
    size_t   Compress(unsigned tableType, unsigned compressionType);

    // Combines another DFA into this one, and assigns it a next expression state
    // Source DFA is emptied
    bool     Combine(DFAGen& sourceDFA);

    // Tests the string against the DFA
    // Return LexemeID for match, 0 otherwise
    unsigned TestString(const String& str) const;

    // Return the empty state count
    size_t   GetEmptyStateCount() const noexcept    { return EmptyStateCount; }

    // Return the byte size of the specified table
    size_t   GetTableSize(TableType type) const noexcept;

    // Return the type of compression used by the DFA
    unsigned GetCompressionType() const noexcept    { return 0u; }

    // Returns the list (buffer) of processing messages (errors, warnings, etc.)
    ParseMessageBuffer& GetMessageBuffer() noexcept { return Messages; }

    // Create a static DFA structure
    bool     CreateStaticDFA(String& str, const String& name,
                             const String& namespaceName = String{}) const;

private:
    friend class Lex;

    // Message buffer for errors, warnings, notes, etc.
    ParseMessageBuffer Messages;

    size_t             EmptyStateCount = 0u;

    // Calculates a set of states that can be accessed by
    // epsilon (empty) links from a given set of states
    void     EpsilonClosure(std::vector<NFANode*>& setOfStates);

    void     SetTransitionState(unsigned state, unsigned ch, unsigned value) {
        TransitionTable[state][GetCharIndex(ch)] = StateType(value);
    }
};

} // namespace Generator
} // namespace SGParser

#endif // INC_SGPARSER_GENERATOR_DFAGEN_H
