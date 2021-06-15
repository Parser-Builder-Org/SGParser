// Filename:  ParseData.cpp
// Content:   Core parse data class implementation
// Provided AS IS under MIT License; see LICENSE file in root folder.

#include "DFATokenizer.h"
#include "ParseData.h"
#include "StdGrammarLexemeEnum.h"
#include "StdGrammarProdEnum.h"

namespace SGParser
{
namespace Generator
{

// *** ParseData

void StdGrammarParseData::CreateVectors(std::map<String, unsigned>& grammarSymbols,
                                        std::vector<Production>& productions) {
    // Define a grammar. Notice the first production is the "augment" production
    // You've always got to have it since it tells the
    // parser the start symbol and creates an augmented grammar
    // Create a set of grammar symbols and productions to represent the grammar

    // *** Terminal set

    static constexpr size_t terminalCount = 43u;
    static constexpr const char* terminalSet[terminalCount] =
    {
      "ROOT_BLOCK'",
      "ROOT_BLOCK",
      // Macro
      "MACRO_SECTION",
      "MACRO_BLOCK",
      "MACRO_LINE",
      "MACRO_NAME",
      "MACRO_EXPRESSION",
      // Expression
      "EXPRESSION_SECTIONLIST",
      "EXPRESSION_SECTION",
      "EXPRESSION_BLOCK",
      "EXPRESSION_LINE",
      // Precedence
      "PRECEDENCE_SECTION",
      "PRECEDENCE_BLOCK",
      "PRECEDENCE_LINE",
      "OPT_NUM",
      "TERMINAL_LIST",
      "TERMINAL",
      "ASSOC",
      // Production
      "PRODUCTION_SECTION",
      "PRODUCTION_STARTLIST",
      "PRODUCTION_STARTSYMBOL",
      "PRODUCTION_STARTSYMBOLLIST",
      "PRODUCTION_BLOCK",
      "PRODUCTION_LINE",
      "PRODUCTION_LEFT",
      "PRODUCTION_LHSNAMELIST",
      "PRODUCTION_LHSNAME",
      "PRODUCTION_RIGHT",
      "PRODUCTION_RHS",
      "PRODUCTION_SYMBOLCOMBINE",
      "PRODUCTION_FULLSYMBOL",
      "PRODUCTION_PREFIX",
      "PRODUCTION_SYMBOL",
      "PRODUCTION_ERROR",
      "NONTERMINAL_LIST",
      "PRODUCTION_POSTFIX",
      "PRODUCTION_PREC",
      "PRODUCTION_REDUCE",
      "REDUCE_LIST",
      "REDUCE_EXPR",
      "REDUCE_NONTERMINAL_LIST",
      "REDUCE_TERMINAL_LIST",
      "NAMED_ERROR"
    };

    // Put in the terminals
    for (size_t i = 0u; i < terminalCount; ++i)
        grammarSymbols.insert_or_assign(terminalSet[i], unsigned(i));

    static constexpr size_t nonTerminalCount = 34u;
    static constexpr struct nonTerminalInfo {
        const char* pName;
        unsigned    Id;
    } nonTerminalSet[nonTerminalCount] =
    {
      {"%error",           SGL_TokenError},
      {"bad_char",         SGL_bad_char},
      {"number_int",       SGL_number_int},
      {"string_cons",      SGL_string_cons},
      {"identifier",       SGL_identifier},

      {"macro",            SGL_macro},

      {"expression",       SGL_expression},
      {"ignore",           SGL_ignore},
      {"push",             SGL_push},
      {"pop",              SGL_pop},
      {"goto",             SGL_goto},

      {"prec",             SGL_prec},
      {"left",             SGL_left},
      {"right",            SGL_right},
      {"nonassoc",         SGL_nonassoc},

      {"production",       SGL_production},
      {"shifton",          SGL_shifton},
      {"reduceon",         SGL_reduceon},
      {"reduce",           SGL_reduce},
      {"action",           SGL_action},
      {"error",            SGL_error},
      {"error_backtrack",  SGL_error_backtrack},

      {",",                SGL_comma},
      {";",                SGL_semicolon},
      {"?",                SGL_question},
      {"&",                SGL_and},
      {"|",                SGL_or},
      {"{",                SGL_curlyopen},
      {"}",                SGL_curlyclose},
      {"(",                SGL_lparen},
      {")",                SGL_rparen},
      {">",                SGL_greaterthan},
      {"->",               SGL_arrow},
      {"quote_cons",       SGL_quote_cons}
    };

    // Add all the nonterminals
    for (size_t i = 0u; i < nonTerminalCount; ++i)
        grammarSymbols.insert_or_assign(nonTerminalSet[i].pName,
                                        nonTerminalSet[i].Id | ProductionMask::Terminal);

    static constexpr size_t productionCount = 107u;
    static constexpr struct ProductionInit {
        const char* pName;
        const char* pGrammarSymbol;
        size_t      Length;
        const char* pRight[6u];
    } init[productionCount]=
    {

      {"RootBlock",                   "ROOT_BLOCK",                 4u, {"MACRO_SECTION", "EXPRESSION_SECTIONLIST", "PRECEDENCE_SECTION", "PRODUCTION_SECTION"} },
      {"RootBlockError",              "ROOT_BLOCK",                 1u, {"%error"}                                                                              },

      // Macro
      {"MacroSection",                "MACRO_SECTION",              2u, {"macro", "MACRO_BLOCK"}                                                                },
      {"MacroSectionError",           "MACRO_SECTION",              2u, {"macro", "%error"}                                                                     },
      {"MacroSectionEmpty",           "MACRO_SECTION",              0u, {}                                                                                      },

      {"MacroBlock",                  "MACRO_BLOCK",                2u, {"MACRO_BLOCK", "MACRO_LINE"}                                                           },
      {"MacroBlockEmpty",             "MACRO_BLOCK",                0u, {}                                                                                      },

      {"MacroLine",                   "MACRO_LINE",                 2u, {"MACRO_NAME", "MACRO_EXPRESSION"}                                                      },
      {"MacroLineError",              "MACRO_LINE",                 2u, {"%error", "MACRO_EXPRESSION"}                                                          },

      {"MacroName",                   "MACRO_NAME",                 3u, {"{", "identifier", "}"}                                                                },
      {"MacroNameError",              "MACRO_NAME",                 2u, {"%error", "}"}                                                                         },

      {"MacroExpr",                   "MACRO_EXPRESSION",           2u, {"quote_cons", ";"}                                                                     },
      {"MacroExprError",              "MACRO_EXPRESSION",           2u, {"%error", ";"}                                                                         },

      // Expression
      {"ExprSectionList",             "EXPRESSION_SECTIONLIST",     2u, {"EXPRESSION_SECTION", "EXPRESSION_SECTIONLIST"}                                        },
      {"ExprSectionListEmpty",        "EXPRESSION_SECTIONLIST",     0u, {}                                                                                      },

      {"ExprSection",                 "EXPRESSION_SECTION",         3u, {"expression", "identifier", "EXPRESSION_BLOCK"}                                        },
      {"ExprSectionError",            "EXPRESSION_SECTION",         2u, {"expression", "%error"}                                                                },

      {"ExprBlock",                   "EXPRESSION_BLOCK",           2u, {"EXPRESSION_BLOCK","EXPRESSION_LINE"}                                                  },
      {"ExprBlockEmpty",              "EXPRESSION_BLOCK",           0u, {}                                                                                      },

      {"ExprLine",                    "EXPRESSION_LINE",            4u, {"quote_cons", "identifier", "OPT_ACTION", ";"}                                         },
      {"ExprLineAlias",               "EXPRESSION_LINE",            6u, {"quote_cons", "identifier", ",", "quote_cons", "OPT_ACTION", ";"}                      },
      {"ExprLineIgnore",              "EXPRESSION_LINE",            4u, {"quote_cons", "ignore", "OPT_ACTION",";"}                                              },
      {"ExprLineError",               "EXPRESSION_LINE",            2u, {"%error", ";"}                                                                         },

      {"OptActionPush",               "OPT_ACTION",                 3u, {",","push","identifier"}                                                               },
      {"OptActionPop",                "OPT_ACTION",                 2u, {",","pop"}                                                                             },
      {"OptActionGoto",               "OPT_ACTION",                 3u, {",","goto","identifier"}                                                               },
      {"OptActionNone",               "OPT_ACTION",                 0u, {}                                                                                      },

      // Precedence
      {"PrecSection",                 "PRECEDENCE_SECTION",         2u, {"prec","PRECEDENCE_BLOCK"}                                                             },
      {"PrecSectionError",            "PRECEDENCE_SECTION",         2u, {"prec","%error"}                                                                       },
      {"PrecSectionEmpty",            "PRECEDENCE_SECTION",         0u, {}                                                                                      },

      {"PrecBlock",                   "PRECEDENCE_BLOCK",           2u, {"PRECEDENCE_BLOCK","PRECEDENCE_LINE"}                                                  },
      {"PrecBlockEmpty",              "PRECEDENCE_BLOCK",           0u, {}                                                                                      },

      {"PrecTerminalListAssoc1",      "PRECEDENCE_LINE",            4u, {"OPT_NUM", "TERMINAL_LIST", "ASSOC", ";"}                                              },
      {"PrecTerminalListAssoc2",      "PRECEDENCE_LINE",            5u, {"OPT_NUM", "TERMINAL_LIST", ",", "ASSOC", ";"}                                         },
      {"PrecAssocTerminalList1",      "PRECEDENCE_LINE",            4u, {"OPT_NUM", "ASSOC", "TERMINAL_LIST", ";"}                                              },
      {"PrecAssocTerminalList2",      "PRECEDENCE_LINE",            5u, {"OPT_NUM", "ASSOC", ",", "TERMINAL_LIST", ";"}                                         },
      {"PrecError",                   "PRECEDENCE_LINE",            2u, {"%error", ";"}                                                                         },

      {"Num",                         "OPT_NUM",                    1u, {"number_int"}                                                                          },
      {"NumComma",                    "OPT_NUM",                    2u, {"number_int", ","}                                                                     },
      {"NumEmpty",                    "OPT_NUM",                    0u, {}                                                                                      },

      {"TerminalOne",                 "TERMINAL_LIST",              1u, {"TERMINAL"}                                                                            },
      {"TerminalList",                "TERMINAL_LIST",              2u, {"TERMINAL_LIST", "TERMINAL"}                                                           },
      {"TerminalListComma",           "TERMINAL_LIST",              3u, {"TERMINAL_LIST", ",", "TERMINAL"}                                                      },

      {"Terminal",                    "TERMINAL",                   1u, {"identifier"}                                                                          },
      {"TerminalQuote",               "TERMINAL",                   1u, {"quote_cons"}                                                                          },

      {"AssocLeft",                   "ASSOC",                      1u, {"left"}                                                                                },
      {"AssocRight",                  "ASSOC",                      1u, {"right"}                                                                               },
      {"AssocNon",                    "ASSOC",                      1u, {"nonassoc"}                                                                            },

      // Production
      {"ProdSection",                 "PRODUCTION_SECTION",         2u, {"PRODUCTION_STARTLIST", "PRODUCTION_BLOCK"}                                            },
      {"ProdSectionEmpty",            "PRODUCTION_SECTION",         0u, {}                                                                                      },

      {"ProdStartNameList",           "PRODUCTION_STARTLIST",       1u, {"PRODUCTION_STARTSYMBOL"}                                                              },
      {"ProdStartNameListNested",     "PRODUCTION_STARTLIST",       2u, {"PRODUCTION_STARTSYMBOL", "PRODUCTION_STARTLIST"}                                      },

      {"ProdStartSymbolDecl",         "PRODUCTION_STARTSYMBOL",     2u, {"production", "PRODUCTION_STARTSYMBOLLIST"}                                            },
      {"ProdStartSymbolDeclError",    "PRODUCTION_STARTSYMBOL",     2u, {"production", "%error"}                                                                },

      {"ProdStartSymbolListId",       "PRODUCTION_STARTSYMBOLLIST", 1u, {"identifier"}                                                                          },
      {"ProdStartSymbolListNested",   "PRODUCTION_STARTSYMBOLLIST", 3u, {"PRODUCTION_STARTSYMBOLLIST", ",", "identifier"}                                       },

      {"ProdBlock",                   "PRODUCTION_BLOCK",           2u, {"PRODUCTION_BLOCK","PRODUCTION_LINE"}                                                  },
      {"ProdBlockEmpty",              "PRODUCTION_BLOCK",           0u, {}                                                                                      },

      {"ProdLine",                    "PRODUCTION_LINE",            2u, {"PRODUCTION_LEFT","PRODUCTION_RIGHT"},                                                 },

      {"ProdLeft",                    "PRODUCTION_LEFT",            3u, {"identifier", "PRODUCTION_LHSNAMELIST", "->"},                                         },
      {"ProdLeftError",               "PRODUCTION_LEFT",            2u, {"%error", "->"},                                                                       },

      {"ProdLHSNameList",             "PRODUCTION_LHSNAMELIST",     1u, {"PRODUCTION_LHSNAME"},                                                                 },
      {"ProdLHSNameListNested",       "PRODUCTION_LHSNAMELIST",     3u, {"PRODUCTION_LHSNAMELIST", "|", "PRODUCTION_LHSNAME"},                                  },

      {"ProdLHSId",                   "PRODUCTION_LHSNAME",         1u, {"identifier"},                                                                         },
      {"ProdLHSNamedError",           "PRODUCTION_LHSNAME",         2u, {"identifier", "NAMED_ERROR"},                                                          },

      {"ProdRight",                   "PRODUCTION_RIGHT",           3u, {"PRODUCTION_RHS", "PRODUCTION_POSTFIX", ";"}                                           },
      {"ProdRightError",              "PRODUCTION_RIGHT",           2u, {"%error", ";"}                                                                         },

      {"ProdRHS",                     "PRODUCTION_RHS",             2u, {"PRODUCTION_RHS", "PRODUCTION_SYMBOLCOMBINE"}                                          },
      {"ProdRHSErrorToken",           "PRODUCTION_RHS",             2u, {"PRODUCTION_RHS", "PRODUCTION_ERROR"}                                                  },
      {"ProdRHSEmpty",                "PRODUCTION_RHS",             0u, {}                                                                                      },

      {"ProdSymbolCombineSingle",     "PRODUCTION_SYMBOLCOMBINE",   1u, {"PRODUCTION_FULLSYMBOL"}                                                               },
      {"ProdSymbolCombine",           "PRODUCTION_SYMBOLCOMBINE",   3u, {"PRODUCTION_SYMBOLCOMBINE", "|", "PRODUCTION_FULLSYMBOL"}                              },

      {"ProdFullSymbol",              "PRODUCTION_FULLSYMBOL",      2u, {"PRODUCTION_PREFIX", "PRODUCTION_SYMBOL"}                                              },
      {"ProdFullSymbolError",         "PRODUCTION_FULLSYMBOL",      2u, {"%error", "PRODUCTION_SYMBOL"}                                                         },

      {"ProdPrefixShift",             "PRODUCTION_PREFIX",          5u, {"PRODUCTION_PREFIX", "shifton", "(", "NONTERMINAL_LIST", ")"}                          },
      {"ProdPrefixShiftError",        "PRODUCTION_PREFIX",          5u, {"PRODUCTION_PREFIX", "shifton", "(", "%error", ")"}                                    },
      {"ProdPrefixReduce",            "PRODUCTION_PREFIX",          5u, {"PRODUCTION_PREFIX", "reduceon", "(", "NONTERMINAL_LIST", ")"}                         },
      {"ProdPrefixReduceError",       "PRODUCTION_PREFIX",          5u, {"PRODUCTION_PREFIX", "reduceon", "(", "%error", ")"}                                   },
      {"ProdPrefixEmpty",             "PRODUCTION_PREFIX",          0u, {}                                                                                      },

      {"ProdSymbolId",                "PRODUCTION_SYMBOL",          1u, {"identifier"}                                                                          },
      {"ProdSymbolQuote",             "PRODUCTION_SYMBOL",          1u, {"quote_cons"}                                                                          },
      {"ProdSymbolAction",            "PRODUCTION_SYMBOL",          4u, {"action", "(", "identifier", ")"}                                                      },
      {"ProdSymbolActionError",       "PRODUCTION_SYMBOL",          4u, {"action", "(", "%error", ")"}                                                          },

      {"ProdRHSError",                "PRODUCTION_ERROR",           1u, {"error"}                                                                               },
      {"ProdRHSErrorBacktrack",       "PRODUCTION_ERROR",           1u, {"error_backtrack"}                                                                     },
      {"ProdRHSErrorNamed",           "PRODUCTION_ERROR",           1u, {"NAMED_ERROR"}                                                                         },

      {"NonterminalListId",           "NONTERMINAL_LIST",           1u, {"identifier"}                                                                          },
      {"NonterminalListNested1",      "NONTERMINAL_LIST",           2u, {"NONTERMINAL_LIST", "identifier"}                                                      },
      {"NonterminalListNested2",      "NONTERMINAL_LIST",           3u, {"NONTERMINAL_LIST", ",", "identifier"}                                                 },

      {"ProdPostfixPrecReduce",       "PRODUCTION_POSTFIX",         2u, {"PRODUCTION_PREC", "PRODUCTION_REDUCE"}                                                },
      {"ProdPostfixReducePrec",       "PRODUCTION_POSTFIX",         2u, {"PRODUCTION_REDUCE", "PRODUCTION_PREC"}                                                },
      {"ProdPostfixReduce",           "PRODUCTION_POSTFIX",         1u, {"PRODUCTION_REDUCE"}                                                                   },
      {"ProdPostfixPrec",             "PRODUCTION_POSTFIX",         1u, {"PRODUCTION_PREC"}                                                                     },
      {"ProdPostfixEmpty",            "PRODUCTION_POSTFIX",         0u, {}                                                                                      },

      {"ProdPrecId",                  "PRODUCTION_PREC",            2u, {"prec", "identifier"}                                                                  },
      {"ProdPrecQuote",               "PRODUCTION_PREC",            2u, {"prec", "quote_cons"}                                                                  },

      {"ProdReduce",                  "PRODUCTION_REDUCE",          4u, {"reduce", "(", "REDUCE_LIST", ")"}                                                     },

      {"ReduceList",                  "REDUCE_LIST",                1u, {"REDUCE_EXPR"}                                                                         },
      {"ReduceListNested",            "REDUCE_LIST",                3u, {"REDUCE_LIST", ",", "REDUCE_EXPR"}                                                     },
      {"ReduceExpr",                  "REDUCE_EXPR",                1u, {"REDUCE_NONTERMINAL_LIST"}                                                             },
      {"ReduceExprWithTerminals",     "REDUCE_EXPR",                3u, {"REDUCE_NONTERMINAL_LIST", "&", "REDUCE_TERMINAL_LIST"}                                },

      {"ReduceNonterminalList",       "REDUCE_NONTERMINAL_LIST",    1u, {"identifier"}                                                                          },
      {"ReduceNonterminalListNested", "REDUCE_NONTERMINAL_LIST",    2u, {"REDUCE_NONTERMINAL_LIST", "identifier"}                                               },

      {"ReduceTerminalList",          "REDUCE_TERMINAL_LIST",       1u, {"TERMINAL"}                                                                            },
      {"ReduceTerminalListNested",    "REDUCE_TERMINAL_LIST",       2u, {"REDUCE_TERMINAL_LIST", "TERMINAL"}                                                    },

      {"NamedError",                  "NAMED_ERROR",                4u, {"error", "(", "identifier", ")"}                                                       },
      {"NamedErrorError",             "NAMED_ERROR",                4u, {"error", "(", "%error", ")"}                                                           }
    };

    // Make a list of productions
    for (size_t i = 0u; i < productionCount; ++i) {
        unsigned arr[6u];
        for (size_t j = 0u; j < init[i].Length; ++j)
            if (init[i].pRight[j])
                arr[j] = grammarSymbols[init[i].pRight[j]];

        productions.emplace_back(init[i].pName, grammarSymbols[init[i].pGrammarSymbol],
                                 arr, unsigned(init[i].Length));
    }
}


void StdGrammarParseData::CreateLexemes(std::vector<Lexeme>& lexemes) {
    // Create the hard-coded internal lexemes structure
    static constexpr size_t LexemeCount = 36u;
    static constexpr struct LexemeInit {
        const char* Name;
        const char* RegulaRegExpr;
    } init[LexemeCount]=
    {
      {"bad_char",         "[^ \r\n\t]"              },
      {"",                 "[ \t\r\n]+"              }, // whitespace
      {"number_int",       "[0-9]+"                  },
      {"string_cons",      "\"[^\"]*\""              },
      {"identifier",       "[A-Za-z_][A-Za-z0-9_]*"  },
      {"",                 "\\/\\/.*"                }, // single line comment
      {"macro",            "%macro"                  },

      {"expression",       "%expression"             },
      {"ignore",           "%ignore"                 },
      {"push",             "%push"                   },
      {"pop",              "%pop"                    },
      {"goto",             "%goto"                   },
 
      {"prec",             "%prec"                   },
      {"left",             "%left"                   },
      {"right",            "%right"                  },
      {"nonassoc",         "%nonassoc"               },

      {"production",       "%production"             },
      {"shifton",          "%shifton"                },
      {"reduceon",         "%reduceon"               },
      {"reduce",           "%reduce"                 },
      {"action",           "%action"                 },
      {"error",            "%error",                 },
      {"error_backtrack",  "%error_backtrack",       },

      {",",                ","                       },
      {";",                ";"                       },
      {"?",                "\\?"                     },
      {"&",                "\\&"                     },
      {"|",                "\\|"                     },
      {"{",                "\\{"                     },
      {"}",                "\\}"                     },
      {"(",                "\\("                     },
      {")",                "\\)"                     },
      {">",                ">"                       },
      {"->",               "\\-\\>"                  },
      {"quote_cons",       "\'((\'\')|[^\'\n\r])*\'" },

      // Expression from the web (old one did not seem to work):
      // /\*[^*]*\*+([^/*][^*]*\*+)*/
      {"",                 "\\/\\*[^\\*]*\\*+([^\\/\\*][^\\*]*\\*+)*\\/" }  // Multi-line comment
      // Improvements:
      // Ideally we have lazy loops (x*?): "/\\*.*?\\*/"
      // Second best, if we have possessive qualifiers (x*+):
      // "/\\*[^*]*+\\*++([^/][^*]*+\\*++)*+/"
      // Worst case, we can at least remove unnecessary backslashes:
      // "/\\*[^*]*\\*+([^/\\*][^*]*\\*+)*/"
    };

    unsigned idCount = TokenCode::TokenFirstID;
    lexemes.resize(LexemeCount);
    for (size_t i = 0u; i < LexemeCount; ++i) {
        lexemes[i].SetLexeme(init[i].Name, init[i].RegulaRegExpr, init[i].Name[0u] ? idCount : 0u);
        if (init[i].Name[0u])
            ++idCount;
    }
}


struct StdGrammarToken final : TokenCode
{
    using PosTracker      = LineOffsetPosTracker;
    using InputCharReader = TokenCharReaderBase<TokenizerBase::ByteReader, PosTracker>;
    using TokenCharReader = TokenCharReaderBase<TokenizerBase::BufferRangeByteReader,
                                                NullPosTracker>;
    using Tokenizer       = DFATokenizer<StdGrammarToken>;

    String Str;
    size_t Line   = 0u;
    size_t Offset = 0u;

    // Read-in from tokenizer function
    void CopyFromTokenizer(CodeType code, const Tokenizer& tokenizer) {
        Code            = code;
        const auto& pos = tokenizer.GetTokenPos();
        Line            = pos.Line;
        Offset          = pos.Offset;

        // Copy the token string
        auto creader    = tokenizer.GetTokenCharReader();

        Str.clear();
        while (!creader.IsEOF()) {
            Str += CharT(creader.GetChar());
            creader.Advance();
        }
    }
};


struct StdGrammarStackElement : ParseStackElement<StdGrammarToken> 
{
    // *** Data members

    enum DataType : unsigned
    {
        DataEmpty           = 0x0000'0000,
        DataVoid            = 0x1000'0000,
        DataString          = 0x2000'0000,
        DataStringVec       = 0x3000'0000,
        DataStackElement    = 0x4000'0000,
        DataStackElementVec = 0x5000'0000,
        DataTypeMask        = 0xF000'0000
    };

    unsigned Type = DataEmpty;

    union
    {
        void*                                pData            = nullptr;
        String*                              pString;
        std::vector<String>*                 pStringVec;
        StdGrammarStackElement*              pStackElement;
        std::vector<StdGrammarStackElement>* pStackElementVec;
    };

    size_t Line              = 0u;
    size_t Offset            = 0u;
    size_t MaxErrorStrLength = 18u;

    // *** Constructors

    StdGrammarStackElement() = default;

    explicit StdGrammarStackElement(String* pstring)
        : Type(DataString), pString(pstring) {
    }

    explicit StdGrammarStackElement(std::vector<String>* pstringVec)
        : Type(DataStringVec), pStringVec(pstringVec) {
    }

    explicit StdGrammarStackElement(StdGrammarStackElement* pstackElement)
        : Type(DataStackElement), pStackElement(pstackElement) {
    }

    explicit StdGrammarStackElement(std::vector<StdGrammarStackElement>* pstackElementVec)
        : Type(DataStackElementVec), pStackElementVec(pstackElementVec) {
    }

    // *** Override functions

    void ShiftToken(TokenType& token, [[maybe_unused]] TokenStream<TokenType>& stream) {
        // Always get the line and offset
        Line   = token.Line;
        Offset = token.Offset;
        SG_ASSERT(pData == nullptr);

        switch (token.Code) {
            case SGL_number_int:
            case SGL_identifier:
                pString = new String{token.Str};
                Type    = DataString;
                break;

            case SGL_quote_cons: {
                pString = new String;
                Type    = DataString;

                // Turn double '' to single ones
                // Copy the String but not the first and last quote characters
                for (size_t i = 1u; i + 1u < token.Str.size(); ++i) {
                    if (token.Str[i] == '\'' && token.Str[i + 1u] == '\'')
                        ++i;
                    *pString += token.Str[i];
                }
                break;
            }

            default:
                pData = nullptr;
                Type  = DataEmpty;
                break;
        }
    }

    // Set the error message information
    void SetErrorData([[maybe_unused]] TokenType& t, TokenStream<TokenType>& stream) {
        TokenType tok;
        stream.GetNextToken(tok);
        // Get Error token String
        SG_ASSERT(pData == nullptr);
        pString = new String{tok.Str};
        Type    = DataString;
        Line    = tok.Line;
        Offset  = tok.Offset;

        // Shorten the error to MaxErrorStrLength characters and add three dots
        if (pString->size() > MaxErrorStrLength)
            *pString = pString->substr(0u, MaxErrorStrLength) + "...";
    }

    // Delete any allocated data
    void Cleanup() {
        if (!pData)
            return;

        switch (Type & DataTypeMask) {
            case DataVoid:      SG_ASSERT(false);  break; // Shouldn't be here
            case DataString:    delete pString;    break;
            case DataStringVec: delete pStringVec; break;

            case DataStackElement:
                pStackElement->Cleanup();
                delete pStackElement;
                break;

            case DataStackElementVec:
                for (auto& se: *pStackElementVec)
                    se.Cleanup();
                delete pStackElementVec;
                break;
        }

        pData = nullptr;
    }

    // *** Custom functions

    // Returns whether or not the stack element is valid
    bool IsValid() const noexcept {
        return pData != nullptr;
    }

    // Set the maximum error string length
    void SetMaxErrorStringLength(size_t maxLength) noexcept {
        MaxErrorStrLength = maxLength;
    }
};


class StdGrammarParseHandler final : public ParseHandler<StdGrammarStackElement>
{
public:
    // Maintains a set of all error/warning/etc. messages
    ParseMessageBuffer          Messages;

    // Total number of errors encountered while parsing
    size_t                      ErrorCount      = 0u;

    // Macro block declaration line number
    size_t                      MacroBlock      = 0u;
    // Precedence block declaration line number
    size_t                      PrecedenceBlock = 0u;
    // Expression block name - declaration line number
    std::map<String, size_t>    ExpressionBlocks;

    // List of all the start symbol names in the order they appear in file
    std::vector<String>         OrderedStartSymbols;
    // Production start symbol - declaration line number
    std::map<String, size_t>    ProductionStartSymbols;
    // Macro name - declaration line number
    std::map<String, size_t>    MacroLines;
    // Expression alias name - declaration line number
    std::map<String, size_t>    ExpressionAliasLines;
    // Production labels - declaration line number
    std::map<String, size_t>    ProductionLines;
    // Token Code - declaration line number
    std::map<unsigned, size_t>  PrecedenceLines;

    // Maintains the largest precedence value while parsing
    unsigned                    PrecValue       = 0u;

    // Production string data
    struct ProductionData final
    {
        // Name (Label) of production
        String                  Name;
        // Left hand grammar symbols
        std::vector<String>     Left;
        // Right hand non terminal names
        std::vector<String>     Right;
        // Whether or not this production should report when reduced
        bool                    NotReported     = false;
    };

    // Array of productions
    std::vector<ProductionData> Productions;

    // Lex class used to store lexeme information
    Lex*                        pLex = nullptr;

    // Builds a group of right hand side productions (used when a '|' appears in the production)
    bool BuildProductionRHS(std::vector<std::vector<String>>& dest, StdGrammarStackElement& right);

    // Main parsing reduce function
    bool Reduce(Parse<StdGrammarStackElement>& parse, unsigned productionID) override;
};


// Expand right hand side productions
bool StdGrammarParseHandler::BuildProductionRHS(std::vector<std::vector<String>>& dest,
                                                StdGrammarStackElement& right) {
    // The right stack element must contain a array of stack elements
    if ((right.Type & StdGrammarStackElement::DataTypeMask) !=
        StdGrammarStackElement::DataStackElementVec)
        return false;

    // Helper struct
    struct SymbolGroup final
    {
        size_t Size    = 0u;
        size_t Counter = 0u;
    };

    // Group of symbol groups used to keep track of the
    std::vector<SymbolGroup> groups;

    // Determines the number of productions which will
    // be built from the data and sets up the groups correspondingly
    size_t prodCount = 1u;
    for (const auto& element: *right.pStackElementVec) {
        // If the sub stack element is also an array of stack elements than
        // create add a new symbol group to the groups array
        if (element.Type == StdGrammarStackElement::DataStackElementVec) {
            SymbolGroup g;
            // Initialize the group to the size of the element array
            g.Size     = element.pStackElementVec->size();
            g.Counter  = 0u;
            prodCount *= g.Size;
            // Add new symbol group to array
            groups.push_back(g);
        }
    }

    // Allocate the necessary number of production slots
    dest.resize(prodCount);

    size_t iprod    = 0u;
    size_t igroup   = groups.size() - 1u;
    size_t icounter = 0u;

    // Build the productions by traversing through
    // all right hand side values and productions
    while (iprod < prodCount) {
        if (icounter != 0u) {
            ++groups[igroup].Counter;

            if (groups[igroup].Counter == groups[igroup].Size) {
                groups[igroup].Counter = 0u;
                icounter = 1u;
                --igroup;
            } else {
                if (igroup != groups.size() - 1u)
                    ++igroup;
                icounter = 0u;
            }
        }
        // If all the counters have been checked than add the production
        else {
            // Group counter
            size_t ig = 0u;
            // Go through all right hand side stack elements
            for (auto& pstackElement: *right.pStackElementVec)
                // Determine what type of stack element it is and store data
                switch (pstackElement.Type) {
                    // Add the simple stack element string to the destination production
                    case StdGrammarStackElement::DataString:
                        dest[iprod].push_back(*pstackElement.pString);
                        break;

                        // Add the set of stack element strings
                    case StdGrammarStackElement::DataStringVec: {
                        // Add all the symbol strings
                        auto& destVec = dest[iprod];
                        destVec.insert(destVec.end(), pstackElement.pStringVec->begin(),
                                       pstackElement.pStringVec->end());
                        break;
                    }

                        // If it's a stack element std::vector it must
                    case StdGrammarStackElement::DataStackElementVec: {
                        auto& ptempElement =
                            (*pstackElement.pStackElementVec)[groups[ig++].Counter];
                        // Make sure it is the correct type
                        if ((ptempElement.Type & StdGrammarStackElement::DataTypeMask) !=
                            StdGrammarStackElement::DataStringVec)
                            return false;
                        // Add all the symbol strings
                        auto& destVec = dest[iprod];
                        destVec.insert(destVec.end(), ptempElement.pStringVec->begin(),
                                       ptempElement.pStringVec->end());
                        break;
                    }
                }

            icounter = 1u;
            ++iprod;
        }
    }

    return true;
}


// Main parse function parses the SG parser standard grammar file
bool StdGrammarParseHandler::Reduce(Parse<StdGrammarStackElement>& parse, unsigned productionID) {
    // Helper function to setup precedence
    const auto setupPrecedence = [&](size_t termlist, size_t assoc) {
        // Don't do anything if there was a sub error
        if (parse[termlist].IsValid() && parse[assoc].IsValid()) {
            Grammar::TerminalPrec termPrec;

            // Assign the associativity
            if (*parse[assoc].pString == "r")
                termPrec.Value = Grammar::TerminalPrec::Right;
            else if (*parse[assoc].pString == "l")
                termPrec.Value = Grammar::TerminalPrec::Left;
            else
                termPrec.Value = Grammar::TerminalPrec::NonAssoc;

            // Set the precedence value
            if (parse[0].pString) {
                const auto& str = *parse[0].pString;
                termPrec.Value |= StringToNumber<decltype(termPrec.Value)>(str);
                PrecValue       = termPrec.Value;
                // Cleanup the number string
                parse[0].Cleanup();
            } else
                termPrec.Value |= PrecValue;
            ++PrecValue;

            // Add the structure to the precedence map
            for (const auto& str: *parse[termlist].pStringVec) {
                const auto tokenCode = str[0u] == '\''
                                           ? pLex->LexemeAliasToToken[str.substr(1u)]
                                           : pLex->LexemeNameToToken[str];
                pLex->Precedence.insert_or_assign(tokenCode | ProductionMask::Terminal, termPrec);
            }
        } else
            parse[0].Cleanup();

        parse[termlist].Cleanup();
        parse[assoc].Cleanup();
    };

    // Check for an error and report if needed
    const auto checkForErrorAndReport = [&](size_t index, const String& code,
                                            const char* message...) {
        if (Messages.GetMessageFlags() & ParseMessageBuffer::MessageError) {
            va_list argList;
            va_start(argList, message);
            const auto messageStr = StringWithVAFormat(message, argList);
            va_end(argList);

            const ParseMessage msg{ParseMessage::ErrorMessage, code, messageStr, 0u,
                                   parse[index].Line, parse[index].Offset};
            Messages.AddMessage(msg);
        }
        ++ErrorCount;
        parse[index].Cleanup();
    };

    // Check for a warning and report if needed
    const auto checkForWarningAndReport = [&](size_t index, const String& code,
                                              const char* message...) {
        if (Messages.GetMessageFlags() & ParseMessageBuffer::MessageWarning) {
            va_list argList;
            va_start(argList, message);
            const auto messageStr = StringWithVAFormat(message, argList);
            va_end(argList);

            const ParseMessage msg{ParseMessage::WarningMessage, code, messageStr, 0u,
                                   parse[index].Line, parse[index].Offset};
            Messages.AddMessage(msg);
        }
        ++ErrorCount;
        parse[index].Cleanup();
    };

    // On a reduction we must modify the token stack by combining
    // the |beta| tokens just popped (|beta| = lastReducedProduction.Length)
    // into an object to represent the single nonterminal on the LHS
    // of the production

    switch (productionID) {
        // ROOT_BLOCK -> MACRO_SECTION EXPRESSION_SECTIONLIST PRECEDENCE_SECTION PRODUCTION_SECTION 
        case SG_RootBlock:
            break;

            // ROOT_BLOCK    -> %error
        case SG_RootBlockError:
            // ERROR: Syntax - In macro name declaration
            checkForErrorAndReport(0, "YC0024E", "Syntax error in root section. Unexpected '%s'",
                                   parse[0].pString->data());
            break;

            // ***** Macro

            // MACRO_SECTION  -> 'macro' MACRO_BLOCK
        case SG_MacroSection:
            // Store the macro block line number
            MacroBlock = parse[0].Line;
            break;

            // MACRO_SECTION  -> 'macro' %error
        case SG_MacroSectionError:
            checkForErrorAndReport(1, "YC0021E",
                             "Syntax error in macro block definition. Unexpected '%s'",
                                   parse[1].pString->data());
            break;

            // MACRO_SECTION  -> <empty>
        case SG_MacroSectionEmpty:
            break;

            // MACRO_BLOCK    -> MACRO_BLOCK MACRO_LINE
        case SG_MacroBlock:
            break;

            // MACRO_BLOCK    -> <empty>
        case SG_MacroBlockEmpty:
            break;

            // MACRO_LINE    -> MACRO_NAME MACRO_EXPRESSION
        case SG_MacroLine:
            // Don't do anything if there was a sub error
            if (parse[0].IsValid() && parse[1].IsValid()) {
                // Add to Macros map instead of Lexemes
                if (pLex->Macros.find(*parse[0].pString) == pLex->Macros.end()) {
                    // Store the macro expression
                    pLex->Macros[*parse[0].pString] = std::move(*parse[1].pString);
                    // Add the lexeme line number to a map
                    MacroLines.insert_or_assign(std::move(*parse[0].pString), parse[0].Line);
                } else
                    checkForErrorAndReport(0, "YC0027E", "Macro '%s' already defined on line %zu",
                                           parse[0].pString->data(),
                                           MacroLines[*parse[0].pString] + 1u);
            }
            // Cleanup
            parse[1].Cleanup();
            parse[0].Cleanup();
            break;

            // MACRO_LINE    -> %error MACRO_EXPRESSION
        case SG_MacroLineError:
            // Delete the expression if it did not have an error
            if (parse[1].IsValid())
                parse[1].Cleanup();

            // ERROR: Syntax - In macro name declaration
            checkForErrorAndReport(0, "YC0021E",
                                   "Syntax error in macro declaration. Unexpected '%s'",
                                   parse[0].pString->data());
            break;

            // MACRO_NAME    -> '%error' '}'
        case SG_MacroNameError:
            // ERROR: Syntax - In macro name declaration
            checkForErrorAndReport(0, "YC0024E", "Syntax error in macro name. Unexpected '%s'",
                                   parse[0].pString->data());
            break;

            // MACRO_NAME    -> '{' 'identifier' '}'
        case SG_MacroName:
            parse[0].pString = parse[1].pString;
            parse[0].Type    = StdGrammarStackElement::DataString;
            parse[1].pString = nullptr;
            break;

            // MACRO_EXPRESSION -> QUOTE ';'
        case SG_MacroExpr:
            break;

            // MACRO_EXPRESSION -> %error ';'
        case SG_MacroExprError:
            // ERROR: Syntax - In macro expression declaration
            checkForErrorAndReport(0, "YC0021E",
                                   "Syntax error in macro expression. Unexpected '%s'",
                                   parse[0].pString->data());
            break;

            // *** Expression

            // EXPRESSION_SECTIONLIST  -> EXPRESSION_SECTION EXPRESSION_SECTIONLIST
        case SG_ExprSectionList:
            break;

            // EXPRESSION_SECTIONLIST  -> <empty>
        case SG_ExprSectionListEmpty:
            parse[0].Cleanup();
            break;

            // EXPRESSION_SECTION  -> 'expression' 'identifier' EXPRESSION_BLOCK
        case SG_ExprSection: {
            // We are reducing by expression, so store its identifier
            // Calculate starting lexeme
            auto       str            = *parse[1].pString;
            const auto exprCount      = unsigned(pLex->Expressions.size());
            const auto startingLexeme = !pLex->Expressions.empty()
                                            ? pLex->Expressions.back().StartLexeme +
                                              pLex->Expressions.back().LexemeCount
                                            : 0u;
            const auto lexemeCount    = unsigned(pLex->Lexemes.size()) - startingLexeme;

            // Store expression name
            if (pLex->ExpressionNames.find(str) == pLex->ExpressionNames.end()) {
                pLex->ExpressionNames[str] = exprCount;
                // Store the expression
                ExpressionBlocks.insert_or_assign(std::move(str), parse[1].Line);
                pLex->Expressions.push_back({startingLexeme, lexemeCount});
            } else
                checkForErrorAndReport(1, "YC0030E",
                                       "Expression block '%s' already defined on line %zu",
                                       str.data(), ExpressionBlocks[str] + 1u);

            // Make sure there are regular expressions defined within the block
            if (lexemeCount == 0u)
                checkForWarningAndReport(1, "YC0030W",
                                         "No regular expressions defined within the '%s' expression block",
                                         str.data());

            parse[1].Cleanup();
            parse[0].Cleanup();
            break;
        }

        // EXPRESSION_SECTION  -> 'expression' %error
        case SG_ExprSectionError:
            checkForErrorAndReport(1, "YC0030E",
                                   "Syntax error in expression block. Unexpected '%s'",
                                   parse[1].pString->data());
            parse[0].Cleanup();
            break;

            // EXPRESSION_BLOCK --> EXPRESSION_BLOCK EXPRESSION_LINE
        case SG_ExprBlock:
            break;

            // EXPRESSION_BLOCK --> <empty>
        case SG_ExprBlockEmpty:
            break;

            // EXPRESSION_LINE --> QUOTE 'identifier' OPT_ACTION ';'
        case SG_ExprLine: {
            // Allow multiple lexemes to share token
            // Make that there is no lexeme with the same name yet
            // Add lexeme if it is not already there
            const auto& s0 = *parse[0].pString;
            const auto& s1 = *parse[1].pString;
            Lexeme lexeme;
            lexeme.SetLexeme(s1, s0);

            // Store action in lexeme
            switch ((*parse[2].pString)[0u]) {
                case 'n': // nothing
                    break;
                case 'p': // push
                    lexeme.Info.Action = LexemeInfo::ActionPush;
                    lexeme.ActionParam = parse[2].pString->substr(1u);
                    break;
                case 'o': // pop
                    lexeme.Info.Action = LexemeInfo::ActionPop;
                    break;
                case 'g': // goto
                    lexeme.Info.Action = LexemeInfo::ActionGoto;
                    lexeme.ActionParam = parse[2].pString->substr(1u);
                    break;
            }

            // Two different expressions can produce the same token code (generated based
            // on the lexeme name). So if the name is already in use, re-use the token code
            if (pLex->LexemeNameToToken.find(s1) != pLex->LexemeNameToToken.end())
                lexeme.Info.TokenCode = pLex->LexemeNameToToken[s1];
            else {
                lexeme.Info.TokenCode = unsigned(pLex->TokenLexemes.size()) +
                                        TokenCode::TokenFirstID;
                pLex->LexemeNameToToken[s1] = lexeme.Info.TokenCode;
                pLex->TokenLexemes.push_back(unsigned(pLex->Lexemes.size()));
            }

            // Store the lexeme
            pLex->Lexemes.push_back(std::move(lexeme));

            // Cleanup - We're done with the data
            parse[2].Cleanup();
            parse[1].Cleanup();
            parse[0].Cleanup();
            break;
        }

        // EXPRESSION_LINE --> QUOTE 'identifier' ',' QUOTE OPT_ACTION ';'
        case SG_ExprLineAlias: {
            // Allow multiple lexemes to share token
            // If alias is already there, it must map to the same token, so that
            // it is not ambiguous
            if (pLex->LexemeAliasToToken.find(*parse[3].pString) !=
                pLex->LexemeAliasToToken.end()) {
                const auto tokenCode   = pLex->LexemeAliasToToken[*parse[3].pString];
                const auto lexemeIndex = pLex->TokenLexemes[tokenCode - TokenCode::TokenFirstID];

                if (pLex->Lexemes[lexemeIndex].Name != *parse[1].pString) {
                    // If the alias is already in the hash, it's a duplicate definition error
                    checkForErrorAndReport(3, "YC0033E",
                                           "Expression alias '%s' already assigned on line %zu",
                                           parse[3].pString->data(),
                                           ExpressionAliasLines[*parse[3].pString] + 1u);
                    // Cleanup (parse[3] is already cleaned by checkForErrorAndReport)
                    parse[4].Cleanup();
                    parse[2].Cleanup();
                    parse[1].Cleanup();
                    parse[0].Cleanup();
                    break;
                }
            }

            // Add lexeme if it is not already there
            Lexeme lexeme;
            lexeme.SetLexeme(*parse[1].pString, *parse[0].pString);

            // Store action in lexeme
            switch ((*parse[4].pString)[0u]) {
                case 'n': // nothing
                    break;
                case 'p': // push
                    lexeme.Info.Action = LexemeInfo::ActionPush;
                    lexeme.ActionParam = parse[4].pString->substr(1u);
                    break;
                case 'o': // pop
                    lexeme.Info.Action = LexemeInfo::ActionPop;
                    break;
                case 'g': // goto
                    lexeme.Info.Action = LexemeInfo::ActionGoto;
                    lexeme.ActionParam = parse[4].pString->substr(1u);
                    break;
            }

            // Allow multiple lexemes to share token

            if (pLex->LexemeNameToToken.find(*parse[1].pString) != pLex->LexemeNameToToken.end())
                lexeme.Info.TokenCode = pLex->LexemeNameToToken[*parse[1].pString];
            else {
                // Record lexeme alias, name and token Id
                lexeme.Info.TokenCode = unsigned(pLex->TokenLexemes.size()) +
                                        TokenCode::TokenFirstID;
                pLex->LexemeNameToToken[*parse[1].pString] = lexeme.Info.TokenCode;
                // And store lexeme
                pLex->TokenLexemes.push_back(unsigned(pLex->Lexemes.size()));
            }

            pLex->LexemeAliasToToken[*parse[3].pString] = lexeme.Info.TokenCode;
            ExpressionAliasLines[*parse[3].pString]     = parse[3].Line;

            pLex->Lexemes.push_back(std::move(lexeme));

            // Cleanup the parse stack data
            parse[4].Cleanup();
            parse[3].Cleanup();
            parse[2].Cleanup();
            parse[1].Cleanup();
            parse[0].Cleanup();
            break;
        }

        // EXPRESSION_LINE --> QUOTE 'ignore' OPT_ACTION ';'
        case SG_ExprLineIgnore: {
            // Add lexeme if it is not already there
            Lexeme lexeme;
            lexeme.SetLexeme("", *parse[0].pString);

            // Store action in lexeme
            switch ((*parse[2].pString)[0u]) {
                case 'n': // nothing
                    break;
                case 'p': // push
                    lexeme.Info.Action = LexemeInfo::ActionPush;
                    lexeme.ActionParam = parse[2].pString->substr(1u);
                    break;
                case 'o': // pop
                    lexeme.Info.Action = LexemeInfo::ActionPop;
                    break;
                case 'g': // goto
                    lexeme.Info.Action = LexemeInfo::ActionGoto;
                    lexeme.ActionParam = parse[2].pString->substr(1u);
                    break;
            }

            // Store the lexeme
            pLex->Lexemes.push_back(std::move(lexeme));

            // Cleanup
            parse[2].Cleanup();
            parse[1].Cleanup();
            parse[0].Cleanup();
            break;
        }

            // EXPRESSION_LINE --> %error ';'
        case SG_ExprLineError:
            checkForErrorAndReport(0, "YC0031E", "Syntax error in expression. Unexpected '%s'",
                                   parse[0].pString->data());
            break;

            // OPT_ACTION -> 'comma' 'push' identifier
        case SG_OptActionPush:
            parse[0].pString = new String{"p" + *parse[2].pString};
            parse[0].Type    = StdGrammarStackElement::DataString;
            parse[2].Cleanup();
            break;

            // OPT_ACTION -> 'comma' 'pop'
        case SG_OptActionPop:
            parse[0].pString = new String{"o"};
            parse[0].Type    = StdGrammarStackElement::DataString;
            break;

            // OPT_ACTION -> 'comma' 'goto' identifier
        case SG_OptActionGoto:
            parse[0].pString = new String{"g" + *parse[2].pString};
            parse[0].Type    = StdGrammarStackElement::DataString;
            parse[2].Cleanup();
            break;

            // OPT_ACTION -> <empty>
        case SG_OptActionNone:
            // Use one character code to represent OptAction
            parse[0].pString = new String{"n"};
            parse[0].Type    = StdGrammarStackElement::DataString;
            break;

            // ***** Precedence

            // PRECEDENCE_SECTION  -> 'prec' PRECEDENCE_BLOCK
        case SG_PrecSection:
            PrecedenceBlock = parse[0].Line;
            break;

            // PRECEDENCE_SECTION  -> 'prec' %error
        case SG_PrecSectionError:
            checkForErrorAndReport(1, "YC0021E",
                                   "Syntax error in precedence block definition. Unexpected '%s'",
                                   parse[1].pString->data());
            break;

            // PRECEDENCE_SECTION  -> <empty>
        case SG_PrecSectionEmpty:
            parse[0].Cleanup();
            break;

            // PRECEDENCE_BLOCK    -> PRECEDENCE_BLOCK PRECEDENCE_LINE
        case SG_PrecBlock:
            break;

            // PRECEDENCE_BLOCK    -> <empty>
        case SG_PrecBlockEmpty:
            parse[0].Cleanup();
            break;

            // PRECEDENCE_LINE    -> OPT_NUM TERMINAL_LIST ASSOC ';'
        case SG_PrecTerminalListAssoc1:
            setupPrecedence(1u, 2u);
            break;

            // PRECEDENCE_LINE -> OPT_NUM TERMINAL_LIST ',' ASSOC ';'
        case SG_PrecTerminalListAssoc2:
            setupPrecedence(1u, 3u);
            break;

            // PRECEDENCE_LINE -> OPT_NUM ASSOC TERMINAL_LIST ';'
        case SG_PrecAssocTerminalList1:
            setupPrecedence(2u, 1u);
            break;

            // PRECEDENCE_LINE -> OPT_NUM ASSOC ',' TERMINAL_LIST ';'
        case SG_PrecAssocTerminalList2:
            setupPrecedence(3u, 1u);
            break;

            // PRECEDENCE_LINE    -> '%error' ';'
        case SG_PrecError:
            // ERROR: Syntax - In macro expression declaration
            checkForErrorAndReport(0, "YC0050E",
                                   "Syntax error in precedence declaration. Unexpected '%s'",
                                   parse[0].pString->data());
            break;

            // OPT_NUM -> 'number_int'
        case SG_Num:
            break;

            // OPT_NUM -> 'number_int' ','
        case SG_NumComma:
            break;

            // OPT_NUM -> <empty>
        case SG_NumEmpty:
            parse[0].Cleanup();
            break;

            // TERMINAL_LIST ->  TERMINAL
        case SG_TerminalOne:
            if (parse[0].pString) {
                const auto pstring  = parse[0].pString;
                parse[0].pStringVec = new std::vector<String>;
                parse[0].Type       = StdGrammarStackElement::DataStringVec;
                parse[0].pStringVec->push_back(std::move(*pstring));
                delete pstring;
            }
            break;

            // TERMINAL_LIST -> TERMIAL_LIST TERMINAL
        case SG_TerminalList:
            if (parse[1].pString) {
                parse[0].pStringVec->push_back(std::move(*parse[1].pString));
                parse[1].Cleanup();
            }
            break;

            // TERMINAL_LIST -> TERMINAL_LIST ',' TERMINAL
        case SG_TerminalListComma:
            if (parse[2].pString) {
                parse[0].pStringVec->push_back(std::move(*parse[2].pString));
                parse[2].Cleanup();
            }
            break;

            // TERMINAL -> 'identifier'
        case SG_Terminal: {
            const auto& str = *parse[0].pString;
            // Make sure the alias exists
            if (pLex->LexemeNameToToken.find(str) == pLex->LexemeNameToToken.end())
                checkForErrorAndReport(0, "YC0050E", "Expression '%s' not defined", str.data());
            else {
                // Check for duplicate precedence definition
                const auto tokenCode = pLex->LexemeNameToToken[str];
                if (PrecedenceLines.find(tokenCode) != PrecedenceLines.end())
                    checkForErrorAndReport(0, "YC0050E",
                                           "Precedence for expression '%s' already defined on line %zu",
                                           str.data(), PrecedenceLines[tokenCode] + 1u);
                else
                    parse[0].Cleanup();
            }
            break;
        }

            // TERMINAL -> QUOTE
        case SG_TerminalQuote: {
            auto& str = *parse[0].pString;
            // Make sure the alias exists
            if (pLex->LexemeAliasToToken.find(str) == pLex->LexemeAliasToToken.end())
                checkForErrorAndReport(0, "YC0050E", "Expression alias '%s' not defined",
                                       str.data());
            else {
                // Check for duplicate precedence definition
                const auto tokenCode = pLex->LexemeAliasToToken[str];
                if (PrecedenceLines.find(tokenCode) != PrecedenceLines.end())
                    checkForErrorAndReport(0, "YC0050E",
                                           "Precedence for expression '%s' already defined on line %zu",
                                           str.data(), PrecedenceLines[tokenCode] + 1u);
                else
                    // Add a "'" alias marker
                    str.insert(0u, "'");
            }
            break;
        }

            // PRECEDENCE_ASSOC -> 'left'
        case SG_AssocLeft:
            parse[0].pString = new String{"l"};
            parse[0].Type    = StdGrammarStackElement::DataString;
            break;

            // PRECEDENCE_ASSOC -> 'right'
        case SG_AssocRight:
            parse[0].pString = new String{"r"};
            parse[0].Type    = StdGrammarStackElement::DataString;
            break;

            // PRECEDENCE_ASSOC -> 'nonassoc'
        case SG_AssocNon:
            parse[0].pString = new String{"n"};
            parse[0].Type    = StdGrammarStackElement::DataString;
            break;

            // ***** Production

            // PRODUCTION_SECTION      -> PRODUCTION_STARTLIST PRODUCTION_BLOCK;
        case SG_ProdSection:
            break;

            // PRODUCTION_SECTION      -> ;
        case SG_ProdSectionEmpty:
            break;

            // PRODUCTION_STARTLIST      -> PRODUCTION_STARTSYMBOL;
        case SG_ProdStartNameList:
            break;

            // PRODUCTION_STARTLIST      -> PRODUCTION_STARTSYMBOL PRODUCTION_STARTLIST;
        case SG_ProdStartNameListNested:
            break;

            // PRODUCTION_STARTSYMBOL    -> 'production' PRODUCTION_STARTSYMBOLLIST;
        case SG_ProdStartSymbolDecl: {
            // Store the production start symbols
            for (auto& str: *parse[1].pStringVec)
                if (ProductionStartSymbols.find(str) == ProductionStartSymbols.end()) {
                    // Store the production block name
                    ProductionStartSymbols.insert_or_assign(str, parse[1].Line);
                    OrderedStartSymbols.push_back(std::move(str));
                } else {
                    checkForErrorAndReport(1, "YC0040E",
                                           "Production start symbol '%s' already defined on line %zu",
                                           str.data(), ProductionStartSymbols[str] + 1u);
                    break;
                }
            parse[1].Cleanup();
            break;
        }

        // PRODUCTION_STARTSYMBOL    -> 'production' %error;
        case SG_ProdStartSymbolDeclError:
            checkForErrorAndReport(1, "YC0021E",
                                   "Syntax error in production block definition. Unexpected '%s'",
                                   parse[1].pString->data());
            parse[0].Cleanup();
            break;

            // PRODUCTION_STARTSYMBOLLIST  -> 'identifier';
        case SG_ProdStartSymbolListId: {
            const auto pstring  = parse[0].pString;
            parse[0].pStringVec = new std::vector<String>;
            parse[0].Type       = StdGrammarStackElement::DataStringVec;
            parse[0].pStringVec->push_back(std::move(*pstring));
            delete pstring;
            break;
        }

        // PRODUCTION_STARTSYMBOLLIST  -> PRODUCTION_STARTSYMBOLLIST ',' 'identifier';
        case SG_ProdStartSymbolListNested:
            parse[0].pStringVec->push_back(std::move(*parse[2].pString));
            break;

            // PRODUCTION_BLOCK --> PRODUCTION_BLOCK PRODUCTION_LINE
        case SG_ProdBlock:
            break;

            // PRODUCTION_BLOCK --> <empty>
        case SG_ProdBlockEmpty:
            parse[0].Cleanup();
            break;

            // PRODUCTION_LINE --> PRODUCTION_LEFT PRODUCTION_RIGHT
        case SG_ProdLine:
            // Don't do anything if their was an error
            if (parse[0].IsValid() && parse[1].IsValid()) {
                const auto& label = (*parse[0].pStringVec)[0u];
                // Make sure the production label is unique and does not already exist
                if (ProductionLines.find(label) != ProductionLines.end())
                    // ERROR: If the production is already in the hash,
                    // it's a duplicate definition error
                    checkForErrorAndReport(0, "YC0044E",
                                           "Production '%s' - already defined on line %zu",
                                           label.data(), ProductionLines[label] + 1u);
                else {
                    ProductionData prod;
                    prod.Name        = label;
                    prod.NotReported = false;

                    // Build right hand side, if the '|' operator was
                    // used then create multiple productions
                    std::vector<std::vector<String>> right;
                    BuildProductionRHS(right, parse[1]);

                    prod.Left.assign(std::make_move_iterator(++(parse[0].pStringVec->begin())),
                                     std::make_move_iterator(parse[0].pStringVec->end()));

                    // Add each productions
                    for (const auto& vec: right) {
                        prod.Right = vec;
                        Productions.push_back(prod);
                    }

                    // Add the production line number to a map
                    ProductionLines.insert_or_assign(std::move(prod.Name), parse[0].Line);
                }
            }

            // Cleanup
            parse[1].Cleanup();
            parse[0].Cleanup();
            break;

        // PRODUCTION_LEFT --> 'identifier' PRODUCTION_LHSNAMELIST '->'
        case SG_ProdLeft: {
            const auto pstr = parse[0].pString;
            const auto pvec = parse[1].pStringVec;
            pvec->insert(pvec->begin(), std::move(*pstr));
            delete pstr;
            parse[0].pStringVec = pvec;
            parse[0].Type       = StdGrammarStackElement::DataStringVec;
            parse[1].pStringVec = nullptr;
            break;
        }

        // PRODUCTION_LEFT --> '%error' '->'
        case SG_ProdLeftError:
            // ERROR: Syntax - In macro expression declaration
            checkForErrorAndReport(0, "YC0042E",
                                   "Syntax error in LHS of production. Unexpected '%s'",
                                   parse[0].pString->data());
            break;

            // PRODUCTION_LHSNAMELIST --> PRODUCTION_LHSNAME
        case SG_ProdLHSNameList:
            break;

            // PRODUCTION_LHSNAMELIST --> PRODUCTION_LHSNAMELIST '|' PRODUCTION_LHSNAME;
        case SG_ProdLHSNameListNested: {
            auto& src  = *parse[2].pStringVec;
            auto& dest = *parse[0].pStringVec;
            dest.insert(dest.end(), std::make_move_iterator(src.begin()),
                        std::make_move_iterator(src.end()));
            parse[2].Cleanup();
            break;
        }

        // PRODUCTION_LHSNAME --> 'identifier'
        case SG_ProdLHSId: {
            const auto pstringVec = new std::vector<String>;
            pstringVec->push_back(std::move(*parse[0].pString));
            delete parse[0].pString;
            parse[0].pStringVec = pstringVec;
            parse[0].Type       = StdGrammarStackElement::DataStringVec;
            break;
        }

        // PRODUCTION_LHSNAME --> 'identifier' NAMED_ERROR;
        case SG_ProdLHSNamedError: {
            const auto pstringVec = new std::vector<String>;
            // Add the identifier
            pstringVec->push_back(std::move(*parse[0].pString));
            delete parse[0].pString;
            // If the named error was valid than add it
            if (parse[1].pString) {
                pstringVec->push_back(std::move(*parse[1].pString));
                parse[1].Cleanup();
            }
            parse[0].pStringVec = pstringVec;
            parse[0].Type       = StdGrammarStackElement::DataStringVec;
            break;
        }

        // PRODUCTION_RIGHT --> PRODUCTION_RHS PRODUCTION_POSTFIX ';'
        case SG_ProdRight:
            // Add the postfix if it exists
            if (parse[1].pStringVec) {
                parse[0].pStackElementVec->emplace_back(parse[1].pStringVec);
                parse[1].pStringVec = nullptr;
            }
            break;

            // PRODUCTION_RIGHT --> '%error' ';'
        case SG_ProdRightError:
            // ERROR: Syntax - In macro expression declaration
            checkForErrorAndReport(0, "YC0043E",
                                   "Syntax error in RHS of production. Unexpected '%s'",
                                   parse[0].pString->data());
            break;

            // PRODUCTION_RHS -> PRODUCTION_RHS PRODUCTION_SYMBOLCOMBINE
        case SG_ProdRHS:
            // Always push the elements from the symbol combine
            if (parse[1].pData) {
                if (parse[1].Type == StdGrammarStackElement::DataStackElementVec)
                    parse[0].pStackElementVec->emplace_back(parse[1].pStackElementVec);
                else
                    parse[0].pStackElementVec->emplace_back(parse[1].pStringVec);
                parse[1].pData = nullptr; // Transferred data
            }
            break;

            // PRODUCTION_RHS --> PRODUCTION_RHS PRODUCTION_ERROR
        case SG_ProdRHSErrorToken:
            parse[0].pStackElementVec->emplace_back(parse[1].pString);
            parse[1].pString = nullptr;
            break;

            // PRODUCTION_RHS --> <empty>
        case SG_ProdRHSEmpty:
            // Make a new std::vector
            parse[0].pStackElementVec = new std::vector<StdGrammarStackElement>;
            parse[0].Type             = StdGrammarStackElement::DataStackElementVec;
            break;

            // PRODUCTION_SYMBOLCOMBINE    -> PRODUCTION_FULLSYMBOL;
        case SG_ProdSymbolCombineSingle:
            break;

            // PRODUCTION_SYMBOLCOMBINE    -> PRODUCTION_SYMBOLCOMBINE '|' PRODUCTION_FULLSYMBOL;
        case SG_ProdSymbolCombine:
            // Create a std::vector of stack elements if it is not already
            if (parse[0].Type == StdGrammarStackElement::DataStringVec) {
                // Save the original string std::vector
                const auto pstringVec     = parse[0].pStringVec;
                // Create a new stack element std::vector
                parse[0].pStackElementVec = new std::vector<StdGrammarStackElement>;
                parse[0].Type             = StdGrammarStackElement::DataStackElementVec;
                // Add the String std::vector as a new element
                parse[0].pStackElementVec->emplace_back(pstringVec);
            }

            // Add the full symbol string std::vector
            parse[0].pStackElementVec->emplace_back(parse[2].pStringVec);
            parse[2].pStringVec = nullptr;
            break;

        // PRODUCTION_FULLSYMBOL    -> PRODUCTION_PREFIX PRODUCTION_SYMBOL;
        case SG_ProdFullSymbol:
            if (!parse[0].pStringVec) {
                parse[0].pStringVec = new std::vector<String>;
                parse[0].Type       = StdGrammarStackElement::DataStringVec;
            }

            // Add the production symbol
            if (parse[1].pString) {
                parse[0].pStringVec->insert(parse[0].pStringVec->begin(),
                                            std::move(*parse[1].pString));
                // Must release string because it is copied into array by value.
                parse[1].Cleanup();
            }
            break;

            // PRODUCTION_FULLSYMBOL    -> %error PRODUCTION_SYMBOL;
        case SG_ProdFullSymbolError:
            // Generate error message
            checkForErrorAndReport(0, "YC0045E",
                                   "Syntax error in production symbol prefix. Unexpected '%s'",
                                   parse[0].pString->data());
            // Disregard the production symbol
            parse[1].Cleanup();
            break;

            // PRODUCTION_PREFIX      -> PRODUCTION_PREFIX 'shifton' '(' NONTERMINAL_LIST ')';
        case SG_ProdPrefixShift:
            if (!parse[0].pStringVec) {
                parse[0].pStringVec = new std::vector<String>;
                parse[0].Type       = StdGrammarStackElement::DataStringVec;
            }

            // Use special '>' marker for %shifton
            for (const auto& str: *parse[3].pStringVec)
                parse[0].pStringVec->push_back(">" + str);
            // Cleanup
            parse[3].Cleanup();
            break;

        // PRODUCTION_PREFIX      -> PRODUCTION_PREFIX 'shifton' '(' %error ')';
        case SG_ProdPrefixShiftError:
            checkForErrorAndReport(3, "YC0045E",
                                   "Syntax error in shifton declaration. Unexpected '%s'",
                                   parse[3].pString->data());
            parse[0].Cleanup();
            break;

            // PRODUCTION_PREFIX      -> PRODUCTION_PREFIX 'reduceon' '(' NONTERMINAL_LIST ')';
        case SG_ProdPrefixReduce:
            if (!parse[0].pStringVec) {
                parse[0].pStringVec = new std::vector<String>;
                parse[0].Type       = StdGrammarStackElement::DataStringVec;
            }
            // Use special '<' marker for %reduceon
            for (const auto& str: *parse[3].pStringVec)
                parse[0].pStringVec->push_back("<" + str);
            // Cleanup
            parse[3].Cleanup();
            break;

        // PRODUCTION_PREFIX      -> PRODUCTION_PREFIX 'reduceon' '(' %error ')';
        case SG_ProdPrefixReduceError:
            checkForErrorAndReport(3, "YC0045E",
                                   "Syntax error in reduceon declaration. Unexpected '%s'",
                                   parse[3].pString->data());
            parse[0].Cleanup();
            break;

            // PRODUCTION_PREFIX       -> <empty>
        case SG_ProdPrefixEmpty:
            parse[0].Cleanup();
            break;

            // PRODUCTION_SYMBOL      -> 'identifier';
        case SG_ProdSymbolId:
            break;

            // PRODUCTION_SYMBOL      -> 'quote_cons';
        case SG_ProdSymbolQuote:
            parse[0].pString->insert(0u, "'");
            break;

            // PRODUCTION_SYMBOL      -> 'action' '(' 'identifier' ')';
        case SG_ProdSymbolAction: {
            parse[0].pString = parse[2].pString;
            parse[0].Type    = StdGrammarStackElement::DataString;
            parse[2].pString = nullptr;
            const auto& str  = *parse[0].pString;

            // Make sure the production label is unique and does not already exist
            if (ProductionLines.find(str) != ProductionLines.end())
                // ERROR: If the production is already in the hash,
                // it's a duplicate definition error
                checkForErrorAndReport(0, "YC0044E",
                                       "Production '%s' - already defined on line %zu",
                                       str.data(), ProductionLines[str] + 1u);
            else {
                // Insert new 'empty' production
                ProductionData prod;
                prod.Name = str;
                prod.Left.push_back(str);

                // Add the production to the list
                Productions.push_back(prod);
                // Add the production line number to a map
                ProductionLines.insert_or_assign(prod.Name, parse[0].Line);
            }
            break;
        }

        // PRODUCTION_SYMBOL    -> 'action' '(' %error ')';
        case SG_ProdSymbolActionError:
            checkForErrorAndReport(2, "YC0045E",
                                   "Syntax error in action declaration. Unexpected '%s'",
                                   parse[2].pString->data());
            parse[0].Cleanup();
            break;

            // PRODUCTION_ERROR      -> 'error';
        case SG_ProdRHSError:
            // Special error token
            parse[0].pString = new String{"%error"};
            parse[0].Type    = StdGrammarStackElement::DataString;
            break;

            // PRODUCTION_ERROR      -> BACKTRACK_ERROR;
        case SG_ProdRHSErrorBacktrack:
            // Special error backtrack name
            parse[0].pString = new String{"%error_backtrack"};
            parse[0].Type    = StdGrammarStackElement::DataString;
            break;

            // PRODUCTION_ERROR      -> NAMED_ERROR;
        case SG_ProdRHSErrorNamed:
            break;

            // NONTERMINAL_LIST -> 'identifier'
        case SG_NonterminalListId: {
            const auto pstring  = parse[0].pString;
            parse[0].pStringVec = new std::vector<String>;
            parse[0].Type       = StdGrammarStackElement::DataStringVec;
            parse[0].pStringVec->push_back(std::move(*pstring));
            delete pstring;
            break;
        }

            // NONTERMINAL_LIST -> NONTERMINAL_LIST 'identifier'
        case SG_NonterminalListNested1:
            parse[0].pStringVec->push_back(std::move(*parse[1].pString));
            parse[1].Cleanup();
            break;

            // NONTERMINAL_LIST -> NONTERMINAL_LIST ',' 'identifier'
        case SG_NonterminalListNested2:
            parse[0].pStringVec->push_back(std::move(*parse[2].pString));
            parse[2].Cleanup();
            break;

            // PRODUCTION_POSTFIX      -> PRODUCTION_PREC PRODUCTION_REDUCE
        case SG_ProdPostfixPrecReduce:
            // Add the precedence String to the String std::vector
            if (parse[0].pString) {
                const auto pstring      = parse[0].pString;
                // Create a new String std::vector
                if (!parse[1].pStringVec)
                    parse[0].pStringVec = new std::vector<String>;
                // Or use the reduce String std::vector
                else {
                    parse[0].pStringVec = parse[1].pStringVec;
                    parse[1].pStringVec = nullptr;
                }
                // Add the precedence to the beginning of the array - It MUST come first
                parse[0].pStringVec->insert(parse[0].pStringVec->begin(), std::move(*pstring));
                delete pstring;
            } else {
                parse[0].pStringVec = parse[1].pStringVec;
                parse[1].pStringVec = nullptr;
            }

            parse[0].Type = StdGrammarStackElement::DataStringVec;
            break;

            // PRODUCTION_POSTFIX      -> PRODUCTION_REDUCE PRODUCTION_PREC
        case SG_ProdPostfixReducePrec:
            // Add the precedence String if it is valid
            if (parse[1].pString) {
                // Create the String std::vector if it is not already done
                if (!parse[0].pStringVec) {
                    parse[0].pStringVec = new std::vector<String>;
                    parse[0].Type       = StdGrammarStackElement::DataStringVec;
                }
                // Add the precedence to the beginning of the array - It MUST come first
                parse[0].pStringVec->insert(parse[0].pStringVec->begin(),
                                            std::move(*parse[1].pString));
                parse[1].Cleanup();
            }
            break;

            // PRODUCTION_POSTFIX      -> PRODUCTION_REDUCE
        case SG_ProdPostfixReduce:
            break;

            // PRODUCTION_POSTFIX      -> PRODUCTION_PREC
        case SG_ProdPostfixPrec:
            // If the precedence was recorded correctly
            if (parse[0].pString) {
                const auto pstring  = parse[0].pString;
                parse[0].pStringVec = new std::vector<String>;
                parse[0].Type       = StdGrammarStackElement::DataStringVec;
                parse[0].pStringVec->push_back(std::move(*pstring));
                delete pstring;
            }
            break;

            // PRODUCTION_POSTFIX      -> <empty>
        case SG_ProdPostfixEmpty:
            parse[0].Cleanup();
            break;

            // PRODUCTION_PREC --> %prec 'identifier'
        case SG_ProdPrecId:
            // Make sure the alias exists
            if (pLex->LexemeNameToToken.find(*parse[1].pString) == pLex->LexemeNameToToken.end()) {
                checkForErrorAndReport(1, "YC0045E", "Expression '%s' not defined",
                                       parse[1].pString->data());
                parse[0].Cleanup();
            } else {
                // Store the String
                parse[0].pString = parse[1].pString;
                parse[1].pString = nullptr;
                parse[0].Type    = StdGrammarStackElement::DataString;
                // Add the '$' marker for precedence
                parse[0].pString->insert(0u, "$");
            }
            break;

            // PRODUCTION_PREC    -> %prec QUOTE
        case SG_ProdPrecQuote:
            // Make sure the alias exists
            if (pLex->LexemeAliasToToken.find(*parse[1].pString) ==
                pLex->LexemeAliasToToken.end()) {
                checkForErrorAndReport(1, "YC0045E", "Expression alias '%s' not defined",
                                       parse[1].pString->data());
                parse[0].Cleanup();
            } else {
                // Store the String
                parse[0].pString = parse[1].pString;
                parse[1].pString = nullptr;
                parse[0].Type    = StdGrammarStackElement::DataString;
                // Add the '$' marker for precedence and add a "'" alias marker
                parse[0].pString->insert(0u, "$'");
            }
            break;

            // PRODUCTION_REDUCE      -> 'reduce' '(' REDUCE_LIST ')'
        case SG_ProdReduce:
            parse[0].pStringVec = parse[2].pStringVec;
            parse[0].Type       = StdGrammarStackElement::DataStringVec;
            parse[2].pStringVec = nullptr;
            break;

            // REDUCE_LIST          -> REDUCE_EXPR
        case SG_ReduceList:
            break;

            // REDUCE_LIST          -> REDUCE_LIST ',' REDUCE_EXPR
        case SG_ReduceListNested:
            if (!parse[0].pStringVec) {
                parse[0].pStringVec = new std::vector<String>;
                parse[0].Type       = StdGrammarStackElement::DataStringVec;
            }

            if (parse[2].pStringVec) {
                auto& src  = *parse[2].pStringVec;
                auto& dest = *parse[0].pStringVec;
                dest.insert(dest.end(), std::make_move_iterator(src.begin()),
                            std::make_move_iterator(src.end()));
                parse[2].Cleanup();
            }
            break;

            // REDUCE_EXPR          -> REDUCE_NONTERMINAL_LIST
        case SG_ReduceExpr:
            // Add a marker to specify the beginning of the nonterminal list
            if (parse[0].pStringVec)
                (*parse[0].pStringVec)[0u].insert(0u, "*");
            break;

            // REDUCE_EXPR          -> REDUCE_NONTERMINAL_LIST '&' REDUCE_TERMINAL_LIST
        case SG_ReduceExprWithTerminals:
            // Add a marker to specify the beginning of the nonterminal list
            if (parse[0].pStringVec)
                (*parse[0].pStringVec)[0u].insert(0u, "*");
            else {
                parse[0].pStringVec = new std::vector<String>;
                parse[0].Type       = StdGrammarStackElement::DataStringVec;
            }

            // Add all the terminals
            if (parse[2].pStringVec) {
                auto& src  = *parse[2].pStringVec;
                auto& dest = *parse[0].pStringVec;
                dest.insert(dest.end(), std::make_move_iterator(src.begin()),
                            std::make_move_iterator(src.end()));
                parse[2].Cleanup();
            }
            break;

            // REDUCE_NONTERMINAL_LIST    -> 'identifier'
        case SG_ReduceNonterminalList:
            if (parse[0].pString) {
                // Add a plus to specify that it is a terminal
                const auto pstring  = parse[0].pString;
                parse[0].pStringVec = new std::vector<String>;
                parse[0].Type       = StdGrammarStackElement::DataStringVec;
                parse[0].pStringVec->push_back(std::move(*pstring));
                delete pstring;
            }
            break;

            // REDUCE_NONTERMINAL_LIST    -> REDUCE_NONTERMINAL_LIST 'identifier'
        case SG_ReduceNonterminalListNested:
            if (parse[1].pString) {
                // Add a plus to specify that it is a terminal
                parse[0].pStringVec->push_back(std::move(*parse[1].pString));
                parse[1].Cleanup();
            }
            break;

            // REDUCE_TERMINAL_LIST      -> TERMINAL
        case SG_ReduceTerminalList:
            if (parse[0].pString) {
                // Add a plus to specify that it is a terminal
                parse[0].pString->insert(0u, "+");
                const auto pstring  = parse[0].pString;
                parse[0].pStringVec = new std::vector<String>;
                parse[0].Type       = StdGrammarStackElement::DataStringVec;
                parse[0].pStringVec->push_back(std::move(*pstring));
                delete pstring;
            }
            break;

            // REDUCE_TERMINAL_LIST      -> REDUCE_TERMINAL_LIST TERMINAL
        case SG_ReduceTerminalListNested:
            if (parse[1].pString) {
                // Add a plus to specify that it is a terminal
                parse[1].pString->insert(0u, "+");
                parse[0].pStringVec->push_back(std::move(*parse[1].pString));
                parse[1].Cleanup();
            }
            break;

            // NAMED_ERROR      -> 'error' '(' 'identifier' ')';
        case SG_NamedError:
            // Special named error token
            parse[0].pString = new String{"%error(" + *parse[2].pString + ")"};
            parse[0].Type    = StdGrammarStackElement::DataString;
            break;

            // NAMED_ERROR      -> 'error' '(' %error ')';
        case SG_NamedErrorError:
            checkForErrorAndReport(2, "YC0045E",
                                   "Syntax error in named error declaration. Unexpected '%s'",
                                   parse[2].pString->data());
            break;

        default:
            parse[0].Cleanup();
            break;
    }

    return true;
}


// Build the parser
bool StdGrammarParseData::BuildParser(InputStream* pinitStr) {
    // *** Internal Standard Grammar parser data

    Lex                           sgLex;
    DFATokenizer<StdGrammarToken> sgTokenizer;
    Parse<StdGrammarStackElement> sgParser;
    Grammar                       sgGrammar;
    StdGrammarParseHandler        sgParseHandler;

    std::vector<Production>       productionVC;
    std::map<String, unsigned>    grammarSymbolVC;

    // *** Load grammar

    if (!StdGrammarDFA.IsValid()) {
        // Setup the lexemes list with the hard-coded values
        CreateLexemes(sgLex.Lexemes);

        // Create a VPP DFA
        if (!sgLex.MakeDFA(StdGrammarDFA))
            return false;
    }

    if (!StdGrammarParseTable.IsValid()) {
        // Load the hard coded grammar... for the SG parser Standard Grammar file format
        CreateVectors(grammarSymbolVC, productionVC);

        // Create the grammar
        sgGrammar.Create(grammarSymbolVC, productionVC);

        // Create a parse table
        sgGrammar.GetMessageBuffer().SetMessageBuffer(Messages.GetMessageBuffer(),
                                                      Messages.GetMessageFlags());
        sgGrammar.GetDebugData().Flags |= GrammarDebugData::Canonical;

        if (!StdGrammarParseTable.Create(sgGrammar, ParseTableType::CLR))
            return false;
    }

    // *** Parse grammar

    // Create the parse class
    if (!sgParser.Create(&StdGrammarParseTable))
        return false;

    // Setup the tokenizer
    if (!sgTokenizer.Create(&StdGrammarDFA, pinitStr))
        return false;

    // Setup the parse class
    sgParser.SetTokenStream(&sgTokenizer);

    // Setup the parse handler
    sgParseHandler.pLex = &UserLex;
    sgParseHandler.Messages.SetMessageBuffer(Messages.GetMessageBuffer(),
                                             Messages.GetMessageFlags());

    // Parse the grammar
    const auto result = sgParser.DoParse(sgParseHandler);

    // Errors were encountered while parsing the command line input
    if (!result || sgParseHandler.ErrorCount > 0u)
        return false;

    // Converts action parameters into numbers
    UserLex.ConvertActionParam();

    std::map<String, unsigned> grammarSymbols;
    std::vector<Production>    productions;
    std::vector<unsigned>      startSymbols;

    // Create the grammar symbol map and production array from the productions data
    if (!CreateSymbolMap(sgParseHandler, grammarSymbols, productions, startSymbols))
        return false;

    UserGrammar.Clear();

    // Setup grammar debug and buffer
    UserGrammar.GetMessageBuffer().SetMessageBuffer(Messages.GetMessageBuffer(), 
                                                    Messages.GetMessageFlags());

    // Now 'grammarSymbols' and 'productions' contain the
    // information needed to make the parser and tokenizer
    UserGrammar.SetPrecedence(UserLex.Precedence);
    UserGrammar.SetStartSymbols(startSymbols);

    UserGrammar.AddGrammarSymbols(grammarSymbols);
    UserGrammar.AddProductions(productions);

    return true;
}


// Go through all of the productions we collected from the file and
// map all of the symbolic names to numbers that the parser can use
bool StdGrammarParseData::CreateSymbolMap(StdGrammarParseHandler& data,
                                          std::map<String, unsigned>& symbols,
                                          std::vector<Production>& productions,
                                          std::vector<unsigned>& startSymbols) {
    size_t errorCount = 0u;

    // Check for an error and report if needed
    const auto checkForErrorAndReport = [&](size_t line, const char* text...) {
        if (Messages.GetMessageFlags() & ParseMessageBuffer::MessageError) {
            va_list argList;
            va_start(argList, text);
            const auto msgText = StringWithVAFormat(text, argList);
            va_end(argList);

            const ParseMessage msg{ParseMessage::ErrorMessage, "", msgText, 0u, line};
            Messages.AddMessage(msg);
        }
        ++errorCount;
    };

    std::map<String, unsigned> nonTerminals;

    // NonTerminal id counter
    auto idCounter         = 0u;
    // Terminal id counter - extra terminals will be added to handle %error(Name)
    auto terminalIdCounter = unsigned(UserLex.TokenLexemes.size()) + TokenCode::TokenFirstID;

    for (auto& prod: data.Productions) {
        Production                             p;
        std::vector<unsigned>                  rhs;
        std::map<unsigned, std::set<unsigned>> reduceData;
        unsigned symbolId;
        unsigned precValue   = 0u;
        bool     reduceValue = false;
        size_t   iright;
        auto&    left        = prod.Left;
        auto&    right       = prod.Right;
        size_t   line        = data.ProductionLines[prod.Name];

        // Form right hand side list
        for (iright = 0u; iright < right.size(); ++iright) {
            bool  prec   = false;
            auto& symbol = right[iright];

            // Check to see if it is the beginning of the reduce overrides
            if (symbol[0u] == '*') {
                reduceValue = true;
                break;
            }

            // Check for precedence
            if (symbol[0u] == '$') {
                // Eat precedence marker '$'
                symbol = symbol.substr(1u);
                prec   = true;
            }

            // Search to see if it is a terminal
            symbolId = NFA::InvalidLexeme;
            // Terminal alias references will have special ' marker up front
            if (symbol[0u] == '\'') {
                // Eat alias marker quote
                symbol = symbol.substr(1u);

                if (UserLex.LexemeAliasToToken.find(symbol) != UserLex.LexemeAliasToToken.end())
                    symbolId = UserLex.LexemeAliasToToken[symbol];
                else
                    // ERROR: Undefined alias referenced
                    checkForErrorAndReport(line, "Undefined alias referenced - '%s'",
                                           symbol.data());
            } else {
                // If symbol has a % in front of it, it must be one of %error( ) directives
                if (symbol[0u] == '%' && symbol[6u] == '(') {
                    // Must be %error(Name)
                    // Make an error terminal, and reference it
                    // Assign it an Id, so it will be stored as a terminal in Symbols
                    if (symbols.find(symbol) == symbols.end())
                        symbolId = terminalIdCounter++ | ProductionMask::ErrorTerminal;
                    else
                        symbolId = symbols[symbol];
                } else {
                    // Standard terminal processing
                    // See if this is a terminal name declared in expressions
                    if (UserLex.LexemeNameToToken.find(symbol) != UserLex.LexemeNameToToken.end())
                        symbolId = UserLex.LexemeNameToToken[symbol];
                    // Special terminal - error, code TokenCode::TCode::TokenError
                    else if (symbol == "%error")
                        symbolId = TokenCode::TokenError;
                    else if (symbol == "%error_backtrack")
                        symbolId = TokenCode::TokenError | ProductionMask::BacktrackError;
                    else {} // Otherwise it is an error
                }
            }

            // Read in any conflict actions
            bool conflictActionsError = false;

            while (iright + 1u < right.size() &&
                   (right[iright + 1u][0u] == '>' || right[iright + 1u][0u] == '<')) {
                ++iright;
                // Terminal must follow %shifton, %reduceon
                if (symbolId != NFA::InvalidLexeme) {
                    // Get nonterminal id
                    unsigned   id;
                    const auto index = rhs.size();
                    const auto s     = right[iright].substr(1u);

                    // Get Nonterminal Id
                    // If this nonterminal wasn't seen yet, assign it an id
                    if (nonTerminals.find(s) == nonTerminals.end())
                        nonTerminals[s] = idCounter++;
                    id = nonTerminals[s];

                    // Make sure there isn't an action on this nonterminal already
                    Production::ConflictAction & action = p.ConflictActions[unsigned(index)];
                    if (action.Actions.find(id) != action.Actions.end())
                        // Error: Multiple conflict
                        checkForErrorAndReport(line,
                                               "Multiple conflict actions on nonterminal %s before '%s'",
                                               s.data(), symbol.data());
                    else
                        // Store action
                        action.Actions[id] = right[iright][0u] == '>'
                                                 ? Production::ConflictAction::Action::Shift
                                                 : Production::ConflictAction::Action::Reduce;
                } else
                    // Conflict actions can only appear before a terminal
                    conflictActionsError = true;
            }

            // Conflict actions defined for nonterminal
            if (conflictActionsError)
                // Error: Multiple conflict actions
                checkForErrorAndReport(line, "Conflict actions before nonterminal %s",
                                       symbol.data());

            // Terminal - mark it with Terminal flag
            if (symbolId != NFA::InvalidLexeme) {
                symbolId |= ProductionMask::Terminal;

                // Get the precedence value
                if (UserLex.Precedence.find(symbolId) != UserLex.Precedence.end())
                    precValue = UserLex.Precedence[symbolId].Value & 
                                Grammar::TerminalPrec::PrecMask;
                else
                    precValue = 0u;
            } else {
                // It's a nonterminal
                // If it's not defined yet, assign an Id to this nonterminal
                if (nonTerminals.find(symbol) == nonTerminals.end())
                    nonTerminals[symbol] = idCounter++;
                // And return Id to store
                symbolId = nonTerminals[symbol];
            }

            if (!prec) {
                // Make sure to add it to grammarSymbols
                if (symbols.find(symbol) == symbols.end())
                    symbols[symbol] = symbolId;

                // And store Id into production
                rhs.push_back(symbolId);
            }
        }

        // Build reduce overrides map
        if (reduceValue) {
            std::vector<unsigned> nonTerminalIds;
            std::vector<unsigned> terminalIds;

            for (auto ireduce = iright; ireduce < right.size(); ++ireduce) {
                auto& symbol = right[ireduce];

                // It is a terminal
                if (symbol[0u] == '+') {
                    symbol = symbol.substr(1u);
                    // Terminal alias references will have special ' marker up front
                    if (symbol[0u] == '\'') {
                        // Eat alias marker quote
                        symbol = symbol.substr(1u);

                        if (UserLex.LexemeAliasToToken.find(symbol) !=
                            UserLex.LexemeAliasToToken.end()) {
                            symbolId = UserLex.LexemeAliasToToken[symbol] | 
                                       ProductionMask::Terminal;
                            terminalIds.push_back(symbolId);
                        } else
                            // ERROR: Undefined terminal alias referenced
                            checkForErrorAndReport(line,
                                                   "Undefined terminal alias referenced - '%s' inside of the %reduce()",
                                                   symbol.data());
                    } else if (UserLex.LexemeNameToToken.find(symbol) !=
                               UserLex.LexemeNameToToken.end()) {
                        symbolId = UserLex.LexemeNameToToken[symbol] | ProductionMask::Terminal;
                        terminalIds.push_back(symbolId);
                    } else
                        // ERROR: Undefined terminal referenced
                        checkForErrorAndReport(line,
                                               "Undefined terminal referenced - '%s' inside of the %reduce()",
                                               symbol.data());
                    continue;
                }

                // It is the first Nonterminal in the set
                if (symbol[0u] == '*') {
                    // store the last reduce data
                    for (const auto nonterm: nonTerminalIds)
                        reduceData[nonterm].insert(terminalIds.begin(), terminalIds.end());

                    // Reset the arrays
                    nonTerminalIds.clear();
                    terminalIds.clear();

                    // Eat the '*' marker
                    symbol = symbol.substr(1u);
                }

                // If it's not defined yet, assign an Id to this nonterminal
                if (nonTerminals.find(symbol) == nonTerminals.end())
                    nonTerminals[symbol] = idCounter++;
                // And return Id to store
                symbolId = nonTerminals[symbol];

                nonTerminalIds.push_back(symbolId);
            }

            // Store the last reduce data
            for (const auto nonterm: nonTerminalIds)
                reduceData[nonterm].insert(terminalIds.begin(), terminalIds.end());
        }

        // Build an array of LHS nonterminal symbol ids
        std::vector<unsigned> leftSymbolId;
        std::vector<unsigned> leftSymbolErrorTerminal;

        for (size_t ileft = 0u; ileft < left.size(); ++ileft) {
            // Store & get id for Left nonterminal
            const auto& symbol = left[ileft];

            if (nonTerminals.find(symbol) != nonTerminals.end())
                symbolId = nonTerminals[symbol];
            else {
                // Assign nonterminal Id
                symbolId = idCounter++;
                nonTerminals[symbol] = symbolId;
                // And record nonterminal symbol
                symbols[symbol]      = symbolId;
            }
            leftSymbolId.push_back(symbolId);

            // If we have associated %error
            if (ileft + 1u < left.size() && left[ileft + 1u][0u] == '%') {
                // Store error terminal's symbol id (might need to assign a new one)
                leftSymbolErrorTerminal.push_back(
                    symbols.find(left[ileft + 1u]) == symbols.end()
                        ? terminalIdCounter++ | ProductionMask::ErrorTerminal
                        : symbols[left[ileft + 1u]]);
                ++ileft;
            } else
                leftSymbolErrorTerminal.push_back(0);
        }

        // Store a production for all left hand side nonterminals
        for (size_t ileft = 0u; ileft < leftSymbolId.size(); ++ileft) {
            // Get the symbol id
            symbolId = leftSymbolId[ileft];

            // Finally store production value
            unsigned* const rght = rhs.size() ? &rhs[0] : nullptr;
            p.SetProduction(prod.Name, symbolId, rght, unsigned(rhs.size()),
                            data.ProductionLines[prod.Name], precValue);
            p.ErrorTerminal = leftSymbolErrorTerminal[ileft];

            // Build the left hand chain
            p.LeftChain.clear();
            for (size_t ichain = 0u; ichain < leftSymbolId.size(); ++ichain)
                // Make sure not to add the same nonterminal
                if (ichain != ileft)
                    p.LeftChain.push_back(leftSymbolId[ichain]);

            // Add the reduce overrides data
            p.ReduceOverrides.clear();
            if (reduceValue)
                p.ReduceOverrides = reduceData;

            // If its an action production don't report it
            if (prod.NotReported)
                p.NotReported = true;

            productions.push_back(p);
        }
    }

    // Process starting symbols
    for (const auto& symbol: data.OrderedStartSymbols)
        if (const auto inonTerminals = nonTerminals.find(symbol);
            inonTerminals == nonTerminals.end())
            // Error: Undefined production start symbol
            checkForErrorAndReport(data.ProductionStartSymbols[symbol],
                                   "Undefined production starting symbol %s", symbol.data());
        else
            // Store starting production's nonterminal Id
            startSymbols.push_back(inonTerminals->second);

    return errorCount == 0u;
}


// *** Parse data class

// Load in the grammar
bool  StdGrammarParseData::LoadGrammar(InputStream* ptokenInput) {
    // Make sure the parse data class is empty
    if (IsValid()) {
        // Report ParseData problem
        const ParseMessage msg{ParseMessage::ErrorMessage, "",
                               "ParseData class must be clear to load grammar"};
        Messages.AddMessage(msg);
        return false;
    }

    return BuildParser(ptokenInput);
}


// Clear the grammar
void StdGrammarParseData::ClearGrammar() {
    UserLex.Clear();
    UserGrammar.Clear();
}


// Make a new dfa
bool StdGrammarParseData::MakeDFA(DFAGen& dfa) {
    // Create a DFA from the lexemes
    return UserLex.MakeDFA(dfa);
}


// Return the parse table
bool StdGrammarParseData::MakeParseTable(ParseTableGen& parseTable,
                                         ParseTableType tableType) {
    return parseTable.Create(UserGrammar, tableType);
}

} // namespace Generator
} // namespace SGParser
