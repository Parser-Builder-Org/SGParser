// Filename:  ParseTableGen.h
// Content:   Parse Table Generator class header file with declarations
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_GENERATOR_PARSETABLEGEN_H
#define INC_SGPARSER_GENERATOR_PARSETABLEGEN_H

#include "ParseTable.h"

namespace SGParser
{
namespace Generator
{

// ***** ParseTable class

// ParseTable is used in Parse to figure out what actions
// to perform on certain input terminal (shift, reduce, accept).
class ParseTableGen final : public ParseTable
{
public:
    // *** Constructors

    // Make an empty parse table
    ParseTableGen() = default;

    // Make parse table out of grammar
    explicit ParseTableGen(class Grammar& grammar, 
                           ParseTableType tableType = ParseTableType::CLR) {
        // Create a new parse table
        Create(grammar, tableType);
    }

    // Initialize the ParseTable out of grammar
    bool     Create(Generator::Grammar& grammar, ParseTableType tableType = ParseTableType::CLR);

    // Create a static parse table structure
    bool     CreateStaticParseTable(String& str, const String& name,
                                    const String& namespaceName = String{}) const;

private:
    friend class Grammar;

    // Marker for empty goto table slot (used in table construction)
    static constexpr uint16_t EmptyGoto = uint16_t(-1);

    // Get reference to Action & Goto entries; for building the table
    uint16_t& GetActionRef(unsigned state, unsigned terminal) {
        SG_ASSERT(state < ActionTable.size() && terminal < ActionWidth);
        return ActionTable[state][terminal];
    }

    // Get reference to action slot
    uint16_t& GetGotoRef(unsigned state, unsigned nonTerminal) {
        SG_ASSERT(state < GotoTable.size() && nonTerminal < GotoWidth);
        return GotoTable[state][nonTerminal];
    }

    // Internal function used on table creation, allocates empty tables
    void      AllocateTables(size_t stateCount, size_t terminalCount, size_t nonTerminalCount);
};

} // namespace Generator
} // namespace SGParser

#endif // INC_SGPARSER_GENERATOR_PARSETABLEGEN_H
