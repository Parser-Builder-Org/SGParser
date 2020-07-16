// Filename:  Grammar.cpp
// Content:   Grammar class implementation for the Parser
// Provided AS IS under MIT License; see LICENSE file in root folder.
//
// Exhaustive canonical set search in MakeParseTable can probably be replaced by hash table,
// for a substantial speed increase.

#include "Grammar.h"
#include "Lex.h"

using namespace SGParser;
using namespace Generator;

// ***** Grammar class

// *** Setup/Cleanup

// Makes a grammar out of a grammar symbols & productions
// Start symbol is obtained from first production
void Grammar::Create(const std::map<String, unsigned>& symbolMap,
                     std::vector<Production>& prodList) {
    // Clears the grammar if it is already valid
    if (ProductionCount || StartSymbols.size())
        Clear();
    // Adds the grammar data
    AddGrammarSymbols(symbolMap);
    AddProductions(prodList);
}

void Grammar::Create(const std::map<String, unsigned>& symbolMap,
                     std::vector<Production>& prodList,
                     const std::map<unsigned, TerminalPrec>& prec) {
    // Clears the grammar if it is already valid
    if (ProductionCount || StartSymbols.size())
        Clear();
    // Adds the grammar data
    SetPrecedence(prec);
    AddGrammarSymbols(symbolMap);
    AddProductions(prodList);
}


// Creates [Accept]->Nonterminal productions for starting nonterminals
// Must be called before any productions are added
void Grammar::CreateAcceptingProductions(unsigned defaultLeft) {
    // Helper lambda function
    const auto createProduction = [&](const auto str, unsigned id, unsigned* right) {
        const auto left        = id | ProductionMask::AcceptingNonTerminal;
        const auto pacceptProd = new Production{str, left, right, 1u};
        pacceptProd->Id        = id;
        Productions[left].push_back(ParseTableProduction(pacceptProd));
        GrammarSymbols[str] = left;
        GrammarSymbolList.push_back(left);
        ++ProductionCount;
    };

    if (const auto size = unsigned(StartSymbols.size()); size > 0u) {
        // Create an accepting production for each start symbol
        for (unsigned i = 0u; i < size; ++i)
            createProduction(StringWithFormat("[Accept%u]", StartSymbols[i]), i,
                             &StartSymbols[i]);
    } else {
        // If no starting symbols were provided, use defaultLeft
        // and add a beginning production
        createProduction("[Accept]", 0u, &defaultLeft);
    }

    // Add the EOF token to the grammar symbol map
    GrammarSymbols["%EOF"] = TokenCode::TokenEOF | ProductionMask::Terminal;
    GrammarSymbolList.push_back(TokenCode::TokenEOF | ProductionMask::Terminal);
}


// *** Grammar Data interface

// Clears all productions & symbols
void Grammar::Clear() {
    GrammarSymbols.clear();
    GrammarSymbolList.clear();
    ClearProductions();
    ProductionCount = 0u;
    StartSymbols.clear();
    Precedence.clear();
    DebugData.Clear();
}


// Delete all the productions that were allocated
void Grammar::ClearProductions() {
    for (const auto& [_, prodVec]: Productions)
        for (const auto& prod: prodVec)
            delete prod.pProduction;
    Productions.clear();
}


// Adds a grammar symbol
// Symbol can be a terminal or nonterminal based on ProductionMask::Terminal bit
void Grammar::SetPrecedence(const std::map<unsigned, TerminalPrec>& prec) {
    Precedence = prec;
}


// Adds a production
void Grammar::AddProduction(Production& prod) {
    if (!ProductionCount)
        CreateAcceptingProductions(prod.Left);
    AddProductionImpl(prod);
}


// Adds multiple productions
void Grammar::AddProductions(std::vector<Production>& prodList) {
    if (!prodList.empty()) {
        if (!ProductionCount)
            CreateAcceptingProductions(prodList[0].Left);

        for (auto& prod: prodList)
            AddProductionImpl(prod);
    }
}


// Implementation helper for adding a production
void Grammar::AddProductionImpl(Production& prod) {
    // If there are other productions with the same label, assign them the same if
    for (const auto leftProd: prod.LeftChain)
        if (Productions.find(leftProd) != Productions.end())
            for (auto prodElem: Productions[leftProd])
                if (prodElem.pProduction->Name == prod.Name) {
                    // Found id
                    prod.Id = prodElem.pProduction->Id;
                    goto prod_id_found;
                }

    prod.Id = unsigned(ProductionCount++);
prod_id_found:
    // This will automatically create an element if needed
    const auto pnewprod = new Production{prod};
    Productions[prod.Left].push_back(ParseTableProduction(pnewprod));
}


// Adds a grammar symbol
// Symbol can be a terminal or nonterminal based on ProductionMask::Terminal bit
void Grammar::AddGrammarSymbol(const String& str, unsigned value) {
    if (GrammarSymbols.find(str) != GrammarSymbols.end()) {
        GrammarSymbols[str] = value;
        // There may have to be a test for 'presence' here
        GrammarSymbolList.push_back(value);
    }
}


// Adds multiple grammar symbols
void Grammar::AddGrammarSymbols(const std::map<String, unsigned>& symbolMap) {
    // Add grammar symbols directly
    for (const auto& [symbol, value]: symbolMap) {
        GrammarSymbols[symbol] = value;
        // There may have to be a test for 'presence' here
        GrammarSymbolList.push_back(value);
    }
}


void Grammar::SetStartSymbols(const std::vector<unsigned>& startSymbols) {
    StartSymbols = startSymbols;
}


// Creates inverse symbols table
void Grammar::CreateInverseSymbols(std::map<unsigned, const String*>& grammarSymbolsInv) const {
    grammarSymbolsInv.clear();

    for (const auto& [symbol, value]: GrammarSymbols)
        grammarSymbolsInv[value] = &symbol;
}


// *** Item Set Closure and Goto operations

// Closure adds productions for all nonterminals directly after the dot,
// until the terminal is hit
// Refer to p. 222, Ullman
void Grammar::Closure(std::vector<ParseTableProduction>& closure) const {
    // Keep a set of productions we have looked at so far
    // we may have to examine a few several times (if their lookahead changed) 
    // in order propagate lookahead correctly
    std::set<unsigned>    lookAtSet;
    std::vector<unsigned> lookAt;
    lookAt.reserve(closure.size());

    for (unsigned i = 0u; i < unsigned(closure.size()); ++i) {
        lookAtSet.insert(i);
        lookAt.push_back(i);
    }

    // For each Productions in the set not already expanded...
    for (size_t i = 0u; i < lookAt.size(); ++i) {
        // Production we are working with (note that we may have to update pointer on change)
        auto ptableProduction = &closure[lookAt[i]];

        lookAtSet.erase(lookAt[i]);

        // If the Dot is at the end of production's RHS, skip it
        if (ptableProduction->Dot >= ptableProduction->pProduction->Length)
            continue;

        // Nonterminal to be "expanded", comes right after the dot
        // Only expand nonterminals
        const auto nonTerminal = ptableProduction->pProduction->Right(ptableProduction->Dot);
        if (nonTerminal & ProductionMask::Terminal)
            continue;

        // Compute First([beta],a)
        std::vector<unsigned> symbols;
        std::set<unsigned>    terminalSet;

        // Store all remaining RHS symbols in a production (after this non-terminal), if any
        for (auto l = ptableProduction->Dot + 1u; l < ptableProduction->pProduction->Length; ++l)
            symbols.push_back(ptableProduction->pProduction->Right(l));

        // Calculate a first set
        if (First(terminalSet, symbols)) {
            // If First(symbols) derives an empty string add lookahead terminals to First terminal set
            terminalSet.insert(ptableProduction->LookAhead.begin(), ptableProduction->LookAhead.end());
        }

        // For each Production in the grammar that starts with the GrammarSymbol pRight
        // after the Dot...

        // For each production deriving this nonterminal
        // (whose Left is the same as our nonterminal after the dot)
        const auto  it          = Productions.find(nonTerminal);
        const auto& derivations = it != Productions.end()
                                      ? it->second 
                                      : decltype(Productions)::mapped_type{};
        for (const auto& der: derivations) {
            // Find a production with the same body in the set
            size_t j = 0u;
            for (; j < closure.size(); ++j) {
                if (closure[j].Dot > 0u)
                    continue;
                if (closure[j].pProduction->Left != der.pProduction->Left)
                    continue;
                if (!closure[j].pProduction->RHSEquals(*der.pProduction))
                    continue;
                break;
            }

            // If this production does not exist, add it
            if (j == closure.size()) {
                closure.push_back(der);
                auto& clos = closure.back();
                clos.Dot   = 0u;
                clos.LookAhead.clear();
                lookAt.push_back(unsigned(j));
                lookAtSet.insert(unsigned(j));
                // Adding may have changed our pointer
                ptableProduction = &closure[lookAt[i]];

                // Merge the look-aheads for the production
                // Note: at this point, j points to our production
                clos.LookAhead.insert(terminalSet.begin(), terminalSet.end());
            } else {
                // Merge the lookahead's for the production
                for (const auto iTerminal: terminalSet)
                    if (closure[j].LookAhead.find(iTerminal) == closure[j].LookAhead.end()) {
                        // If we modified a production, record it in lookAt array
                        // because we might have to update items it depended on
                        closure[j].LookAhead.insert(iTerminal);
                        if (lookAtSet.find(unsigned(j)) == lookAtSet.end()) {
                            lookAtSet.insert(unsigned(j));
                            lookAt.push_back(unsigned(j));
                        }
                    }
            }
        }
    }
}


// Compute Goto item set on symbol, given an existing item set
// Refer to p. 224, Ullman
void Grammar::Goto(std::vector<ParseTableProduction>& gotoSet,
                   const std::vector<ParseTableProduction>& itemSet, unsigned symbol) const {
    gotoSet.clear();
    // gotoIndex's used to access end of gotoSet std::vector
    size_t gotoIndex = 0u;

    // Look for all productions of the form A -> [alpha].[symbol][beta], and add them
    // to the goto set after shifting the Dot pRight by one
    for (const auto& prod: itemSet)
        if (prod.Dot < prod.pProduction->Length)
            if (prod.pProduction->Right(prod.Dot) == symbol) {
                gotoSet.push_back(prod);
                ++gotoSet[gotoIndex].Dot;
                ++gotoIndex;
            }

    // Order productions, so we can compare them linearly
    std::sort(gotoSet.begin(), gotoSet.end());

    // Now compute the closure of the above set
    Closure(gotoSet);
}


// *** First & Follow set construction

// Returns true if the symbol can derive an empty production (epsilon)
// Refer to p. 189, Ullman
bool Grammar::First(std::set<unsigned>& terminalsFound, unsigned symbol, bool* mark) const {
    // Set if there are productions to epsilon
    bool derivesEmptyFlag = false;

    // If it's a terminal, the First set is just that symbol
    if (symbol & ProductionMask::Terminal) {
        terminalsFound.insert(symbol);
        return false;
    }

    // Check if there's a Productions to epsilon
    const auto  it         = Productions.find(symbol);
    const auto& production = it != Productions.end()
                                 ? it->second 
                                 : decltype(Productions)::mapped_type{};
    for (const auto& prod: production)
        if (prod.pProduction->Length == 0u)
            derivesEmptyFlag = true;

    // Compute First() for a nonterminal
    for (const auto& prod: production) {
        // If production was not yet processed
        if (!mark[prod.pProduction->Id]) {
            mark[prod.pProduction->Id] = true;

            for (unsigned j = 0u; j < prod.pProduction->Length; ++j) {
                // If a terminal, add it to a set and we are done
                if (prod.pProduction->Right(j) & ProductionMask::Terminal) {
                    terminalsFound.insert(prod.pProduction->Right(j));
                    break;
                } else {
                    // Nonterminal
                    // Add it's First set, if it cant be empty, break out
                    if (!First(terminalsFound, prod.pProduction->Right(j), mark))
                        break;
                }
            }
        }
    }

    return derivesEmptyFlag;
}


// Returns true if the symbol can derive an empty production (epsilon)
// Refer to p. 189, Ullman
bool Grammar::First(std::set<unsigned>& terminalsFound,
                    const std::vector<unsigned>& symbols) const {
    bool derivesEmptyFlag = true;

    // Mark productions we already looked at
    const auto mark = new bool[ProductionCount];
    std::fill(mark, mark + ProductionCount, false);

    // Go through all symbols, and add First for all productions
    // As long as the production derives an empty string
    for (const auto symbol: symbols)
        if (derivesEmptyFlag = First(terminalsFound, symbol, mark); !derivesEmptyFlag)
            break;

    delete[] mark;

    return derivesEmptyFlag;
}


// *** ParseTable generation routines

namespace SGParser::Generator {

// Custom conflict structure for Shift-Reduce and Reduce-Reduce
struct Conflict final
{
    unsigned              State;
    unsigned              RRFlag;
    unsigned              Item1;
    unsigned              ReduceLeft1;
    unsigned              Item2;
    unsigned              ReduceLeft2;
    unsigned              Terminal;
    std::vector<unsigned> ShiftIndexes;
    std::set<unsigned>    ShiftProductions;
};

// MakeTableData
struct MakeTableData final
{
    ParseTableGen&                                   Table;
    std::vector<uint16_t*>&                          ActionTable;
    std::vector<std::vector<ParseTableProduction>*>& CanonicalSet;
};

} // namespace SGParser::Generator


// Print the production array
size_t Grammar::PrintProductions(String& str, const std::vector<ParseTableProduction>& prodVec,
                                 unsigned specialTerminal, bool labels, bool lineNumbers,
                                 bool points) {
    size_t nameLength = 0u;
    String dest;

    // Display Shift productions
    // Calculate name length
    if (labels)
        for (const auto& prod: prodVec)
            if (prod.pProduction->Name.size() > nameLength)
                nameLength = prod.pProduction->Name.size();

    // And display productions
    for (const auto& prod: prodVec) {
        String tempStr = "    ";

        // Display Production line number
        if (lineNumbers)
            tempStr += StringWithFormat("Line %4zu: ", prod.pProduction->Line + 1u);

        // Display Production label
        if (labels) {
            tempStr += prod.pProduction->Name;
            const auto spaces = nameLength - prod.pProduction->Name.size();
            tempStr.append(spaces, ' ');
        }

        // And left hand side
        const auto  s   = tempStr + " " + *GrammarSymbolsInv[prod.pProduction->Left] + " -> ";
        auto        pos = prod.PrintRHS(tempStr, &GrammarSymbolsInv, !points);
        dest += s + tempStr;

        // If at the end, print lookahead
        if (prod.Dot == prod.pProduction->Length) {
            dest += "['";
            dest += *GrammarSymbolsInv[specialTerminal];
            dest += "']";
        }
        dest += "\n";

        if (points) {
            // And finally the '^' character
            pos += s.size();
            dest.append(pos, ' ');
            dest += "^\n";
        }
    }

    str.swap(dest);

    return nameLength;
}

// Calculates productions in which nonterminal is used before lookahead
void Grammar::GetNonterminalFollowProductions(const MakeTableData& td,
                                              std::vector<ParseTableProduction>& displayProds,
                                              unsigned nonTerminal,
                                              unsigned lookAheadTerminal) const {
    const auto la = lookAheadTerminal & ProductionMask::TerminalValue;

    // Set to ensure we don't add the same production (based on Id) more then once
    std::set<unsigned>    displayProdsIdSet;
    // Set of nonterminals to look at (if our nonterminal is used
    // in another one, we consider them too)
    std::set<unsigned>    leftSet;
    std::vector<unsigned> leftList;

    // Add nonterminal
    leftSet.insert(nonTerminal);
    leftList.push_back(nonTerminal);

    for (size_t isymbol = 0u; isymbol < leftList.size(); ++isymbol) {
        const auto leftSymbol = leftList[isymbol];

        // Find a state, that:
        //  1. Has an action on our terminal
        //  2. Has a dot after our production's Left symbol
        for (size_t state = 0u; state < td.CanonicalSet.size(); ++state) {
            // Must have an action on our lookahead
            if (td.ActionTable[state][la] == 0u)
                continue;

            for (const auto& prod: *td.CanonicalSet[state])
                if (prod.Dot > 0u &&
                    prod.pProduction->Right(prod.Dot - 1u) == leftSymbol &&
                    (prod.Dot != prod.pProduction->Length ||
                     prod.LookAhead.find(lookAheadTerminal) != prod.LookAhead.end())) {
                    // Found our state
                    if (td.ActionTable[state][la] & ParseTable::ReduceMask &&
                        td.ActionTable[state][la] != ParseTable::AcceptValue) {
                        const unsigned left = 
                            td.Table.ReduceProductions[td.ActionTable[state][la] & 
                                                       ~ParseTable::ReduceMask].Left;
                        if (leftSet.find(left) == leftSet.end()) {
                            leftSet.insert(left);
                            leftList.push_back(left);

                            if (displayProdsIdSet.find(prod.pProduction->Id) ==
                                displayProdsIdSet.end()) {
                                displayProdsIdSet.insert(prod.pProduction->Id);
                                displayProds.push_back(prod);
                            }
                        }
                    }
                    
                    // It must be a shift or accept
                    if (td.ActionTable[state][la] & ParseTable::ShiftMask) {
                        // Display this production (if something is following us)
                        auto dotPos = prod.Dot;

                        // Note that if nonterminal derives an empty string,
                        // we move the dot & try again
                        while (dotPos < prod.pProduction->Length) {
                            const auto sym = prod.pProduction->Right(dotPos);

                            if (sym & ProductionMask::Terminal) {
                                if ((sym & ProductionMask::TerminalValue) == la)
                                    if (displayProdsIdSet.find(prod.pProduction->Id) ==
                                        displayProdsIdSet.end()) {
                                        displayProdsIdSet.insert(prod.pProduction->Id);
                                        displayProds.push_back(prod);
                                        displayProds.back().Dot = dotPos;
                                    }
                            } else {
                                // Nonterminal
                                std::vector<unsigned> symbols;
                                std::set<unsigned>    firstSet;
                                symbols.push_back(prod.pProduction->Right(dotPos));
                                if (First(firstSet, symbols)) {
                                    // Should we deal with empty sets?
                                    ++dotPos;
                                    if (dotPos == prod.pProduction->Length) {
                                        // Accepting nonterminal is special - 
                                        // nothing can derive it, so stop
                                        if (prod.pProduction->Left &
                                            ProductionMask::AcceptingNonTerminal) {
                                            if (displayProdsIdSet.find(prod.pProduction->Id) ==
                                                displayProdsIdSet.end()) {
                                                displayProdsIdSet.insert(prod.pProduction->Id);
                                                displayProds.push_back(prod);
                                                displayProds.back().Dot = dotPos;
                                            }

                                        } else if (leftSet.find(prod.pProduction->Left) ==
                                                   leftSet.end()) {
                                            leftSet.insert(prod.pProduction->Left);
                                            leftList.push_back(prod.pProduction->Left);
                                        }
                                    }
                                    continue;
                                }
                                // Only add production if it's in the first set
                                if (firstSet.find(lookAheadTerminal) != firstSet.end())
                                    if (displayProdsIdSet.find(prod.pProduction->Id) ==
                                        displayProdsIdSet.end()) {
                                        displayProdsIdSet.insert(prod.pProduction->Id);
                                        displayProds.push_back(prod);
                                        displayProds.back().Dot = dotPos;
                                    }
                            }
                            break;
                        }
                    }
                }
        }
    }
}


// Prints conflict report for one conflict
void Grammar::PrintConflict(const MakeTableData& td, String& str, const Conflict& conflict) {
    String     dest;
    const auto conflictName = conflict.RRFlag ? "Reduce" : "Shift";
    const auto& stateItem   = *td.CanonicalSet[conflict.State];
    std::vector<ParseTableProduction> prodVec;

    const bool lineNumbers = !(DebugData.Flags & GrammarDebugData::ConflictReportNoLineNumbers);
    const bool labels      = !(DebugData.Flags & GrammarDebugData::ConflictReportNoLabels);
    const bool points      = !(DebugData.Flags & GrammarDebugData::ConflictReportNoPoints);

    // Store the header
    dest += StringWithFormat("State %u: %s-Reduce conflict on lookahead '%s'\n",
                             conflict.State, conflictName,
                             GrammarSymbolsInv[conflict.Terminal]->data());

    // *** First set (Shift or Reduce) productions

    if (conflict.RRFlag) {
        dest += "Reduce for " + *GrammarSymbolsInv[conflict.ReduceLeft1] + "\n";
        prodVec.push_back((*td.CanonicalSet[conflict.State])[conflict.Item1]);
        GetNonterminalFollowProductions(td, prodVec, conflict.ReduceLeft1, conflict.Terminal);
    } else {
        String shiftString;
        // Form a string for shift production
        for (const auto iSet: conflict.ShiftProductions) {
            if (!shiftString.empty())
                shiftString += ",";
            shiftString += *GrammarSymbolsInv[iSet];
        }
        dest += "Shift for " + shiftString + "\n";
        // Build the temporary array of production pointers
        prodVec.resize(conflict.ShiftIndexes.size());
        for (size_t iVec = 0u; iVec < conflict.ShiftIndexes.size(); ++iVec)
            prodVec[iVec] = stateItem[conflict.ShiftIndexes[iVec]];
    }

    // Display fist set (Shift or Reduce) productions
    PrintProductions(dest, prodVec, conflict.Terminal, labels, lineNumbers, points);

    // *** Second (conflicting) set productions - always Reduce

    dest += "Reduce for " + *GrammarSymbolsInv[conflict.ReduceLeft2] + "\n";
    prodVec.clear();
    prodVec.push_back((*td.CanonicalSet[conflict.State])[conflict.Item2]);

    GetNonterminalFollowProductions(td, prodVec, conflict.ReduceLeft2, conflict.Terminal);
    PrintProductions(dest, prodVec, conflict.Terminal, labels, lineNumbers, points);

    dest += "\n\n";

    str.swap(dest);
}


// Make ParseTable out of grammar
bool Grammar::MakeParseTable(ParseTableGen& table, ParseTable::TableType type) {
    // Check production & grammar symbol consistency
    if (!CheckProductions()) {
        GrammarSymbolsInv.clear();
        table.FreeTables();
        return false;
    }

    CreateInverseSymbols(GrammarSymbolsInv);

    // Find the array indexes of the maximum terminal and
    // maximum nonterminal to determine the widths of the tables
    unsigned maxTerminal    = 0u;
    unsigned maxNonTerminal = 0u;
    for (const auto sym: GrammarSymbolList)
        if (sym & ProductionMask::Terminal) {
            if ((sym & ProductionMask::TerminalValue) > maxTerminal)
                maxTerminal = sym & ProductionMask::TerminalValue;
        } else {
            if (!(sym & ProductionMask::AcceptingNonTerminal))
                if (sym > maxNonTerminal)
                    maxNonTerminal = sym;
        }

    ++maxTerminal;
    ++maxNonTerminal;
    // In case it's a screwy grammar with no terminals...
    if (maxTerminal < TokenCode::TokenEOF)
        maxTerminal = TokenCode::TokenFirstID;

    // Setup non-terminal lookup table
    table.NonTerminals.resize(maxNonTerminal);
    for (auto& nonTerm: table.NonTerminals)
        nonTerm.StartState = ParseTableGen::EmptyGoto;

    // And terminal lookup table
    table.Terminals.resize(maxTerminal);
    for (unsigned i = 0u; i < maxTerminal; ++i) {
        if (i == TokenCode::TokenError ||
            GrammarSymbolsInv.find(i|ProductionMask::Terminal|ProductionMask::ErrorTerminal) !=
            GrammarSymbolsInv.end())
            table.Terminals[i].ErrorTerminal = uint8_t(1u);
        else
            table.Terminals[i].ErrorTerminal = uint8_t(0u);
    }

    // Setup Production list in ParseTable
    table.ReduceProductions.resize(ProductionCount);
    for (auto& reduceProd: table.ReduceProductions)
        reduceProd.ErrorTerminalFlag = uint32_t(0u);

    for (const auto& [_, prodVec]: Productions)
        for (const auto& prod: prodVec) {
            table.ReduceProductions[prod.pProduction->Id].Left        = prod.pProduction->Left;
            table.ReduceProductions[prod.pProduction->Id].Length      = prod.pProduction->Length;
            table.ReduceProductions[prod.pProduction->Id].NotReported =
                prod.pProduction->NotReported;

            if (prod.pProduction->ErrorTerminal) {
                // If Error Terminal, mark this production as possibly throwing an error
                table.ReduceProductions[prod.pProduction->Id].ErrorTerminalFlag = uint32_t(1u);
                // And record error value in a map for corresponding nonterminal
                table.ProductionErrorTerminals[prod.pProduction->Id |
                                                (prod.pProduction->Left << 16u)] =
                    prod.pProduction->ErrorTerminal;
            }
        }

    // Construct the canonical set
    std::vector<std::vector<ParseTableProduction>*> canonicalSet;
    std::vector<uint16_t*>                          actionTable;
    std::vector<uint16_t*>                          gotoTable;
    // Keep track of 'source' states (that generated this state) for debug dump
    std::vector<std::set<unsigned>>                 sourceStates;

    // Start off by setting canonicalSet to Closure() starting productions
    const auto startCount = StartSymbols.size() ? unsigned(StartSymbols.size()) : 1u;
    unsigned   state;
    unsigned   newState;

    for (state = 0u; state < startCount; ++state) {
        const auto pgotoSet = new std::vector<ParseTableProduction>;
        // Calculate closure of this accepting production
        const auto& prod = Productions[state | ProductionMask::AcceptingNonTerminal][0u];
        pgotoSet->push_back(prod);
        (*pgotoSet)[0u].Dot = 0u;
        (*pgotoSet)[0u].LookAhead.clear();
        (*pgotoSet)[0u].LookAhead.insert(TokenCode::TokenEOF | ProductionMask::Terminal);
        Closure(*pgotoSet);
        // Add item, and space for its corresponding table entries
        canonicalSet.push_back(pgotoSet);
        actionTable.push_back(new uint16_t[maxTerminal]);
        std::fill(actionTable[state], actionTable[state] + maxTerminal, uint16_t(0u));
        gotoTable.push_back(new uint16_t[maxNonTerminal]);
        std::fill(gotoTable[state], gotoTable[state] + maxNonTerminal, uint16_t(-1));
        // Store starting state in nonterminal lookup table
        table.NonTerminals[prod.pProduction->Right(0u)].StartState = uint16_t(state);
        // Source state - no source
        sourceStates.emplace_back();
    }

    // Allocate goto std::vector ahead of time
    auto pgotoSet = new std::vector<ParseTableProduction>;

    for (state = 0u; state < canonicalSet.size(); ++state) {
        // For every symbol, both terminal & nonTerminal
        for (const auto symbol: GrammarSymbolList) {
            // Compute Goto(canonicalSet[state], Symbol)
            Goto(*pgotoSet, *canonicalSet[state], symbol);

            // The goto set better not be empty
            // Empty set would means no transition on this symbol, i.e. parse error
            if (!pgotoSet->size())
                continue;

            // If the Item is not there, make a new state for it
            unsigned searchIndex = 0u;
        find_next_productionset:
            newState = unsigned(ParseTableProduction::FindVectorInSetOfSets(*pgotoSet, 
                                    canonicalSet, searchIndex, type == ParseTable::TableType::LR));
            if (newState == ParseTable::InvalidState) {
                // Make a new state for the set
                newState = unsigned(canonicalSet.size());
                canonicalSet.push_back(pgotoSet);

                pgotoSet = new std::vector<ParseTableProduction>;
                // Save the source state
                std::set<unsigned> v;
                v.insert(state);
                sourceStates.push_back(v);
                // And allocate its tables
                actionTable.push_back(new uint16_t[maxTerminal]);
                std::fill(actionTable[newState], actionTable[newState] + maxTerminal, uint16_t(0u));
                gotoTable.push_back(new uint16_t[maxNonTerminal]);
                std::fill(gotoTable[newState], gotoTable[newState] + maxNonTerminal, uint16_t(-1));
            } else {
                auto& cSetItem = *canonicalSet[newState];

                // For CLR make sure we can merge states
                if (type == ParseTable::TableType::CLR) {
                    // Combining tables in LALR can cause reduce/reduce conflicts
                    // Detect conflict, and don't merge conflicting states

                    // Collect all terminals that can possibly lead to a 'reduce' action
                    // Not to cause conflicts, they should be unique with respect to
                    // nonterminal they generate on reduce
                    std::map<unsigned, unsigned> reduceTerminals;

                    // We do not check for local reduce-reduce conflicts here
                    for (const auto& setItem: cSetItem)
                        if (setItem.Dot == setItem.pProduction->Length)
                            for (const auto iTerminal: setItem.LookAhead)
                                reduceTerminals[iTerminal] = setItem.pProduction->Left;

                    // Now go through new state we are merging with the old one
                    // And make sure same letters can't generate a different production
                    for (const auto& prod: *pgotoSet)
                        if (prod.Dot == prod.pProduction->Length)
                            for (const auto iTerminal: prod.LookAhead) {
                                // Is this terminal present in a map?
                                if (reduceTerminals.find(iTerminal) != reduceTerminals.end()) {
                                    // If reduce on this terminal would not result in 
                                    // the same value, we got a problem

                                    // Note that this might also happen if original had 
                                    // a reduce-reduce conflict
                                    // because we don't check for it up front
                                    if (reduceTerminals[iTerminal] != prod.pProduction->Left) {
                                        searchIndex = newState + 1u;
                                        // Report a note
                                        if (Messages.GetMessageFlags() & 
                                            ParseMessageBuffer::MessageNote) {
                                            const auto msgtext = StringWithFormat(
                                                "Recovered from LALR combine state R/R conflict on state %u",
                                                newState);
                                            const ParseMessage msg{ParseMessage::NoteMessage,
                                                                   "Conflict Recovery", msgtext};
                                            Messages.AddMessage(msg);
                                        }
                                        goto find_next_productionset;
                                    }
                                }
                            }
                }

                if (type != ParseTable::TableType::LR) {
                    // Add another source state
                    sourceStates[newState].insert(state);

                    // Add new lookahead symbols to productions, if any
                    // We assume productions appear in the same order in both 
                    // cononicalSet item & gotoSet
                    for (size_t i = 0u; i < cSetItem.size(); ++i)
                        for (const auto terminal: (*pgotoSet)[i].LookAhead) {
                            auto nextState = newState;
                            auto pprod     = &cSetItem[i];

                            // If production does not yet have this lookahead
                            while (pprod->LookAhead.find(terminal) == pprod->LookAhead.end()) {
                                // Insert the lookahead terminal
                                pprod->LookAhead.insert(terminal);

                                // If dot is not at the end
                                // we need to store this lookahead into 
                                // all other productions that were
                                // originally derived from this one
                                if (pprod->Dot == pprod->pProduction->Length)
                                    break;

                                const auto nextSymbol = pprod->pProduction->Right(pprod->Dot);
                                // Use action table to figure out where 
                                // next symbol would lead us to
                                if (nextSymbol & ProductionMask::Terminal) {
                                    // Masks out the action flags
                                    if (nextState = (unsigned(actionTable[nextState][nextSymbol &
                                                     ProductionMask::TerminalValue]) &
                                                     ~ParseTable::ActionMask) & 0x0000'FFFFu;
                                        !nextState)
                                        break;
                                }
                                // Use the goto table to figure out where next
                                // symbol would lead us to
                                else
                                    if (nextState = gotoTable[nextState][nextSymbol];
                                        nextState == ParseTableGen::EmptyGoto)
                                        break;

                                // Get a list of productions for that state
                                auto& prodVec = *canonicalSet[nextState];

                                unsigned prodIndex = 0u;
                                // Find our production in this new state
                                for (; prodIndex < prodVec.size(); ++prodIndex)
                                    if (prodVec[prodIndex].pProduction->Left ==
                                        pprod->pProduction->Left &&
                                        prodVec[prodIndex].Dot == pprod->Dot + 1u &&
                                        prodVec[prodIndex].pProduction->RHSEquals(
                                            *pprod->pProduction))
                                        break;

                                // Production HAS to be there, since a symbol lead us to this state
                                pprod = &prodVec[prodIndex];
                            }
                        }
                }

                // Always keep gotoset clear
                pgotoSet->clear();
            }

            // *** Set Shift or Goto actions for the symbol

            if (symbol & ProductionMask::Terminal)
                // Set Shift state, if it's a terminal
                actionTable[state][symbol & ProductionMask::TerminalValue] =
                    uint16_t(newState | ParseTable::ShiftMask);
            else
                // Set Goto state for nonterminal
                if (!(symbol & ProductionMask::AcceptingNonTerminal))
                    gotoTable[state][symbol] = uint16_t(newState);
        }
    }

    // Resize state information std::vector
    table.StateInfos.resize(canonicalSet.size());

    // *** Set Reduce actions for the state

    std::vector<Conflict> conflicts;

    // This can not be done in a previous look for LALR,
    // because extra look ahead can be added to a state after it is processed
    for (state = 0u; state < canonicalSet.size(); ++state) {
        const auto& stateItem = *canonicalSet[state];

        // Initialize state information
        table.StateInfos[state].Record           = uint8_t(0u);
        table.StateInfos[state].BacktrackOnError = uint8_t(0u);

        // Keep a map of which item caused a reduce for each terminal
        // This way we can lookup previous item in case of R-R conflict
        std::map<unsigned, unsigned> terminalItems;

        for (unsigned i = 0u; i < stateItem.size(); ++i) {
            const auto& prod = stateItem[i];

            // If dot's in the end of production, set reduce action
            if (prod.Dot != prod.pProduction->Length) {
                const auto terminal = prod.pProduction->Right(prod.Dot);
                // Set record flag in stateinfo if state has a named error after the dot
                // Or if it's a try block
                if (terminal & ProductionMask::ErrorTerminal) {
                    if ((terminal & ProductionMask::TerminalValue) == TokenCode::TokenError) {
                        // Error with backtracking
                        if (terminal & ProductionMask::BacktrackError) {
                            table.StateInfos[state].Record           = uint8_t(1u);
                            table.StateInfos[state].BacktrackOnError = uint8_t(1u);
                        }
                    } else {
                        // Named error
                        table.StateInfos[state].Record = uint8_t(1u);
                    }
                }
                continue;
            }

            // Set To reduce by production on each Lookahead Symbol
            for (const auto iTerminal: prod.LookAhead) {
                const auto la        = iTerminal & ProductionMask::TerminalValue;
                auto&      actionRef = actionTable[state][la];

                if (actionRef == uint16_t(0u)) {
                set_reduce_action:
                    terminalItems[iTerminal] = i;
                    // If reducing by a start symbol, set Accept action
                    if (la == TokenCode::TokenEOF &&
                        (prod.pProduction->Left & ProductionMask::AcceptingNonTerminal))
                        actionRef = uint16_t(ParseTable::AcceptValue);
                    else  // otherwise, reduce as needed
                        actionRef = uint16_t(ParseTable::ReduceMask | prod.pProduction->Id);
                } else {
                    // Save actionRef value for report in case we change it
                    const auto oldItem = terminalItems[iTerminal];

                    // There may be an conflict action on this terminal
                    // If so, perform appropriate action
                    if (actionRef & ParseTable::ShiftMask) {
                        for (const auto& stateIt: stateItem) {
                            if (stateIt.Dot == stateIt.pProduction->Length)
                                continue;
                            if (stateIt.pProduction->Right(stateIt.Dot) == iTerminal) {
                                if (stateIt.pProduction->ConflictActions.find(stateIt.Dot) !=
                                    stateIt.pProduction->ConflictActions.end()) {
                                    // If there is an entry for this terminal
                                    auto& action = 
                                        stateIt.pProduction->ConflictActions[stateIt.Dot];
                                    if (action.Actions.find(prod.pProduction->Left) !=
                                        action.Actions.end()) {
                                        // Conflict action found
                                        if (action.Actions[prod.pProduction->Left] == 
                                            Production::ConflictAction::Action::Reduce)
                                            goto set_reduce_action;
                                        // Do nothing
                                        goto no_conflict_warning;
                                    }
                                }
                            }
                        }

                        // Check if precedence can resolve this conflict
                        if (Precedence.find(iTerminal) != Precedence.end()) {
                            const auto& prec    = Precedence[iTerminal];
                            const auto  precVal = prec.Value & TerminalPrec::PrecMask;
                            const auto  assoc   = TerminalPrec::Assoc(prec.Value & 
                                                                      TerminalPrec::AssocMask);

                            // If token precedence's greater then production, then shift
                            if (precVal > prod.pProduction->Precedence)
                                goto no_conflict_warning;
                            // If token precedence's smaller then production, then reduce
                            else if (precVal < prod.pProduction->Precedence)
                                goto set_reduce_action;
                            else {
                                // If both precedences are equal
                                // If left associative, then reduce
                                if (assoc == TerminalPrec::Left)
                                    goto set_reduce_action;
                                else if (assoc == TerminalPrec::NonAssoc) {
                                    // If non associative, same operator may not appear twice
                                    // So it's an error
                                    actionRef = uint16_t(0u);
                                }
                                // Otherwise shift
                                goto no_conflict_warning;
                            }
                        }
                    } else if (actionRef & ParseTable::ReduceMask &&
                               actionRef != ParseTable::AcceptValue) {
                        // Reduce-reduce conflict
                        auto& prod1 = *stateItem[terminalItems[iTerminal]].pProduction;
                        auto& prod2 = *prod.pProduction;

                        // See if this production is declared to take 
                        // precedence on this non-terninal
                        if (prod1.ReduceOverrides.find(prod2.Left) !=
                            prod1.ReduceOverrides.end())
                            if (prod1.ReduceOverrides[prod2.Left].size() == 0u ||
                                prod1.ReduceOverrides[prod2.Left].find(iTerminal) !=
                                prod1.ReduceOverrides[prod2.Left].end())
                                // Takes precedence, non-conflict
                                goto no_conflict_warning;

                        // See if this production is declared to take
                        // precedence on this non-terninal
                        if (prod2.ReduceOverrides.find(prod1.Left) !=
                            prod2.ReduceOverrides.end()) {
                            if (prod2.ReduceOverrides[prod1.Left].size() == 0u ||
                                prod2.ReduceOverrides[prod1.Left].find(iTerminal) !=
                                prod2.ReduceOverrides[prod1.Left].end()) {
                                // Second precedence, non-conflict
                                terminalItems[iTerminal] = i;
                                // If reducing by a start symbol, set Accept action
                                if (la == TokenCode::TokenEOF &&
                                    prod.pProduction->Left & ProductionMask::AcceptingNonTerminal)
                                    actionRef = uint16_t(ParseTable::AcceptValue);
                                else
                                    actionRef =
                                        uint16_t(ParseTable::ReduceMask | prod.pProduction->Id);
                                goto no_conflict_warning;
                            }
                        }

                        // Resolve R-R by default to first production appearing in file
                        if (prod2.Id > prod1.Id) {
                            terminalItems[iTerminal] = i;
                            // If reducing by a start symbol, set Accept action
                            if (la == TokenCode::TokenEOF &&
                                prod.pProduction->Left & ProductionMask::AcceptingNonTerminal)
                                actionRef = uint16_t(ParseTable::AcceptValue);
                            else
                                actionRef = 
                                    uint16_t(ParseTable::ReduceMask | prod.pProduction->Id);
                        }

                        // See if this is a user-controlled reduction, and if so, resolve it
                        for (const auto lChain: prod1.LeftChain)
                            if (lChain == prod2.Left) {
                                // Display a note
                                if (Messages.GetMessageFlags() & ParseMessageBuffer::MessageNote) {
                                    const auto msgtext = StringWithFormat(
                                                  "State %-3d: User controlled R/R conflict on '%s', symbols %s/%s",
                                                  state, GrammarSymbolsInv[iTerminal]->data(),
                                                  GrammarSymbolsInv[prod1.Left]->data(),
                                                  GrammarSymbolsInv[prod2.Left]->data());
                                    const ParseMessage msg{ParseMessage::NoteMessage, "YC001?W",
                                                           msgtext, 0u, prod.pProduction->Line};
                                    Messages.AddMessage(msg);
                                }
                                goto no_conflict_warning;
                            }
                    }

                    // Report an action conflict warning
                    if (Messages.GetMessageFlags() & ParseMessageBuffer::MessageWarning) {
                        Conflict c;
                        c.State    = state;
                        c.Terminal = iTerminal;

                        if (actionRef & ParseTable::ShiftMask) {
                            // Left hand side nonterminal indexes that had a shift on this symbol
                            c.RRFlag = 0u;

                            // Find which production shift corresponds to
                            // that is, which production has this terminal after the dot
                            // (there has to be at least one or more)
                            for (unsigned j = 0u; j < stateItem.size(); ++j) {
                                if (stateItem[j].Dot == stateItem[j].pProduction->Length)
                                    continue;
                                if (stateItem[j].pProduction->Right(stateItem[j].Dot) ==
                                    iTerminal) {
                                    c.ShiftProductions.insert(stateItem[j].pProduction->Left);
                                    c.ShiftIndexes.push_back(j);
                                }
                            }

                            // Form a string for shift production
                            String ShiftString;
                            for (const auto iSet: c.ShiftProductions) {
                                if (!ShiftString.empty())
                                    ShiftString += ",";
                                ShiftString += *GrammarSymbolsInv[iSet];
                            }

                            c.ReduceLeft2 = table.ReduceProductions[prod.pProduction->Id].Left;
                            c.Item2       = i;

                            // And display a message
                            const auto msgtext = StringWithFormat(
                                          "State %-3d: S/R conflict on '%s', between %s/%s",
                                          state, GrammarSymbolsInv[iTerminal]->data(),
                                          ShiftString.data(),
                                          GrammarSymbolsInv[c.ReduceLeft2]->data());
                            const ParseMessage msg{ParseMessage::WarningMessage, "YC0010W",
                                                   msgtext, 0u, prod.pProduction->Line};
                            Messages.AddMessage(msg);
                        } else {
                            // If Reduce mask, it's and R-R conflict, so get its Id
                            // Otherwise its a conflict on accept, so it's nonterminal value
                            // is always AcceptingNonTerminal
                            c.RRFlag      = 1u;
                            c.Item1       = oldItem;
                            c.Item2       = i;
                            c.ReduceLeft1 = stateItem[c.Item1].pProduction->Left;
                            c.ReduceLeft2 = table.ReduceProductions[prod.pProduction->Id].Left;

                            const auto msgtext = StringWithFormat(
                                          "State %-3d: R/R conflict on '%s', symbols %s/%s",
                                          state, GrammarSymbolsInv[iTerminal]->data(),
                                          GrammarSymbolsInv[c.ReduceLeft1]->data(),
                                          GrammarSymbolsInv[c.ReduceLeft2]->data());
                            const ParseMessage msg{ParseMessage::WarningMessage, "YC0011W",
                                                   msgtext, 0u, prod.pProduction->Line};
                            Messages.AddMessage(msg);
                        }

                        // Store conflict information for conflict report, if needed
                        if (DebugData.Flags & GrammarDebugData::ConflictReport)
                            conflicts.push_back(c);
                    }
                no_conflict_warning:;
                }
            }
        }
    }

    // Free the extra GotoSet
    delete pgotoSet;

    // *** Generate debugging conflict report

    if (DebugData.Flags & GrammarDebugData::ConflictReport) {
        MakeTableData td{table, actionTable, canonicalSet};
        size_t len = 0u;
        String str;
        String buffer;

        if (conflicts.size() > 0u && (DebugData.Flags & GrammarDebugData::ProgressMask)) {
            str = "Generating conflict report";
            if (DebugData.Flags & GrammarDebugData::PrintProgress)
                std::printf("%s", str.data());
            if (DebugData.Flags & GrammarDebugData::StoreProgress)
                DebugData.Progress.push_back(str.data());
        }

        for (size_t i = 0u; i < conflicts.size(); ++i) {
            if (DebugData.Flags & GrammarDebugData::PrintProgress) {
                // Create the progress buffer
                buffer = StringWithFormat(" %zu/%zu", i + 1u, conflicts.size());
                // Delete the previous progress
                for (size_t j = 0u; j < len; ++j)
                    std::printf("\b");
                // Save the length of the progress
                len = buffer.size();
                // Print the progress buffer
                std::printf("%s", buffer.data());
                if (conflicts.size() == i + 1u)
                    std::printf("\n");
            }
            PrintConflict(td, DebugData.Conflicts, conflicts[i]);
        }
    }

    // Copy data off into the action table and delete our data
    const auto conCount = canonicalSet.size();
    table.AllocateTables(conCount, maxTerminal, maxNonTerminal);

    // *** Canonical Debug functionality

    // Go through the canonical set to compute the
    // maximum left side and right side
    size_t maxLHS = 0u;
    size_t maxRHS = 0u;

    if (DebugData.Flags & GrammarDebugData::Canonical) {
        // Resize the canonical item array
        DebugData.CanonicalItems.resize(conCount);
        for (size_t i = 0u; i < conCount; ++i) {
            // Resize each production item array
            const auto prodCount = canonicalSet[i]->size();
            DebugData.CanonicalItems[i].resize(prodCount + 1u);
            for (size_t j = 0u; j < prodCount; ++j) {
                const auto& pr = (*canonicalSet[i])[j];

                if (maxLHS < GrammarSymbolsInv[pr.pProduction->Left]->size())
                    maxLHS = GrammarSymbolsInv[pr.pProduction->Left]->size();
                // Get the max right hand side string length
                String s;
                pr.PrintRHS(s, &GrammarSymbolsInv);
                if (maxRHS < s.size())
                    maxRHS = s.size();
            }
        }
    }

    // Go through and delete all the canonical item sets
    // Copy the action and goto table data into the ParseTable structure
    for (size_t i = 0u; i < conCount; ++i) {
        // Store all the productions for every canonical item
        if (DebugData.Flags & GrammarDebugData::Canonical) {
            const auto prodCount = canonicalSet[i]->size();

            // Print out source states
            (DebugData.CanonicalItems[i])[0u] = "Source state(s): ";
            if (sourceStates[i].size()) {
                auto iSet = sourceStates[i].begin();
                while (iSet != sourceStates[i].end()) {
                    const auto msgtext = StringWithFormat("%u", *iSet);
                    (DebugData.CanonicalItems[i])[0u] += msgtext;
                    ++iSet;
                    if (iSet != sourceStates[i].end())
                        (DebugData.CanonicalItems[i])[0u] += ", ";
                }
            } else
                (DebugData.CanonicalItems[i])[0u] += "Accept";

            for (size_t j = 0u; j < prodCount; ++j) {
                const auto& pr = (*canonicalSet[i])[j];
                pr.Print(*(&(DebugData.CanonicalItems[i])[j + 1u]), &GrammarSymbolsInv, maxLHS,
                         maxRHS);
            }
        }

        // Copy table data
        std::copy(actionTable[i], actionTable[i] + maxTerminal,
                  &(table.GetActionRef(unsigned(i), 0u)));
        std::copy(gotoTable[i], gotoTable[i] + maxNonTerminal,
                  &(table.GetGotoRef(unsigned(i), 0u)));

        // And free the stuff
        delete canonicalSet[i];
        delete[] actionTable[i];
        delete[] gotoTable[i];
    }

    // Report stats if needed
    if (Messages.GetMessageFlags() & ParseMessageBuffer::MessageStats) {
        static constexpr const char* tableTypes[] = 
        {
            "", 
            "LR(1)",
            "LALR(1)", 
            "CLR(1)"
        };

        const auto msgtext = StringWithFormat(
                                 "Created %s parse table: %u States, %u Terminals, %u NonTerminals",
                                 tableTypes[size_t(type)], state, maxTerminal, maxNonTerminal);
        const ParseMessage msg{ParseMessage::StatMessage, "", msgtext};
        Messages.AddMessage(msg);
    }

    // Setup the table type and initial state
    table.Type         = type;
    table.InitialState = 0u;

    // Free inverse symbols
    GrammarSymbolsInv.clear();

    return true;
}


// Checks productions, returns true for no errors
// If productions have no errors, its ok to generate parse table
bool Grammar::CheckProductions() {
    size_t       errorCount = 0u;
    bool         clearSymbolsFlag;

    // First check for at least one production
    if (!ProductionCount) {
        if (Messages.GetMessageFlags() & ParseMessageBuffer::MessageError) {
            const ParseMessage msg{ParseMessage::ErrorMessage, "", "No productions defined"};
            Messages.AddMessage(msg);
        }
        return false;
    }

    // Create inverse symbols for string lookup
    clearSymbolsFlag = GrammarSymbolsInv.empty();
    if (clearSymbolsFlag)
        CreateInverseSymbols(GrammarSymbolsInv);

    // Check that all nonterminals & terminals referenced are defined
    // Create SymbolList set, to check for presence in
    std::set<unsigned> symbols{GrammarSymbolList.begin(), GrammarSymbolList.end()};

    // Go through all the productions
    for (const auto& [lhs, productions]: Productions) {
        // Check left hand side symbol
        // Production std::vector should always be not empty if it appears in map
        if (symbols.find(lhs) == symbols.end()) {
            if (Messages.GetMessageFlags() & ParseMessageBuffer::MessageError) {
                const auto msgtext = StringWithFormat("LHS Production symbol %u not defined", lhs);
                const ParseMessage msg{ParseMessage::ErrorMessage, "", msgtext};
                Messages.AddMessage(msg);
            }
            ++errorCount;
        }

        // Check right hand side
        for (const auto& prod: productions)
            for (unsigned j = 0u; j < prod.pProduction->Length; ++j) {
                size_t errorFlag = 0u;

                // Report any RHS items not in GrammarSymbolList
                if (symbols.find(prod.pProduction->Right(j)) == symbols.end()) {
                    if (Messages.GetMessageFlags() & ParseMessageBuffer::MessageError) {
                        String msgtext;
                        if (prod.pProduction->Right(j) & ProductionMask::Terminal)
                            msgtext = StringWithFormat(
                                          "Symbol \"%s\", RHS terminal %08X not defined",
                                          GrammarSymbolsInv[lhs]->data(),
                                          prod.pProduction->Right(j) &
                                          ProductionMask::TerminalValue);
                        else
                            msgtext = StringWithFormat(
                                          "Symbol \"%s\", RHS nonterminal %08X not defined",
                                          GrammarSymbolsInv[lhs]->data(),
                                          prod.pProduction->Right(j));
                        const ParseMessage msg{ParseMessage::ErrorMessage, "", msgtext};
                        Messages.AddMessage(msg);
                    }
                    ++errorFlag;
                    ++errorCount;
                }

                // And report any nonterminals not derived
                if (errorFlag == 0u)
                    if (!(prod.pProduction->Right(j) & ProductionMask::Terminal))
                        if (Productions.find(prod.pProduction->Right(j)) == Productions.end()) {
                            if (Messages.GetMessageFlags() & ParseMessageBuffer::MessageError) {
                                const auto msgtext = StringWithFormat(
                                    "Symbol \"%s\", RHS nonterminal \"%s\" not defined in grammar",
                                    GrammarSymbolsInv[lhs]->data(),
                                    GrammarSymbolsInv[prod.pProduction->Right(j)]->data());
                                const ParseMessage msg{ParseMessage::ErrorMessage, "", msgtext};
                                Messages.AddMessage(msg);
                            }
                            ++errorCount;
                        }
            }
    }

    // Final check - not reachable productions
    if (Messages.GetMessageFlags() & ParseMessageBuffer::MessageWarning &&
        !(Messages.GetMessageFlags() & ParseMessageBuffer::NoMessageUnreachableProduction)) {
        // Report warnings if:
        // 1. LHS Production symbol can not be reached from start symbol
        // 2. and it does not appear in any RHS except itself

        // Build up a set of reachable Left symbols
        std::set<unsigned>    reachableSymbols;
        std::vector<unsigned> symbolStack;

        size_t i = 0;
        for (; i < StartSymbols.size(); ++i) {
            reachableSymbols.insert(StartSymbols[i]);
            symbolStack.push_back(StartSymbols[i]);
        }

        // No start symbols - first production's left must've been used
        if (i == 0u) {
            reachableSymbols.insert(
                Productions[ProductionMask::AcceptingNonTerminal][0].pProduction->Left);
            symbolStack.push_back(
                Productions[ProductionMask::AcceptingNonTerminal][0].pProduction->Left);
        }

        for (size_t j = 0u; j < symbolStack.size(); ++j)
            for (const auto& prod: Productions[symbolStack[j]])
                // Add every nonterminal referenced on the RHS that has not been added yet
                for (unsigned r = 0u; r < prod.pProduction->Length; ++r)
                    if (!(prod.pProduction->Right(r) & ProductionMask::Terminal))
                        if (reachableSymbols.find(prod.pProduction->Right(r)) ==
                            reachableSymbols.end()) {
                            reachableSymbols.insert(prod.pProduction->Right(r));
                            symbolStack.push_back(prod.pProduction->Right(r));
                        }

        // Go through all production's Left symbols
        // Report unreachable ones
        for (const auto& [lhs, prods]: Productions) {
            // If production results in an error being thrown, don't warn about it being
            // unreachable
            if (!(prods.size() && prods[0u].pProduction->ErrorTerminal))
                if (reachableSymbols.find(lhs) == reachableSymbols.end())
                    if ((lhs & ProductionMask::AcceptingNonTerminal) == 0u) {
                        const auto msgtext = StringWithFormat(
                                                 "Unreachable production symbol \"%s\"",
                                                 GrammarSymbolsInv[lhs]->data());
                        const ParseMessage msg{ParseMessage::WarningMessage, "", msgtext};
                        Messages.AddMessage(msg);
                    }
        }
    }

    if (clearSymbolsFlag)
        GrammarSymbolsInv.clear();

    return (errorCount == 0u);
}


size_t Grammar::CreateProductionVector(std::vector<Production*>& pprodVec) const {
    // Resize the production name std::vector
    pprodVec.resize(ProductionCount);

    // Go through productions and assign names
    for (const auto& [_, prodVec]: Productions)
        for (const auto& prod: prodVec)
            pprodVec[prod.pProduction->Id] = prod.pProduction;

    return pprodVec.size();
}


size_t Grammar::CreateNonterminalVector(std::vector<const String*>& pnonterminalVec) const {
    // Nonterminals have to be in a sorted order
    // (because they index the table)
    std::map<unsigned, const String*> nonTerminals;

    // Go through grammar symbols and save the nonterminals
    for (const auto& [str, prod]: GrammarSymbols)
        if ((prod & ProductionMask::Terminal) == 0u)
            nonTerminals[prod & ProductionMask::TerminalValue] = &str;

    // Now, save the std::vector
    pnonterminalVec.clear();
    for (const auto& [_, nonTerm]: nonTerminals)
        pnonterminalVec.push_back(nonTerm);

    return pnonterminalVec.size();
}


size_t Grammar::CreateTerminalVector(std::vector<String>& pterminalVec, const Lex& lex) const {
    // Make sure there are lexemes to deal with
    if (lex.Lexemes.size() == 0u)
        return 0u;

    // Make sure the std::vector is empty
    pterminalVec.clear();

    // Add the hard-coded default terminals
    pterminalVec.emplace_back("TokenError");
    pterminalVec.emplace_back("TokenEOF");

    // Add all the lexemes
    for (const auto lexeme: lex.TokenLexemes)
        pterminalVec.push_back(lex.Lexemes[lexeme].Name);

    // Go through grammar symbols and save the named error terminals
    for (const auto& [str, prod]: GrammarSymbols)
        if (prod & ProductionMask::ErrorTerminal)
            pterminalVec.push_back("TokenError_" + str.substr(6u, str.size() - 7u));

    return pterminalVec.size();
}


// *** Grammar Output for C/C++

// Produces an enumeration of production names (for reduce keywords)
// Returns number of elements
size_t GrammarOutputC::CreateProductionSwitch(String& str, const String& className,
                                              const String& stackName, const String& prefix,
                                              const String& enumClassName) const {
    if (!pGrammar)
        return 0u;

    const String tab = "    ";
    String dest = "";

    // *** Function declaration

    dest += "bool ";
    dest += className;
    dest += "::Reduce(Parse<";
    dest += stackName;
    dest += "> &parse, unsigned productionID)\n";
    dest += "{\n";

    // *** Switch statement declaration

    if (useEnumClasses) {
        dest += tab + "switch (static_cast<";
        dest += enumClassName;
        dest += ">(productionID))\n";
    } else {
        dest += tab + "switch (productionID)\n";
    }

    dest += tab + "{\n";

    // *** Case statements

    // Create inverse symbols for string lookup
    std::map<unsigned, const String*> grammarSymbolsInv;
    pGrammar->GetInverseGrammarSymbols(grammarSymbolsInv);

    // Go through productions and assign pointers
    std::vector<Production*> prods;
    const auto size = pGrammar->CreateProductionVector(prods);

    // If there are multiple productions with the same name,
    // this counter will be used to generate different labels for them
    size_t sameNameCounter = 1u;

    // Go through the productions and build case statements
    // Starting from the 2nd, the first is always Accept
    for (size_t i = 1u; i < size; ++i) {
        // Return immediately in case of the empty production name
        if (prods[i]->Name.empty())
            return 0u;

        // Store the comment
        dest += tab + tab + "// ";
        dest += *grammarSymbolsInv[prods[i]->Left];
        dest += " -> ";

        if (prods[i]->Length > 0u) {
            for (unsigned k = 0u; k < prods[i]->Length; ++k) {
                if (prods[i]->Right(k) & ProductionMask::Terminal)
                    dest += "'" + *grammarSymbolsInv[prods[i]->Right(k)] + "'";
                else
                    dest += *grammarSymbolsInv[prods[i]->Right(k)];
                dest += " ";
            }
        } else {
            dest += "<empty>";
        }

        dest += "\n";

        // Store the case statements
        dest += tab + tab + "case ";
        dest += useEnumClasses? enumClassName + "::" + prefix : prefix;

        const auto& productionName = prods[i]->Name;

        // If there are multiple productions with the same name,
        // generate different labels for them
        if (productionName == prods[i - 1u]->Name) {
            ++sameNameCounter;
            dest += productionName + StringWithFormat("%u", sameNameCounter);
        } else {
            sameNameCounter = 1u;
            // Check why this special case exists
            dest += productionName == "[Accept10]" ? String{"AcceptPVMRoot"} : productionName;
        }

        dest += ":\n";
        dest += tab + tab + tab;
        dest += "break;\n\n";
    }

    dest += tab + "}\n";
    dest += tab + "return 1u;\n";
    dest += "}\n";

    // Clear the grammar symbol table because it did not originally exist
    grammarSymbolsInv.clear();

    str.swap(dest);

    return size;
}


// Produces an enumeration of production names (for reduce keywords)
// Returns number of elements
size_t GrammarOutputC::CreateProductionEnum(String& str, const String& name,
                                            const String& prefix) const {
    if (!pGrammar)
        return 0u;

    String dest;

    // Go through productions and assign pointers
    std::vector<Production*> prodVec;
    const auto size = pGrammar->CreateProductionVector(prodVec);

    // If there are multiple productions with the same name, 
    // this counter will be used to generate different labels for them
    size_t sameNameCounter = 1u;

    // Generate strings
    createEnum(dest, name, prefix, size,
               [&](size_t enumValueIndex) {
                   // Start with the 2nd production, the first is always Accept
                   if (enumValueIndex > 0u) {
                       const auto& productionName = prodVec[enumValueIndex]->Name;

                       // If there are multiple productions with the same name, 
                       // generate different labels for them
                       if (productionName == prodVec[enumValueIndex - 1u]->Name) {
                           ++sameNameCounter;
                           return (productionName + StringWithFormat("%zu", sameNameCounter));
                       } else {
                           sameNameCounter = 1u;
                           // Check why this special case exists
                           return productionName == "[Accept10]" ? String{"AcceptPVMRoot"}
                                                                 : productionName;
                       }
                   } else {
                       // Return the initial accepting production enum entry
                       return String{"Accept"};
                   }
               });

    str.swap(dest);

    return size;
}

// Produces an enumeration of all the nonterminal; Returns number of elements
size_t GrammarOutputC::CreateNonterminalEnum(String& str, const String& name,
                                             const String& prefix) const {
    if (!pGrammar)
        return 0u;

    String dest;

    std::vector<const String*> nonterminalVec;
    const auto size = pGrammar->CreateNonterminalVector(nonterminalVec);

    // Generate strings
    createEnum(dest, name, prefix, size,
               [&](size_t index) {
                   const auto& valueName = *nonterminalVec[index];
                   // Check why this special case exists
                   return valueName == "[Accept1]" ? String{"Accept1"} : valueName;
               });

    str.swap(dest);

    return size;
}

// Create a hard-coded lexeme enum
size_t GrammarOutputC::CreateTerminalEnum(String& str, const Lex& lex, const String& name,
                                          const String& prefix) const {
    if (!pGrammar)
        return 0u;

    String dest;

    std::vector<String> terminalVec;
    const auto size = pGrammar->CreateTerminalVector(terminalVec, lex);

    // Generate strings
    createEnum(dest, name, prefix, size,
               [&](size_t index) {
                   return terminalVec[index];
               });

    str.swap(dest);

    return size;
}

// Creates enumeration body
template <typename EnumValueNameGenerator>
void GrammarOutputC::createEnum(String& str, const String& name, const String& prefix,
                                const size_t size,
                                EnumValueNameGenerator enumValueNameGenerator) const {
    // EnumValueNameGenerator signature check
    static_assert(std::is_invocable_r<String, EnumValueNameGenerator, size_t>::value,
                  "EnumValueNameGenerator should be callable with signature 'String(size_t)'");

    str += "//////////////////////////// ";
    str += name;
    str += " ////////////////////////////\n\n";

    // Enum type declaration
    str += useEnumClasses ? "enum class " : "enum ";
    str += name;
    str += "\n{\n";

    // Generate strings with the enum values, optionally enclose every value with 'enclosure'
    // string
    const auto listEnumValues = [&](const String& enclosure = "") {
        for (size_t i = 0u; i < size; ++i) {
            // Create comment with the value index
            str += StringWithFormat("    /*%zu*/ ", i);
            // Generate and add an enclosed enum value name
            str += enclosure + prefix + enumValueNameGenerator(i) + enclosure;
            str += i != size - 1u ? ",\n" : "\n";
        }
    };

    listEnumValues();

    str += "};";

    // if needed, add an array of string literals to simplify an enum stringification
    if (createEnumStrings) {
        str += "\n\nconstexpr char const* const StringifyEnum";
        str += name;
        str += "[] =\n{\n";
        // Generate strings as the enum values enclosed with double quotes
        listEnumValues("\"");
        str += "};\n";
    }
}
