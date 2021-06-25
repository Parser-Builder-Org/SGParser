// Filename:  ParseTableGen.cpp
// Content:   Parse Table Generator class implementation
// Provided AS IS under MIT License; see LICENSE file in root folder.

#include "ParseTableGen.h"
#include "Grammar.h"

#include <map>

namespace SGParser
{
namespace Generator
{

// *** Parse Table Generator class implementation

// Initializes the parse table
bool ParseTableGen::Create(Generator::Grammar& grammar, ParseTableType tableType) {
    return grammar.MakeParseTable(*this, tableType);
}


// *** Table creation interface functions

void ParseTableGen::AllocateTables(size_t stateCount, size_t terminalCount,
                                   size_t nonTerminalCount) {
    FreeTables();

    // *** Allocate memory for the action table

    ActionWidth = terminalCount;
    ActionTable.reserve(stateCount);

    const auto newActionSize  = stateCount * ActionWidth;
    auto       newActionTable = new uint16_t[newActionSize];
    std::fill(newActionTable, newActionTable + newActionSize, uint16_t(0u));

    for (size_t i = 0u; i < stateCount; ++i, newActionTable += ActionWidth)
        ActionTable.push_back(newActionTable);

    // *** Allocate memory for the goto table

    GotoWidth = nonTerminalCount;
    GotoTable.reserve(stateCount);

    const auto newGotoSize  = stateCount * GotoWidth;
    auto       newGotoTable = new uint16_t[newGotoSize];
    std::fill(newGotoTable, newGotoTable + newGotoSize, EmptyGoto);

    for (size_t i = 0u; i < stateCount; ++i, newGotoTable += GotoWidth)
        GotoTable.push_back(newGotoTable);
}


// Create a static parse table structure
bool ParseTableGen::CreateStaticParseTable(String& str, const String& name,
                                           const String& namespaceName) const {
    // The ParseTable must be valid
    if (!IsValid())
        return false;

    // Row size (# of elements)
    static constexpr size_t actionRowCount = 1u;
    static constexpr size_t gotoRowCount   = 1u;
    static constexpr size_t rpRowCount     = 10u;
    static constexpr size_t ntRowCount     = 10u;
    static constexpr size_t tRowCount      = 10u;
    static constexpr size_t siRowCount     = 10u;

    // Data structure sizes
    const auto actionHeight = ActionTable.size();
    const auto gotoHeight   = GotoTable.size();
    const auto rpSize       = ReduceProductions.size();
    const auto ntSize       = NonTerminals.size();
    const auto tSize        = Terminals.size();
    const auto siSize       = StateInfos.size();
    const auto petSize      = ProductionErrorTerminals.size();

    // Store the height and width into a tree
    const auto actionHeightStr = StringFromNumber(actionHeight);
    const auto actionWidthStr  = StringFromNumber(ActionWidth);
    const auto gotoHeightStr   = StringFromNumber(gotoHeight);
    const auto gotoWidthStr    = StringFromNumber(GotoWidth);
    const auto rpSizeStr       = StringFromNumber(rpSize);
    const auto ntSizeStr       = StringFromNumber(ntSize);
    const auto tSizeStr        = StringFromNumber(tSize);
    const auto siSizeStr       = StringFromNumber(siSize);
    const auto petSizeStr      = StringFromNumber(petSize);

    String dest;

    // *** Add the includes

    dest += "#include \"ParseTableType.h\"\n\n#include <cstdint>\n\n";

    // *** Add namespace declaration if needed

    if (!namespaceName.empty())
        dest += "namespace " + namespaceName + "\n{\n\n";

    // *** Add the Action table

    dest += "static uint16_t " + name + "_ActionTable[" +
            actionHeightStr + "][" + actionWidthStr +"] =\n{\n";

    // Go through all the transitions and add them
    for (size_t h = 0u; h < actionHeight; ++h) {
        dest += "    {";
        auto sep = "";
        for (size_t w = 0u; w < ActionWidth; ++w) {
            dest += sep + StringWithFormat("0x%04X", unsigned(ActionTable[h][w]));
            sep = ", ";
        }
        dest += "}";

        if ((h + 1u) % actionRowCount == 0u)
            dest += h != actionHeight - 1u ? ",\n" : "\n";
        else
            dest += h != actionHeight - 1u ? "," : "\n";
    }
    dest += "};\n\n";

    // *** Add the Goto table

    dest += "static uint16_t " + name + "_GotoTable[" +
            gotoHeightStr + "][" + gotoWidthStr + "] =\n{\n";

    // Go through all the transitions and add them
    for (size_t h = 0u; h < gotoHeight; ++h) {
        dest += "    {";
        auto sep = "";
        for (size_t w = 0u; w < GotoWidth; ++w) {
            dest += sep + StringWithFormat("0x%04X", unsigned(GotoTable[h][w]));
            sep = ", ";
        }
        dest += "}";

        if ((h + 1u) % gotoRowCount == 0u)
            dest += h != gotoHeight - 1u ? ",\n" : "\n";
        else
            dest += h != gotoHeight - 1u ? "," : "\n";
    }
    dest += "};\n\n";

    if (rpSize > 0u) {
        // *** Add the Reduce production table

        dest += "static uint32_t " + name + "_ReduceProduction[" + rpSizeStr + "][4] =\n{\n    ";

        size_t i = 1u;
        // Go through all the accept states and add them
        for (const auto [length, left, notReported, errorTerminalFlag] : ReduceProductions) {
            dest += StringWithFormat("{%u, %u, %u, %u}", unsigned(length), unsigned(left),
                                     unsigned(notReported), unsigned(errorTerminalFlag));

            if (i % rpRowCount == 0u)
                dest += i != rpSize ? ",\n    " : "\n";
            else
                dest += i != rpSize ? ", " : "\n";
            ++i;
        }
        dest += "};\n\n";
    }

    if (ntSize > 0u) {
        // *** Add the Non Terminals

        dest += "static uint16_t " + name +"_Nonterminals[" + ntSizeStr + "] =\n{\n    ";

        size_t i = 1u;
        // Go through all the accept states and add them
        for (const auto [startState]: NonTerminals) {
            dest += StringWithFormat("0x%04X", unsigned(startState));

            if (i % ntRowCount == 0u)
                dest += i != ntSize ? ",\n    " : "\n";
            else
                dest += i != ntSize ? ", " : "\n";
            ++i;
        }
        dest += "};\n\n";
    }

    if (tSize > 0u) {
        // *** Add the Terminals

        dest += "static uint8_t " + name + "_Terminals[" + tSizeStr + "] =\n{\n    ";

        size_t i = 1u;
        // Go through all the accept states and add them
        for (const auto [errorTerminal]: Terminals) {
            dest += StringWithFormat("%u", unsigned(errorTerminal));

            if (i % tRowCount == 0u)
                dest += i != tSize ? ",\n    " : "\n";
            else
                dest += i != tSize ? ", " : "\n";
            ++i;
        }
        dest += "};\n\n";
    }

    if (siSize > 0u) {
        // *** Add the StateInfos

        dest += "static uint8_t " + name + "_StateInfos[" + siSizeStr + "][2] =\n{\n    ";

        size_t i = 1u;
        // Go through all the accept states and add them
        for (const auto [record, backtrackOnError]: StateInfos) {
            dest += StringWithFormat("{%u, %u}", unsigned(record), unsigned(backtrackOnError));

            if (i % siRowCount == 0u)
                dest += i != siSize ? ",\n    " : "\n";
            else
                dest += i != siSize ? ", " : "\n";
            ++i;
        }
        dest += "};\n\n";
    }

    if (petSize > 0u) {
        // *** Add the Production Error Terminals

        dest += "static uint32_t " + name + "_ProductionErrorTerminals[" +
                petSizeStr + "][2] =\n{\n    ";

        // Create an ordered map from (unordered) ProductionErrorTerminals
        // only for the sake of pretty looking output
        std::map<unsigned, unsigned> sortedProdErrorTerminals{ProductionErrorTerminals.begin(),
                                                              ProductionErrorTerminals.end()};
        size_t i = 1u;
        // Go through all the accept states and add them
        for (const auto& [prodId, errorTerminal]: sortedProdErrorTerminals) {
            dest += StringWithFormat("{%u, %u}", prodId, errorTerminal);

            if (i % rpRowCount == 0u)
                dest += i != rpSize ? ",\n    " : "\n";
            else
                dest += i != rpSize ? ", " : "\n";
            ++i;
        }
        dest += "};\n\n";
    }

    // *** Add the StaticParseTable structure

    dest += "static SGParser::StaticParseTable " + name + " =\n{\n    ";

    // The table type
    dest += "SGParser::ParseTableType::";
    switch (Type) {
        // Set initially, no table
        case ParseTableType::None:
            dest += "None";
            break;
            // LR(1) parsing table
        case ParseTableType::LR:
            dest += "LR";
            break;
            // LALR(1) parsing table
        case ParseTableType::LALR:
            dest += "LALR";
            break;
            // Compacted LR(1) parsing table (similar to LALR,
            // but won't generate erroneous reduce-reduce conflicts)
        case ParseTableType::CLR:
            dest += "CLR";
            break;
    }
    dest += ",\n    ";

    // Action table entry
    dest += actionHeightStr + "u,\n    " + actionWidthStr + "u,\n    " +
            name + "_ActionTable[0u],\n    ";

    // Goto table entry
    dest += gotoHeightStr + "u,\n    " + gotoWidthStr + "u,\n    " +
            name + "_GotoTable[0u],\n    ";

    // Reduce Production entry
    dest += rpSizeStr + "u,\n    ";
    if (rpSize > 0u)
        dest += name + "_ReduceProduction[0u],\n    ";
    else
        dest += "nullptr,\n    ";

    // NonTerminal entry
    dest += ntSizeStr + "u,\n    ";
    if (ntSize > 0u)
        dest += name + "_Nonterminals,\n    ";
    else
        dest += "nullptr,\n    ";

    // Terminal entry
    dest += tSizeStr + "u,\n    ";
    if (tSize > 0u)
        dest += name + "_Terminals,\n    ";
    else
        dest += "nullptr,\n    ";

    // StateInfo entry
    dest += siSizeStr + "u,\n    ";
    if (siSize > 0u)
        dest += name + "_StateInfos[0u],\n    ";
    else
        dest += "nullptr,\n    ";

    // Production Error Terminal entry
    dest += petSizeStr + "u,\n    ";
    if (petSize > 0u)
        dest += name + "_ProductionErrorTerminals[0u]\n    };";
    else
        dest += "nullptr\n};\n";

    // Close namespace declaration of needed
    if (!namespaceName.empty())
        dest += "\n} // namespace " + namespaceName + "\n";

    str.swap(dest);

    return true;
}

} // namespace Generator
} // namespace SGParser
