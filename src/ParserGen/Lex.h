// Filename:  Lex.h
// Content:   Lex class, converts REs & labels to DFA
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_GENERATOR_LEX_H
#define INC_SGPARSER_GENERATOR_LEX_H

#include "ParseMessage.h"
#include "Grammar.h"
#include "NFA.h"
#include "DFAGen.h"
#include "Lexeme.h"


/** @todo Complete Unicode support.
    Currently, Parser supports UTF-8 characters in string literals, comments, and keywords,
    but not in regular expressions.
    The root cause of this limitation is that the grammar's regular-expression machinery
    can handle char-sets containing characters from 7-bit ASCII only.
    Possible steps to support non-ASCII characters in numeric-literals and/or non-keyword
    identifiers are:
**/
/**
    - Tokenizer should be modified to properly read UTF-8 characters, e.g. by adopting
      existing UTF8::String class.
**/
/**
    - CharTable class should be modified to provide a mapping from the character range
      to some state in the DFA table. Character range should not unfold to the array
      of characters, instead, it should represent the ranges in the form [from, to]
      (and possibly ^[from, to] - inverted range). Such an approach may require
      the implementation of functions for ranges manipulation, e.g. union, intersection,
      splitting, etc.
**/
/**
    - Optional: DFA table can be optimized by reducing the number of rows to an actual
      number of character ranges in use. Such an approach will probably require two passes
      for parsing (first pass for gathering the needed character ranges, the second pass
      for creating corresponding DFA table).
**/

namespace SGParser
{
namespace Generator
{

// ***** Lex class

class Lex final 
{
public:
    // DFA Construction type to use
    enum class DFAConstructType
    {
        // Create the DFA using many NFAs
        NFA,
        // Create the DFA using a set of syntax trees
        SyntaxTree
    };

    // Expressions - contain groups of lexemes
    struct Expression final 
    {
        unsigned StartLexeme;
        unsigned LexemeCount;
    };

public:
    // Default constructor
    Lex() = default;

    // Returns whether or not the lex is usable
    bool IsValid() const noexcept                   { return !Lexemes.empty(); }

    // Obtain message buffer
    ParseMessageBuffer& GetMessageBuffer() noexcept { return Messages; }

    // Converts action parameters into numbers
    bool ConvertActionParam();

    // Clear the Lexeme data
    void Clear() noexcept;

    // Builds a DFA from the lexemes
    // using either NFA based or DFA syntax tree based construction
    bool MakeDFA(DFAGen& dfa, DFAConstructType type = DFAConstructType::NFA);

    // Check for an error and report if needed
    void CheckForErrorAndReport(const char* message...);

    // Macros (macro name -> macro regular expression)
    std::map<String, String>                  Macros;

    // Lexeme
    std::vector<Lexeme>                       Lexemes;

    std::vector<Expression>                   Expressions;
    std::map<String, unsigned>                ExpressionNames;

    // Token codes, these are indexes into 'Lexemes'
    // Contains lexemes that result in a token (are not ignored)
    // token code -> lexeme index
    std::vector<unsigned>                     TokenLexemes;

    // Lexeme name -> token lexeme index
    std::map<String, unsigned>                LexemeNameToToken;
    // Lexeme alias name -> token lexeme index
    std::map<String, unsigned>                LexemeAliasToToken;

    // Precedence map only has entries for terminals (terminal -> prec/assoc)
    // High order bit (ProductionMask::Terminal) is set in the terminal, as expected
    // (see Grammar.h for TerminalPrec structure definition)
    std::map<unsigned, Grammar::TerminalPrec> Precedence;

    // Cached NFA parse table used by MakeNFA
    ParseTableGen                             NFAParseTable;

    // Message buffer for reporting data
    ParseMessageBuffer                        Messages;

private:
    // Create a grammar from the hard-coded regular expression
    void CreateRegExpGrammar(Grammar& grammar) const;

    // Builds an NFA from a regular expression
    bool MakeNFA(NFA& nfa, const String& regExpr, std::map<String, NFA*>& macroNFAs);

    // Builds a Syntax Tree from a regular expression (used for DFA creation)
    template <class T>
    bool MakeSyntaxTree(DFASyntaxTree<T>& tree, const String& regExpr,
                        std::map<String, DFASyntaxTree<T>*>& macroSyntaxTrees);

    // Builds a DFA from a syntax tree
    template <class T>
    bool MakeDFA(DFAGen& dfa, DFASyntaxTree<T>& tree, std::vector<Lexeme>& lexemes);
    bool MakeDFAUsingNFA(DFAGen& dfa);
    bool MakeDFAUsingSyntaxTree(DFAGen& dfa);
};

} // namespace Generator
} // namespace SGParser

#endif // INC_SGPARSER_GENERATOR_LEX_H
