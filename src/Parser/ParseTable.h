// Filename:  ParseTable.h
// Content:   Parse Table class header file with declarations
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_PARSETABLE_H
#define INC_SGPARSER_PARSETABLE_H

#include "SGString.h"
#include "ParseTableType.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace SGParser
{

// ***** ParseTable class

// ParseTable is used in Parse to figure out what actions
// to perform on certain input terminal (shift, reduce, accept).
class ParseTable
{
public:
    // Action mask should be applied by the parser on a value
    // returned by GetAction in order to determine what to do next
    enum ActionMask : unsigned
    {
        ShiftMask   = 0x0000'8000,
        ReduceMask  = 0x0000'4000,
        // Accept value, special case of reduce
        AcceptValue = 0x0000'4FFF,
        // Any of above
        ActionMask  = 0x0000'C000,

        // Mask used to extract a state to go to
        // or a production to reduce by
        ExtractMask = 0x0000'3FFF
    };
    
    // Concise array of productions
    // This is needed for reduce actions
    struct ReduceProduction final
    {
        // Number to pop off stack on reduce
        // Corresponds to number of items on the right hand side of production
        uint32_t Length            : 8;
        // Nonterminal to look up in goto, (left hand side of production)
        uint32_t Left              : 22;
        // Do not report the production to the user
        uint32_t NotReported       : 1;
        // Set if this reduce production has possible named error actions
        uint32_t ErrorTerminalFlag : 1;
    };

    // Structure for every nonterminal
    struct NonTerminal final
    {
        // Start state if we can start with this nonterminal, -1 otherwise
        uint16_t StartState;
    };

    // Structure for every terminal
    struct Terminal final
    {
        // Whether its a error terminal or not
        uint8_t ErrorTerminal      : 1;
    };

    // Structure for every state
    struct StateInfo final 
    {
        // Record flag
        uint8_t Record             : 1;
        // Whether to backtrack on error in this state
        uint8_t BacktrackOnError   : 1;
    };

    // Const for invalid state representation
    static constexpr unsigned InvalidState = unsigned(-1);

public:
    // *** Constructors & destructor

    // Make an empty parse table
    ParseTable() = default;

    // Creates a DFA using a static DFA structure
    explicit ParseTable(const struct StaticParseTable& staticTable) { Create(staticTable); }

    // No copy/move allowed
    ParseTable(const ParseTable&)                = delete;
    ParseTable(ParseTable&&) noexcept            = delete;
    ParseTable& operator=(const ParseTable&)     = delete;
    ParseTable& operator=(ParseTable&&) noexcept = delete;

    // Destructor
    ~ParseTable() { Destroy(); }

    // Creates a DFA using a static DFA structure
    // Parse table data is cleared before the creation and will remain
    // clear in case of exceptions (basic exception safety is provided)
    void Create(const StaticParseTable& staticTable);

    // Destroys the parse table data
    void Destroy() noexcept;

    // Return true if parse table is usable
    bool IsValid() const noexcept { return Type != ParseTableType::None; }

    // ***** Parser Interface

    // Returns initial state for parser stack
    unsigned GetInitialState() const noexcept { return InitialState; }

    // Get state from action table
    // Have to apply ActionMasks to figure out what to do
    unsigned GetAction(unsigned state, unsigned terminal) const {
        SG_ASSERT(state < ActionTable.size() && terminal < ActionWidth);
        return unsigned(int16_t(ActionTable[state][terminal]));
    }

    // Information
    size_t GetStateCount() const noexcept       { return ActionTable.size(); }
    size_t GetTerminalCount() const noexcept    { return ActionWidth; }
    size_t GetNonTerminalCount() const noexcept { return GotoWidth; }

    // Handling Reduce
    // Have to mask out action with ExtractMask !!!

    // Get new parser state after reduce
    unsigned GetReduceState(unsigned state, unsigned action) const {
        SG_ASSERT(state < GotoTable.size() &&
                  action < ReduceProductions.size() &&
                  ReduceProductions[action].Left < GotoWidth);
        return unsigned(int16_t(GotoTable[state][ReduceProductions[action].Left]));
    }
    
    unsigned GetLeftReduceState(unsigned state, unsigned left) const {
        SG_ASSERT(state < GotoTable.size() && left < GotoWidth);
        return unsigned(int16_t(GotoTable[state][left]));
    }

    // Get number of symbols to pop off stack on reduce by certain action
    unsigned GetReduceActionPopSize(unsigned action) const {
        SG_ASSERT(action < ReduceProductions.size());
        return unsigned(ReduceProductions[action].Length);
    }

    ReduceProduction GetReduceProduction(unsigned prodId) const {
        SG_ASSERT(prodId < ReduceProductions.size());
        return ReduceProductions[prodId];
    }

    // Returns start state given a nonterminal, or -1 for none
    unsigned GetStartState(unsigned nonTerminal) const {
        return nonTerminal < NonTerminals.size()
                   ? unsigned(NonTerminals[nonTerminal].StartState)
                   : InvalidState;
    }

    // Information about our symbols & states
    std::vector<NonTerminal> NonTerminals;
    std::vector<Terminal>    Terminals;
    std::vector<StateInfo>   StateInfos;
    // Map from (prodId | Nonterminal<<16)  to ErrorTerminal, if any
    // Should be checked if ErrorTerminal flag in ReduceProduction is set
    std::unordered_map<unsigned, unsigned> ProductionErrorTerminals;

protected:
    // Type of table (LR, LALR, CLR)
    ParseTableType Type    = ParseTableType::None;
    // State to start parsing with
    unsigned  InitialState = InvalidState;

    // Action Table
    // A vector of pointers to arrays of uint16_t which ... [state][terminal]
    // Pointers inside the vector are owning only when StaticFlag == false
    size_t    ActionWidth  = 0u;
    std::vector<uint16_t*> ActionTable;

    // Goto Table
    // A vector of pointers to arrays of uint16_t which ... [state][nonterminal]
    // Pointers inside the vector are owning only when StaticFlag == false
    size_t    GotoWidth    = 0u;
    std::vector<uint16_t*> GotoTable;

    // Current static state
    // If true than the parse table is using static data
    bool      StaticFlag   = false;

    // This array is consulted on reduce action
    std::vector<ReduceProduction> ReduceProductions;

    // Frees tables
    void FreeTables() noexcept;
};


// *** Static Parse table

// Used for hard-coded parse table
struct StaticParseTable final
{
    // Parse table type
    ParseTableType Type;
    // Action table data
    size_t    ActionHeight;
    size_t    ActionWidth;
    uint16_t* pActionTable;
    // Goto table data
    size_t    GotoHeight;
    size_t    GotoWidth;
    uint16_t* pGotoTable;
    // Reduce production list
    size_t    ProductionCount;
    uint32_t* pReduceProductions;
    // Information about our symbols & states
    size_t    NonTerminalCount;
    uint16_t* pNonTerminals;

    size_t    TerminalCount;
    uint8_t*  pTerminals;

    size_t    StateInfoCount;
    uint8_t*  pStateInfos;
    // Production error terminal count
    size_t    ProductionErrorTerminalCount;
    uint32_t* pProductionErrorTerminals;
};

} // namespace SGParser

#endif // INC_SGPARSER_PARSETABLE_H
