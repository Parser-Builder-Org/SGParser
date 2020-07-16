// Filename:  ParseTable.cpp
// Content:   Parse Table class implementation
// Provided AS IS under MIT License; see LICENSE file in root folder.

#include "ParseTable.h"

using namespace SGParser;

// ***** Parse Table class implementation

// Creates a DFA using a static DFA structure
// Parse table data is cleared before the creation and will remain
// clear in case of exceptions (basic exception safety is provided)
void ParseTable::Create(const StaticParseTable& staticTable) {
    // Free the existing data
    Destroy();

    // Try to reserve needed space
    ActionTable.reserve(staticTable.ActionHeight);
    GotoTable.reserve(staticTable.GotoHeight);
    ReduceProductions.reserve(staticTable.ProductionCount);
    NonTerminals.reserve(staticTable.NonTerminalCount);
    Terminals.reserve(staticTable.TerminalCount);
    StateInfos.reserve(staticTable.StateInfoCount);

    // Temporary container for swap-initialization of ProductionErrorTerminals
    decltype(ProductionErrorTerminals) newProductionErrorTerminals;
    // Initialize the temporary terminals container
    for (size_t i = 0u; i < staticTable.ProductionErrorTerminalCount; ++i)
        newProductionErrorTerminals[staticTable.pProductionErrorTerminals[i * 2u]] =
            staticTable.pProductionErrorTerminals[i * 2u + 1u];

    // From this point we can (safely) initialize the actual data

    // Initialize the action table
    auto actionTable = staticTable.pActionTable;
    for (size_t i = 0u; i < staticTable.ActionHeight; ++i, actionTable += staticTable.ActionWidth)
        ActionTable.push_back(actionTable);
    ActionWidth = staticTable.ActionWidth;

    // Initialize the goto table
    auto gotoTable = staticTable.pGotoTable;
    for (size_t i = 0u; i < staticTable.GotoHeight; ++i, gotoTable += staticTable.GotoWidth)
        GotoTable.push_back(gotoTable);
    GotoWidth = staticTable.GotoWidth;

    // Initialize the reduce productions table
    for (size_t i = 0u; i < staticTable.ProductionCount; ++i) {
        ReduceProductions.emplace_back();
        auto& reduceProduction             = ReduceProductions.back();
        reduceProduction.Length            = staticTable.pReduceProductions[i * 4u];
        reduceProduction.Left              = staticTable.pReduceProductions[i * 4u + 1u];
        reduceProduction.NotReported       = staticTable.pReduceProductions[i * 4u + 2u];
        reduceProduction.ErrorTerminalFlag = staticTable.pReduceProductions[i * 4u + 3u];
    }

    // Initialize the nonterminals
    for (size_t i = 0u; i < staticTable.NonTerminalCount; ++i) {
        NonTerminals.emplace_back();
        NonTerminals.back().StartState = staticTable.pNonTerminals[i];
    }

    // Initialize the terminals
    for (size_t i = 0u; i < staticTable.TerminalCount; ++i) {
        Terminals.emplace_back();
        Terminals.back().ErrorTerminal = staticTable.pTerminals[i];
    }

    // Initialize the terminals
    for (size_t i = 0u; i < staticTable.StateInfoCount; ++i) {
        StateInfos.emplace_back();
        auto& stateInfo            = StateInfos.back();
        stateInfo.Record           = staticTable.pStateInfos[i * 2u];
        stateInfo.BacktrackOnError = staticTable.pStateInfos[i * 2u + 1u];
    }

    // Swap-initialize the terminals
    ProductionErrorTerminals.swap(newProductionErrorTerminals);

    // Assign the table type
    Type         = staticTable.Type;
    StaticFlag   = true;
    InitialState = 0u;
}


// Destroys the parse table data
void ParseTable::Destroy() noexcept {
    FreeTables();
    Terminals.clear();
    NonTerminals.clear();
    StateInfos.clear();
    ProductionErrorTerminals.clear();
    Type         = TableType::None;
    InitialState = InvalidState;
}


void ParseTable::FreeTables() noexcept {
    // Free the action table if we own it
    if (!StaticFlag && !ActionTable.empty())
        delete[] ActionTable[0u];
    ActionTable.clear();

    // Free the goto table if we own it
    if (!StaticFlag && !GotoTable.empty())
        delete[] GotoTable[0u];
    GotoTable.clear();

    // Reset data
    ActionWidth  = 0u;
    GotoWidth    = 0u;
    StaticFlag   = false;
}
