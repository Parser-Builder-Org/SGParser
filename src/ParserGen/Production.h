// Filename:  Production.h
// Content:   Production class header file with declarations
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_GENERATOR_PRODUCTION_H
#define INC_SGPARSER_GENERATOR_PRODUCTION_H

#include "SGString.h"
#include "ProductionMask.h"

#include <map>
#include <set>
#include <vector>

namespace SGParser
{
namespace Generator
{

// ***** Production

// Every nonterminal in a production is assigned a number
// Looks like: Left -> Right[0] Right[1] ... Right[Length - 1]
struct Production final
{
    // A terminal on RHS can have associated conflict actions
    struct ConflictAction final
    {
        enum class Action
        {
            Shift,
            Reduce
        };
        // Conflict actions map reduce options (Nonterminals) to pre-defined behavior
        std::map<unsigned, Action> Actions;
    };

    // *** Production header

    // Name of the production
    String    Name;
    // A unique Id of a production, always < Grammar::ProductionCount
    unsigned  Id            = 0u;
    // Production precedence value
    unsigned  Precedence    = 0u;
    // The number of the nonterminal on the LHS of the production
    unsigned  Left          = 0u;
    // Right hand side of the production
    // Right hand side arrays can be shared, so they are reference counted with use of pRight[0]
    unsigned  Length        = 0u;
    unsigned* pRight        = nullptr;
    // The line number the production was declared on
    size_t    Line          = 0u;
    // Flag to tell whether or not it is to be reported to the user when reduced
    bool      NotReported   = false;
    // ErrorTerminal (to throw %error(Name) on reduce, 0 for none)
    unsigned  ErrorTerminal = 0u;
    // Reduce map
    std::map<unsigned, std::set<unsigned>> ReduceOverrides;
    // Chain of Left hand non terminals for user controlled code
    std::vector<unsigned>                  LeftChain;
    // Map for RHS index
    std::map<unsigned, ConflictAction>     ConflictActions;

    // *** Constructors & destructor

    // Default constructor
    Production() = default;

    // Copy constructor
    Production(const Production& source) {
        pRight = nullptr;
        *this  = source;
    }

    // Constructor with default values
    Production(const String& name, unsigned left, const unsigned* pright,
               unsigned rightCount, size_t line = 0u, unsigned prec = 0u);

    // Destructor
    ~Production();

    // Set production value
    void      SetProduction(const String& name, unsigned left, const unsigned* pright,
                            unsigned rightCount, size_t line = 0u, unsigned prec = 0u);

    // Should be used to access RHS
    unsigned& Right(unsigned index) noexcept { return pRight[index + 1u]; }

    // *** Helper functions

    // Copy operator
    Production& operator=(const Production& right);

    // Compare production - Special case of Equals
    bool operator==(const Production& right) const noexcept;
    // Compare Right hand side
    bool RHSEquals(const Production& right) const noexcept;
};


// ***** Parse table production

// Includes the dot and lookahead
// This is provided for efficiency purposes
struct ParseTableProduction final
{
    // Pointer to the production
    Production*        pProduction = nullptr;
    // Position in RHS
    unsigned           Dot         = 0u;
    // Lookahead character in LR and LALR (practically, a character
    // that can possibly follow this production)
    // Must be a terminal, or'd with the TerminalMask
    std::set<unsigned> LookAhead;

    // Constructors
    ParseTableProduction() = default;

    explicit ParseTableProduction(Production* pprod, unsigned dot = 0u)
        : pProduction{pprod},
          Dot{dot} {
    }

    // Compare production (Left, Right, Dot & Lookahead)
    // Special case of Equals
    bool operator==(const ParseTableProduction& right) const noexcept;
    // Less operator, for sorting productions
    // Sorts by Id, Dot, and LookAhead
    bool operator<(const ParseTableProduction& right) const noexcept;

    // *** Debugging

    // Store the production into a String
    void   Print(String& str, std::map<unsigned, const String*>* pgrammarSymbolsInv = nullptr,
                 size_t leftSpace = 0u, size_t rightSpace = 0u) const;
    // Prints the Right hand side of the production
    // Return character position of dot (or 0)
    size_t PrintRHS(String& str, std::map<unsigned, const String*>* pgrammarSymbolsInv = nullptr,
                    bool printDot = true) const;
    // Prints the look ahead characters (terminals)
    void   PrintLookAhead(String& str,
                          std::map<unsigned, const String*>* pgrammarSymbolsInv = nullptr) const;

    // *** Utility Functions for production data structures

    // Return index if production std::vector is in a set of std::vectors, -1 for error
    static size_t FindVectorInSetOfSets(const std::vector<ParseTableProduction>& v,
                          const std::vector<std::vector<ParseTableProduction>*>& setOfSets,
                          size_t startIndex = 0u, bool compareLookAhead = false) noexcept;
};

} // namespace Generator
} // namespace SGParser

#endif // INC_SGPARSER_GENERATOR_PRODUCTION_H
