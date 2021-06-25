// Filename:  Production.h
// Content:   Production class implementation for the Parser
// Provided AS IS under MIT License; see LICENSE file in root folder.

#include "Production.h"
#include "Tokenizer.h"

namespace SGParser
{
namespace Generator
{

// *** Production Class implementation

// *** Constructors

// Constructor with for default value
Production::Production(const String& name, unsigned left, const unsigned* pright,
                       unsigned rightCount, size_t line, unsigned prec)
    : Name{name},
      Id{0u},
      Precedence{prec},
      Left{left},
      Length{rightCount},
      pRight{new unsigned[Length + 1u]},
      Line{line},
      NotReported{0u},
      ErrorTerminal{0u} {
    pRight[0u] = 1u;
    std::copy(pright, pright + Length, pRight + 1u);
}


// Destructor
Production::~Production() {
    // Delete RHS
    if (pRight)
        if (!(--pRight[0u]))
            delete[] pRight;
}


// Set production value
void Production::SetProduction(const String& name, unsigned left, const unsigned* pright,
                               unsigned rightCount, size_t line, unsigned prec) {
    // Delete RHS
    if (pRight)
        if (!(--pRight[0u]))
            delete[] pRight;

    Name        = name;
    Precedence  = prec;
    Left        = left;
    Length      = rightCount;
    pRight      = new unsigned[size_t(Length) + 1u];
    Line        = line;
    NotReported = 0u;

    pRight[0u]  = 1u;
    std::copy(pright, pright + Length, pRight + 1u);
}


// *** Production routines

// Copy operator
Production& Production::operator=(const Production& source) {
    // Destroy previous pRight, assign new one
    if (pRight != source.pRight) {
        if (pRight)
            if (!(--pRight[0u]))
                delete[] pRight;

        if (pRight = source.pRight; pRight != nullptr)
            ++pRight[0u];
    }

    Name            = source.Name;
    Id              = source.Id;
    Precedence      = source.Precedence;
    Left            = source.Left;
    Length          = source.Length;
    Line            = source.Line;
    NotReported     = source.NotReported;
    ErrorTerminal   = source.ErrorTerminal;
    LeftChain       = source.LeftChain;
    ReduceOverrides = source.ReduceOverrides;
    ConflictActions = source.ConflictActions;

    return *this;
}


// Productions are the same
bool Production::operator==(const Production& right) const noexcept {
    return (Left == right.Left) && RHSEquals(right);
}


// Compare Right hand side
bool Production::RHSEquals(const Production& right) const noexcept {
    if (pRight == right.pRight)
        return true;
    else if (Length != right.Length)
        return false;
    return std::equal(pRight + 1u, pRight + 1u + Length, right.pRight + 1u);
}


// *** ParseTableProduction

// Productions are the same
bool ParseTableProduction::operator==(const ParseTableProduction& right) const noexcept {
    return (*pProduction == *right.pProduction) && (Dot == right.Dot) &&
           (LookAhead == right.LookAhead);
}


// Less operator, for sorting productions
bool ParseTableProduction::operator<(const ParseTableProduction& right) const noexcept {
    if (pProduction->Id < right.pProduction->Id)
        return true;
    else if (pProduction->Id == right.pProduction->Id) {
        if (Dot < right.Dot)
            return true;
        else if (Dot == right.Dot)
            if (LookAhead < right.LookAhead)
                return true;
    }
    return false;
}


// *** Debugging

// Builds a string of the Right hand side of the production
size_t ParseTableProduction::PrintRHS(String& str,
                                      std::map<unsigned, const String*>* pgrammarSymbolsInv,
                                      bool printDot) const {
    String dest;
    size_t dotPos = 0u;

    // Go through all right hand side terminals and store them
    unsigned i = 0u;
    for (; i < pProduction->Length; ++i) {
        if (Dot == i) {
            dotPos = dest.size();
            if (printDot)
                dest += "* ";
        }

        if (pProduction->Right(i) & ProductionMask::Terminal) {
            if (pgrammarSymbolsInv)
                dest += StringWithFormat("'%s' ",
                                         (*pgrammarSymbolsInv)[pProduction->Right(i)]->data());
            else {
                const unsigned terminal = pProduction->Right(i) & ProductionMask::TerminalValue;
                if (terminal < 32u + TokenCode::TokenFirstID)
                    dest += StringWithFormat("'%X' ", terminal);
                else
                    dest += StringWithFormat("'%c' ", char(terminal - TokenCode::TokenFirstID));
            }
        } else {
            if (pgrammarSymbolsInv)
                dest += *(*pgrammarSymbolsInv)[pProduction->Right(i)] + " ";
            else
                dest += StringWithFormat("%u ", pProduction->Right(i));
        }
    }

    if (Dot == i) {
        dotPos = dest.size();
        if (printDot)
            dest += "* ";
    }

    str.swap(dest);

    return dotPos;
}


// Builds a string of the Look Ahead values of the production
void ParseTableProduction::PrintLookAhead(String& str,
                                   std::map<unsigned, const String*>* pgrammarSymbolsInv) const {
    String dest;
    bool   flag = false;

    for (const auto iTerminal : LookAhead) {
        if (flag)
            dest += " ";
        flag = true;

        const auto terminal = iTerminal & ProductionMask::TerminalValue;

        if (pgrammarSymbolsInv) {
            if (terminal == TokenCode::TokenEOF)
                dest += "EOF";
            else
                dest += *(*pgrammarSymbolsInv)[iTerminal];
        } else {
            if (terminal == TokenCode::TokenEOF)
                dest += "EOF";
            else if (terminal < 32u + TokenCode::TokenFirstID)
                dest += StringWithFormat("x%02X", terminal);
            else
                dest += StringWithFormat("%c", char(terminal - TokenCode::TokenFirstID));
        }
    }

    str.swap(dest);
}


// Build a string of the production
void ParseTableProduction::Print(String& str,
                                 std::map<unsigned, const String*>* pgrammarSymbolsInv,
                                 size_t leftSpace, size_t rightSpace) const {
    // Print the production id (0-9999)
    String buff = StringWithFormat("ID:%-4d ", pProduction->Id);
    String dest = buff;

    // *** Print out the Left non terminal

    if (pgrammarSymbolsInv) {
        // Build the format string with the correct spacing
        buff  = StringWithFormat("%%-%zus -> ", leftSpace);
        // Store the left non terminal
        dest += StringWithFormat(buff.data(),
                                 (*pgrammarSymbolsInv)[pProduction->Left]->data());
    } else
        dest += StringWithFormat("%-3d -> ", pProduction->Left);

    // Save the current length
    const auto slen = dest.size();

    // *** Build and add the right hand side
    String rhs;
    PrintRHS(rhs, pgrammarSymbolsInv);
    dest += rhs;

    // *** Build and add the look ahead

    if (Dot == pProduction->Length) {
        // If the right space is zero than give it some padding
        if (rightSpace == 0u)
            rightSpace = rhs.size() + 4u;

        // Build the format string with the correct spacing
        buff = StringWithFormat("%%-%zus LA: '", slen + rightSpace);
        // Reposition the buffer
        dest = StringWithFormat(buff.data(), dest.data());

        String la;
        PrintLookAhead(la, pgrammarSymbolsInv);
        dest += la + "'";
    }

    str.swap(dest);
}


// *** Utility Functions for production data structures

// Return index if production std::vector is in a set of std::vectors, -1 for error
size_t ParseTableProduction::FindVectorInSetOfSets(const std::vector<ParseTableProduction>& v,
                                  const std::vector<std::vector<ParseTableProduction>*>& setOfSets,
                                  size_t startIndex, bool compareLookAhead) noexcept {
    if (compareLookAhead) {
        // Compare lookahead, for LR
        for (size_t i = startIndex; i < setOfSets.size(); ++i) {
            const auto& s = *setOfSets[i];
            if (s.size() == v.size()) {
                size_t j = 0u;
                for (; j < v.size(); ++j)
                    if (v[j].Dot != s[j].Dot ||
                        v[j].pProduction->Left != s[j].pProduction->Left ||
                        !v[j].pProduction->RHSEquals(*s[j].pProduction) ||
                        v[j].LookAhead != s[j].LookAhead)
                        break;
                if (j == v.size())
                    return i;
            }
        }
    } else {
        // Do not compare lookahead, for LALR
        for (size_t i = startIndex; i < setOfSets.size(); ++i) {
            const auto& s = *setOfSets[i];
            if (s.size() == v.size()) {
                size_t j = 0u;
                for (; j < v.size(); ++j)
                    if (v[j].Dot != s[j].Dot ||
                        v[j].pProduction->Left != s[j].pProduction->Left ||
                        !v[j].pProduction->RHSEquals(*s[j].pProduction))
                        break;
                if (j == v.size())
                    return i;
            }
        }
    }
    // It's not in
    return size_t(-1);
}

} // namespace Generator
} // namespace SGParser
