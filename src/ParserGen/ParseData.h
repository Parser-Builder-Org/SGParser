// Filename:  ParseData.h
// Content:   Core parse data grammar construction class
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_GENERATOR_PARSEDATA_H
#define INC_SGPARSER_GENERATOR_PARSEDATA_H

#include "ParseMessage.h"
#include "ParseTableGen.h"
#include "Lex.h"
#include "DFAGen.h"
#include "Tokenizer.h"
#include "Parser.h"

namespace SGParser
{
namespace Generator
{

// ***** Parse Data

class ParseData
{
public:
    virtual ~ParseData() = default;

    // *** Initializing grammar

    // Load in the grammar
    virtual bool LoadGrammar(InputStream* ptokenInput)                                        = 0;

    // Clear the grammar
    virtual void ClearGrammar()                                                               = 0;

    // Return true if the rule file is loaded
    virtual bool IsValid() const noexcept                                                     = 0;

    // *** Using grammar

    // Create the DFA from lexemes
    virtual bool MakeDFA(DFAGen& dfa)                                                         = 0;
    // Create the parse table
    virtual bool MakeParseTable(ParseTableGen& parseTable,
                                ParseTableType tableType = ParseTableType::CLR)               = 0;
};


// Standard SG grammar parse data class
// - SG format is similar to the Visual Parse++ file format
class StdGrammarParseData final : public ParseData
{
public:
    // *** Constructors

    // Create an empty
    StdGrammarParseData() = default;
    // Create and load from data
    explicit StdGrammarParseData(InputStream* ptokenInput) { BuildParser(ptokenInput); }

    // *** Managing loaded grammar (and lexemes)

    // Load in the grammar
    bool LoadGrammar(InputStream* ptokenInput) override;
    // Clear the grammar
    void ClearGrammar() override;

    // *** Using grammar

    // Create the DFA from lexemes
    bool MakeDFA(DFAGen& dfa) override;
    // Create the parse table
    bool MakeParseTable(ParseTableGen& parseTable,
                        ParseTableType tableType = ParseTableType::CLR) override;

    // *** Variable Access

    // Return true if data is loaded
    bool IsValid() const noexcept override { return UserLex.IsValid() && UserGrammar.IsValid(); }

    // Obtain the user lex data
    Lex&                GetLex() noexcept           { return UserLex; }

    // Obtain the user grammar data
    Grammar&            GetGrammar() noexcept       { return UserGrammar; }

    // Obtain message buffer
    ParseMessageBuffer& GetMessageBuffer() noexcept { return Messages; }

private:
    // Standard Grammar Rule file parsing data
    ParseTableGen     StdGrammarParseTable;
    DFAGen            StdGrammarDFA;

    // User data
    Lex               UserLex;
    Grammar           UserGrammar;

    // Message buffer for reporting (errors, warnings, etc)
    ParseMessageBuffer Messages;

    // Loads and parses the grammar
    bool BuildParser(InputStream* ptokenInput);

    // Creates a hard-coded set of
    void CreateVectors(std::map<String, unsigned>& grammarSymbols,
                       std::vector<Production>& productions);
    // Creates a hard-coded set of lexemes for the Standard grammar file type
    void CreateLexemes(std::vector<Lexeme>& lexemes);
    // Create the grammar symbol map
    bool CreateSymbolMap(class StdGrammarParseHandler& data,
                         std::map<String, unsigned>& symbols,
                         std::vector<Production>& productions,
                         std::vector<unsigned>& startSymbols);
};

} // namespace Generator
} // namespace SGParser

#endif // INC_SGPARSER_GENERATOR_PARSEDATA_H
