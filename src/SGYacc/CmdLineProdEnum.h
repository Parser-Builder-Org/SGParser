// Filename:  CmdLineProdEnum.h
// Content:   Command line grammar production tokens enumeration
// Provided AS IS under MIT License; see LICENSE file in root folder.

enum CmdLineProdEnum
{
    CL_Accept,
    CL_CmdLineExp,
    CL_GrammarNameExp,
    CL_EmptyGrammarNameExp,
    CL_OptionListExp,
    CL_EmptyOptionListExp,

    CL_ConfigFileOption,

    CL_TableTypeLROption,
    CL_TableTypeLALROption,
    CL_TableTypeCLROption,

    CL_ParseDataOption,
    CL_ParseDataParamList,
    CL_ParseDataParamListEmpty,
    CL_ParseDataFileNameParam,
    CL_ParseDataStringParam,
    CL_ParseDataDisplayParam,

    CL_NamespaceOption,
    CL_NamespaceClassNameParam,
    CL_NamespaceClassNameParamEmpty,

    CL_EnumFileOption,
    CL_EnumFileFileNameParam,
    CL_EnumFileFileNameParamEmpty,

    CL_EnumClassesOption,

    CL_EnumStringsOption,

    CL_TermEnumOption,
    CL_TermEnumParamList,
    CL_TermEnumParamListEmpty,
    CL_TermEnumFileNameParam,
    CL_TermEnumClassNameParam,
    CL_TermEnumPrefixParam,

    CL_NonTermEnumOption,
    CL_NonTermEnumParamList,
    CL_NonTermEnumParamListEmpty,
    CL_NonTermEnumFileNameParam,
    CL_NonTermEnumClassNameParam,
    CL_NonTermEnumPrefixParam,

    CL_ProdEnumOption,
    CL_ProdEnumParamList,
    CL_ProdEnumParamListEmpty,
    CL_ProdEnumFileNameParam,
    CL_ProdEnumClassNameParam,
    CL_ProdEnumPrefixParam,

    CL_ReduceFuncOption,
    CL_ReduceFuncParamList,
    CL_ReduceFuncParamListEmpty,
    CL_ReduceFuncFileNameParam,
    CL_ReduceFuncClassNameParam,
    CL_ReduceFuncStackNameParam,
    CL_ReduceFuncPrefixParam,

    CL_StaticDFAOption,
    CL_StaticDFAParamList,
    CL_StaticDFAParamListEmpty,
    CL_StaticDFAFileNameParam,
    CL_StaticDFAClassNameParam,

    CL_StaticParseTableOption,
    CL_StaticParseTableParamList,
    CL_StaticParseTableParamListEmpty,
    CL_StaticParseTableFileNameParam,
    CL_StaticParseTableClassNameParam,

    CL_CanonicalOption,
    CL_CanonicalFileNameParam,
    CL_CanonicalFileNameParamEmpty,

    CL_ConflictReportOption,
    CL_ConflictReportParamList,
    CL_ConflictReportParamListEmpty,
    CL_ConflictReportFileNameParam,
    CL_ConflictReportLinesParam,
    CL_ConflictReportLabelsParam,
    CL_ConflictReportPointsParam,

    CL_WarningOption,

    CL_NotesOption,

    CL_StatsOption,

    CL_HelpOption,
    CL_HelpParamList,
    CL_HelpParamListEmpty,
    CL_HelpMessageParam,
    CL_HelpOptionParam,

    CL_QuietModeOption,

    CL_VersionOption,

    CL_CmdLineGrammarOption,
    CL_CmdLineGrammarFileNameParam1,
    CL_CmdLineGrammarFileNameParam2,
    CL_CmdLineGrammarFileNameParamEmpty,

    CL_FileName1,
    CL_FileName2,
    CL_FileName3,

    CL_ClassName1,
    CL_ClassName2
};
