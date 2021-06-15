// Filename:  ProductionMask.h
// Content:   Production mask consts
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_PRODUCTIONMASK_H
#define INC_SGPARSER_PRODUCTIONMASK_H

namespace SGParser
{
namespace ProductionMask
{

// Values in productions are nonterminals, unless the TerminalMask is set
static constexpr unsigned Terminal             = 0x8000'0000;
// Named error terminal
static constexpr unsigned ErrorTerminal        = 0x4000'0000;
// Backtracking error terminal
static constexpr unsigned BacktrackError       = 0x2000'0000;

static constexpr unsigned TerminalValue        = 0x0FFF'FFFF;
// A value for an 'accepting' NonTerminal (used for LHS) of first production
static constexpr unsigned AcceptingNonTerminal = 0x1000'0000;

} // namespace ProductionMask
} // namespace SGParser

#endif // INC_SGPARSER_PRODUCTIONMASK_H
