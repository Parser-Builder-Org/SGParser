// Filename:  Grammar.h
// Content:   Grammar class header file with declarations
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_GENERATOR_GRAMMAR_H
#define INC_SGPARSER_GENERATOR_GRAMMAR_H

#include "Production.h"
#include "ParseMessage.h"
#include "ParseTableGen.h"
#include "Parser.h"

#include <map>
#include <set>
#include <vector>

namespace SGParser {
namespace Generator {

// ***** Grammar representation

// Grammar class holds grammar in terms of productions and their text labels
// Routines to perform calculation of the parse table are also included here

// Grammar Debug structure
struct GrammarDebugData final
{
    enum ActionFlags : unsigned  
    {
        Canonical                   = 0x0000'0001,
        ConflictReport              = 0x0000'0010,
        ConflictReportNoLineNumbers = 0x0000'0020,
        ConflictReportNoLabels      = 0x0000'0040,
        ConflictReportNoPoints      = 0x0000'0080,

        StoreProgress               = 0x4000'0000,
        PrintProgress               = 0x8000'0000,
        ProgressMask                = 0xC000'0000
    };

    // Action flags
    unsigned                         Flags = 0u;

    // Debug data
    std::vector<std::vector<String>> CanonicalItems;
    String                           Conflicts;
    // Progress stats
    std::vector<String>              Progress;

    // *** Functions

    void Clear() {
        Flags = 0u;
        Conflicts.clear();
        CanonicalItems.clear();
        Progress.clear();
    }
};


// *** Grammar class

struct MakeTableData;
struct Conflict;
class  Lex;

class Grammar final
{
public:
    // Map from tokens to their precedence & associativity
    struct TerminalPrec final
    {
        enum Assoc : unsigned 
        {
            Left       = 0x0000'0000,
            Right      = 0x4000'0000,
            NonAssoc   = 0x8000'0000,
            AssocMask  = 0xC000'0000,
            PrecMask   = 0x3FFF'FFFF
        };

        unsigned Value = Left;
    };

public:
    // *** Constructors & destructor

    // Makes empty grammar
    Grammar() = default;

    // Makes a grammar out of a grammar symbols & productions
    // Start symbol is obtained from first production
    Grammar(const std::map<String, unsigned>& symbolMap, std::vector<Production>& prodList) {
        Create(symbolMap, prodList);
    }

    // Makes a grammar out of a grammar symbols & productions
    // Start symbol is obtained from first production
    // Assigns the precedence map
    Grammar(const std::map<String, unsigned>& symbolMap, std::vector<Production>& prodList,
            const std::map<unsigned, TerminalPrec>& prec) {
        Create(symbolMap, prodList, prec);
    }

    // No copy/move allowed
    Grammar(const Grammar&)                = delete;
    Grammar(Grammar&&) noexcept            = delete;
    Grammar& operator=(const Grammar&)     = delete;
    Grammar& operator=(Grammar&&) noexcept = delete;

    // Destroys the grammar
    ~Grammar() { ClearProductions(); }

    // *** Creation

    // Makes a grammar out of a grammar symbols & productions
    // Start symbol is obtained from first production
    void Create(const std::map<String, unsigned>& symbolmap, std::vector<Production>& prodList);
    void Create(const std::map<String, unsigned>& symbolmap, std::vector<Production>& prodList,
                const std::map<unsigned, TerminalPrec>& prec);

    // *** Grammar Data interface

    // Clears all grammar data
    void Clear();
    // Clears all productions
    void ClearProductions();

    // Adds a production
    void AddProduction(Production& prod);
    // Adds multiple productions
    void AddProductions(std::vector<Production>& prodList);

    // Adds a grammar symbol (1 fro success)
    // Symbol can be a terminal or nonterminal based on ProductionMask::Terminal bit
    void AddGrammarSymbol(const String& str, unsigned value);
    // Adds multiple grammar symbols
    void AddGrammarSymbols(const std::map<String, unsigned>& symbolMap);

    // Sets the current starting production
    void SetStartSymbols(const std::vector<unsigned>& symbol);

    // Sets the precedence map
    void SetPrecedence(const std::map<unsigned, TerminalPrec>& prec);

    // *** General access and utility functionality

    // Returns whether or not the grammar is usable
    bool                        IsValid() const noexcept     { return ProductionCount != 0u; }

    // Obtain message buffer
    ParseMessageBuffer&         GetMessageBuffer() noexcept  { return Messages; }

    // Obtain debug data
    GrammarDebugData&           GetDebugData() noexcept      { return DebugData; }

    // Obtain the set of valid start symbols
    std::vector<unsigned>&      GetStartSymbols() noexcept   { return StartSymbols; }

    // Strings corresponding to production values
    std::map<String, unsigned>& GetGrammarSymbols() noexcept { return GrammarSymbols; }

    // Inverse symbols list, only created occasionally
    void GetInverseGrammarSymbols(std::map<unsigned, const String*>& grammarSymbolInv) {
        CreateInverseSymbols(grammarSymbolInv);
    }

    // List of grammar symbols (includes both nonterminals and terminals)
    std::vector<unsigned>& GetGrammarSymbolList() noexcept   { return GrammarSymbolList; }

    // A map of productions
    // All productions are grouped together based on their left hand side
    std::map<unsigned, std::vector<ParseTableProduction>>& GetProductions() noexcept {
        return Productions;
    }

    // Number of productions, used to generate IDs
    size_t GetProductionCount() const noexcept { return ProductionCount; }

    // *** Table construction

    // Return true for success (no errors), false otherwise
    bool   MakeParseTable(ParseTableGen& table, ParseTable::TableType type);

    // Checks productions, returns true for no errors
    // If productions have no errors, its ok to generate parse table
    bool   CheckProductions();

    // *** Store grammar data into strings

    // Creates an array of productions (for reduce keywords); Returns number of elements
    size_t CreateProductionVector(std::vector<Production*>& prodVec) const;
    // Creates an array of nonterminals; Returns number of elements
    size_t CreateNonterminalVector(std::vector<const String*>& nonterminalVec) const;
    // Creates a terminal String constant
    size_t CreateTerminalVector(std::vector<String>& terminalVec, const Lex& lex) const;

    // *** Utility computation functions on grammar

    // Compute First terminal set for a list of symbols (i.e. either nonterminals or terminals)
    // Result is a complete test of terminals that appears in the beginning of any production
    // looked up by symbol
    // Return true if First contains an empty production, otherwise returns zero
    bool   First(std::set<unsigned>& terminalsFound, const std::vector<unsigned>& symbols) const;
    bool   First(std::set<unsigned>& terminalsFound, unsigned symbol, bool* mark) const;

    // Follow set generation

    // Compute Goto item set on symbol, given an existing item set
    // Which means find all productions which have specified symbol following the dot,
    // collect them following shift of the dot to the right and perform closure
    void   Goto(std::vector<ParseTableProduction>& gotoSet,
                const std::vector<ParseTableProduction>& itemSet, unsigned symbol) const;
    // Compute Closure
    // Add productions deriving nonterminals after the dot, recursively
    void   Closure(std::vector<ParseTableProduction>& closure) const;

private:
    // Strings corresponding to production values
    std::map<String, unsigned>                            GrammarSymbols;
    // Inverse symbols list, only created occasionally
    std::map<unsigned, const String*>                     GrammarSymbolsInv;

    // List of grammar symbols (includes both nonterminals and terminals)
    std::vector<unsigned>                                 GrammarSymbolList;

    // A map of productions
    // All productions are grouped together based on their left hand side
    std::map<unsigned, std::vector<ParseTableProduction>> Productions;
    // Number of productions, used to generate IDs
    size_t                                                ProductionCount = 0u;

    // Precedence map only has entries for terminals (terminal -> prec/assoc)
    // High order bit (ProductionMask::Terminal) is set in the terminal, as expected
    std::map<unsigned, TerminalPrec>                      Precedence;

    // A set of valid start symbols
    std::vector<unsigned>                                 StartSymbols;

    // Records debug data
    GrammarDebugData                                      DebugData;
    // Parse message reporting (error/warning/etc)
    ParseMessageBuffer                                    Messages;

    // *** Internal Utility functions

    // Implementation helper for adding a production
    void   AddProductionImpl(Production& prod);

    // Creates inverse symbols table
    void   CreateInverseSymbols(std::map<unsigned, const String*>& inverseSymbols) const;

    // Creates [Accept]->Nonterminal productions for starting nonterminals
    void   CreateAcceptingProductions(unsigned defaultLeft);

    // *** Debug text i/o

    // Debug conflict report functionality
    size_t PrintProductions(String& str, const std::vector<ParseTableProduction>& prodVec,
                            unsigned specialTerminal, bool labels = true, bool lineNumbers = true,
                            bool points = true);
    void   PrintConflict(const MakeTableData& td, String& str, const Conflict& conflict);

    // Calculates productions in which nonterminal is used before lookahead
    void   GetNonterminalFollowProductions(const MakeTableData& td,
                                           std::vector<ParseTableProduction>& prodVec,
                                           unsigned nonTerminal, unsigned lookAhead) const;
};


// Generic grammar output with no functionality except default constructor
class GrammarOutput 
{
public:
    // Default Constructor
    GrammarOutput(Grammar* pgrammar = nullptr) : pGrammar{pgrammar} {}

    virtual ~GrammarOutput() = default;

    // Assign a specific grammar
    void SetGrammar(Grammar* pgrammar = nullptr) noexcept { pGrammar = pgrammar; }

    // *** Store grammar data into strings

    // Produces an enumeration of production names (for reduce keywords)
    // Returns number of elements
    virtual size_t CreateProductionEnum(String& str, const String& name,
                                        const String& prefix = "") const                     = 0;
    // Creates a Reduce function and switch statement with cases for all enumerations
    // Returns number of elements
    virtual size_t CreateProductionSwitch(String& str, const String& className,
                                          const String& stackName, const String& prefix = "",
                                          const String& enumClassName = "") const            = 0;

    // Produces an enumeration of nonterminals
    // Returns number of elements
    virtual size_t CreateNonterminalEnum(String& str, const String& name,
                                         const String& prefix = "") const                    = 0;
    // Creates a enumeration of terminals
    // Returns number of elements
    virtual size_t CreateTerminalEnum(String& str, const Lex& lex, const String& name,
                                      const String& prefix = "") const                       = 0;

protected:
    Grammar* pGrammar;
};


// Grammar Output which dumps C files
class GrammarOutputC final : public GrammarOutput 
{
public:
    GrammarOutputC(Grammar* pgrammar = nullptr, bool enumClasses = false, bool enumStrings = false)
        : GrammarOutput{pgrammar},
          useEnumClasses{enumClasses},
          createEnumStrings{enumStrings}
    {}

    // *** Store grammar data into strings

    // Produces an enumeration of production names (for reduce keywords)
    // Returns number of elements
    size_t CreateProductionEnum(String& str, const String& name,
                                const String& prefix = "") const override;
    // Creates a Reduce function and switch statement with cases for all enumerations
    // Returns number of elements
    size_t CreateProductionSwitch(String& str, const String& className, const String& stackName,
                                  const String& prefix= "",
                                  const String& enumClassName = "") const override;

    // Produces an enumeration of nonterminals
    // Returns number of elements
    size_t CreateNonterminalEnum(String& str, const String& name,
                                 const String& prefix = "") const override;
    // Creates an enumeration of terminals
    // Returns number of elements
    size_t CreateTerminalEnum(String& str, const Lex& lex, const String& name,
                              const String& prefix = "") const override;

private:
    // If true then use 'enum class' instead of 'enum'
    const bool useEnumClasses;

    // If true then create string literals for enumeration stringification
    const bool createEnumStrings;

    // Creates an enumeration with 'size' number if values using result of call
    // 'enumValueNameGenerator(index)' as an enum value name for the given index
    // EnumValueNameGenerator should be callable with signature 'String(size_t)'
    // Returns number of elements
    template <typename EnumValueNameGenerator>
    void createEnum(String& str, const String& name, const String& prefix, const size_t size,
                    EnumValueNameGenerator enumValueNameGenerator) const;
};

} // namespace Generator
} // namespace SGParser

#endif // INC_SGPARSER_GENERATOR_GRAMMAR_H
