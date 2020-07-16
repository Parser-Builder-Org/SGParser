// Filename:  StdGrammarProdEnum.h
// Content:   Standard grammar production terms enumeration
// Provided AS IS under MIT License; see LICENSE file in root folder.

enum  StdGrammarProductionEnum
{
    SG_Accept,
    SG_RootBlock,
    SG_RootBlockError,

    // Macro
    SG_MacroSection,
    SG_MacroSectionError,
    SG_MacroSectionEmpty,

    SG_MacroBlock,
    SG_MacroBlockEmpty,

    SG_MacroLine,
    SG_MacroLineError,

    SG_MacroName,
    SG_MacroNameError,

    SG_MacroExpr,
    SG_MacroExprError,

    // Expression
    SG_ExprSectionList,
    SG_ExprSectionListEmpty,

    SG_ExprSection,
    SG_ExprSectionError,

    SG_ExprBlock,
    SG_ExprBlockEmpty,

    SG_ExprLine,
    SG_ExprLineAlias,
    SG_ExprLineIgnore,
    SG_ExprLineError,

    SG_OptActionPush,
    SG_OptActionPop,
    SG_OptActionGoto,
    SG_OptActionNone,

    // Precedence
    SG_PrecSection,
    SG_PrecSectionError,
    SG_PrecSectionEmpty,

    SG_PrecBlock,
    SG_PrecBlockEmpty,

    SG_PrecTerminalListAssoc1,
    SG_PrecTerminalListAssoc2,
    SG_PrecAssocTerminalList1,
    SG_PrecAssocTerminalList2,
    SG_PrecError,

    SG_Num,
    SG_NumComma,
    SG_NumEmpty,

    SG_TerminalOne,
    SG_TerminalList,
    SG_TerminalListComma,

    SG_Terminal,
    SG_TerminalQuote,

    SG_AssocLeft,
    SG_AssocRight,
    SG_AssocNon,

    // Production
    SG_ProdSection,
    SG_ProdSectionEmpty,

    SG_ProdStartNameList,
    SG_ProdStartNameListNested,

    SG_ProdStartSymbolDecl,
    SG_ProdStartSymbolDeclError,

    SG_ProdStartSymbolListId,
    SG_ProdStartSymbolListNested,

    SG_ProdBlock,
    SG_ProdBlockEmpty,

    SG_ProdLine,

    SG_ProdLeft,
    SG_ProdLeftError,

    SG_ProdLHSNameList,
    SG_ProdLHSNameListNested,

    SG_ProdLHSId,
    SG_ProdLHSNamedError,

    SG_ProdRight,
    SG_ProdRightError,

    SG_ProdRHS,
    SG_ProdRHSErrorToken,
    SG_ProdRHSEmpty,

    SG_ProdSymbolCombineSingle,
    SG_ProdSymbolCombine,

    SG_ProdFullSymbol,
    SG_ProdFullSymbolError,

    SG_ProdPrefixShift,
    SG_ProdPrefixShiftError,
    SG_ProdPrefixReduce,
    SG_ProdPrefixReduceError,
    SG_ProdPrefixEmpty,

    SG_ProdSymbolId,
    SG_ProdSymbolQuote,
    SG_ProdSymbolAction,
    SG_ProdSymbolActionError,

    SG_ProdRHSError,
    SG_ProdRHSErrorBacktrack,
    SG_ProdRHSErrorNamed,

    SG_NonterminalListId,
    SG_NonterminalListNested1,
    SG_NonterminalListNested2,

    SG_ProdPostfixPrecReduce,
    SG_ProdPostfixReducePrec,
    SG_ProdPostfixReduce,
    SG_ProdPostfixPrec,
    SG_ProdPostfixEmpty,

    SG_ProdPrecId,
    SG_ProdPrecQuote,

    SG_ProdReduce,

    SG_ReduceList,
    SG_ReduceListNested,

    SG_ReduceExpr,
    SG_ReduceExprWithTerminals,

    SG_ReduceNonterminalList,
    SG_ReduceNonterminalListNested,

    SG_ReduceTerminalList,
    SG_ReduceTerminalListNested,

    SG_NamedError,
    SG_NamedErrorError
};
