// This is a generated file.
// Copyright is in `CmdLineParser.gr` - see that file for details.

bool ParseHandler::Reduce(Parse<StackElement> &parse, unsigned productionID)
{
    switch (productionID)
    {
        // CmdLine -> FileNameOption OptionList 
        case PE_CmdLineExp:
            break;

        // FileNameOption -> FileName 
        case PE_GrammarNameExp:
            break;

        // FileNameOption -> <empty>
        case PE_EmptyGrammarNameExp:
            break;

        // OptionList -> Option OptionList 
        case PE_OptionListExp:
            break;

        // OptionList -> <empty>
        case PE_EmptyOptionListExp:
            break;

        // Option -> '@' FileName 
        case PE_ConfigFileOption:
            break;

        // Option -> '-lr' 
        case PE_TableTypeLROption:
            break;

        // Option -> '-lalr' 
        case PE_TableTypeLALROption:
            break;

        // Option -> '-clr' 
        case PE_TableTypeCLROption:
            break;

        // Option -> '-parse' ParseDataParamList 
        case PE_ParseDataOption:
            break;

        // ParseDataParamList -> ParseDataParam ParseDataParamList 
        case PE_ParseDataParamList:
            break;

        // ParseDataParamList -> <empty>
        case PE_ParseDataParamListEmpty:
            break;

        // ParseDataParam -> '+filename' ':' FileName 
        case PE_ParseDataFileNameParam:
            break;

        // ParseDataParam -> '+string' ':' 'string' 
        case PE_ParseDataStringParam:
            break;

        // ParseDataParam -> '+display' 
        case PE_ParseDataDisplayParam:
            break;

        // Option -> '-namespaces' NamespaceParam 
        case PE_NamespaceOption:
            break;

        // NamespaceParam -> '+nsname' ':' ClassName 
        case PE_NamespaceClassNameParam:
            break;

        // NamespaceParam -> <empty>
        case PE_NamespaceClassNameParamEmpty:
            break;

        // Option -> '-enumfile' EnumFileParam 
        case PE_EnumFileOption:
            break;

        // EnumFileParam -> '+filename' ':' FileName 
        case PE_EnumFileFileNameParam:
            break;

        // EnumFileParam -> <empty>
        case PE_EnumFileFileNameParamEmpty:
            break;

        // Option -> '-enumclasses' 
        case PE_EnumClassesOption:
            break;

        // Option -> '-enumstrings' 
        case PE_EnumStringsOption:
            break;

        // Option -> '-termenum' TermEnumParamList 
        case PE_TermEnumOption:
            break;

        // TermEnumParamList -> TermEnumParam TermEnumParamList 
        case PE_TermEnumParamList:
            break;

        // TermEnumParamList -> <empty>
        case PE_TermEnumParamListEmpty:
            break;

        // TermEnumParam -> '+filename' ':' FileName 
        case PE_TermEnumFileNameParam:
            break;

        // TermEnumParam -> '+classname' ':' ClassName 
        case PE_TermEnumClassNameParam:
            break;

        // TermEnumParam -> '+prefix' ':' ClassName 
        case PE_TermEnumPrefixParam:
            break;

        // Option -> '-nontermenum' NonTermEnumParamList 
        case PE_NonTermEnumOption:
            break;

        // NonTermEnumParamList -> NonTermEnumParam NonTermEnumParamList 
        case PE_NonTermEnumParamList:
            break;

        // NonTermEnumParamList -> <empty>
        case PE_NonTermEnumParamListEmpty:
            break;

        // NonTermEnumParam -> '+filename' ':' FileName 
        case PE_NonTermEnumFileNameParam:
            break;

        // NonTermEnumParam -> '+classname' ':' ClassName 
        case PE_NonTermEnumClassNameParam:
            break;

        // NonTermEnumParam -> '+prefix' ':' ClassName 
        case PE_NonTermEnumPrefixParam:
            break;

        // Option -> '-prodenum' ProdEnumParamList 
        case PE_ProdEnumOption:
            break;

        // ProdEnumParamList -> ProdEnumParam ProdEnumParamList 
        case PE_ProdEnumParamList:
            break;

        // ProdEnumParamList -> <empty>
        case PE_ProdEnumParamListEmpty:
            break;

        // ProdEnumParam -> '+filename' ':' FileName 
        case PE_ProdEnumFileNameParam:
            break;

        // ProdEnumParam -> '+classname' ':' ClassName 
        case PE_ProdEnumClassNameParam:
            break;

        // ProdEnumParam -> '+prefix' ':' ClassName 
        case PE_ProdEnumPrefixParam:
            break;

        // Option -> '-reducefunc' ReduceFuncParamList 
        case PE_ReduceFuncOption:
            break;

        // ReduceFuncParamList -> ReduceFuncParam ReduceFuncParamList 
        case PE_ReduceFuncParamList:
            break;

        // ReduceFuncParamList -> <empty>
        case PE_ReduceFuncParamListEmpty:
            break;

        // ReduceFuncParam -> '+filename' ':' FileName 
        case PE_ReduceFuncFileNameParam:
            break;

        // ReduceFuncParam -> '+classname' ':' ClassName 
        case PE_ReduceFuncClassNameParam:
            break;

        // ReduceFuncParam -> '+stackname' ':' ClassName 
        case PE_ReduceFuncStackNameParam:
            break;

        // ReduceFuncParam -> '+prefix' ':' ClassName 
        case PE_ReduceFuncPrefixParam:
            break;

        // Option -> '-dfa' StaticDFAParamList 
        case PE_StaticDFAOption:
            break;

        // StaticDFAParamList -> StaticDFAParam StaticDFAParamList 
        case PE_StaticDFAParamList:
            break;

        // StaticDFAParamList -> <empty>
        case PE_StaticDFAParamListEmpty:
            break;

        // StaticDFAParam -> '+filename' ':' FileName 
        case PE_StaticDFAFileNameParam:
            break;

        // StaticDFAParam -> '+classname' ':' ClassName 
        case PE_StaticDFAClassNameParam:
            break;

        // Option -> '-parsetable' StaticParseTableParamList 
        case PE_StaticParseTableOption:
            break;

        // StaticParseTableParamList -> StaticParseTableParam StaticParseTableParamList 
        case PE_StaticParseTableParamList:
            break;

        // StaticParseTableParamList -> <empty>
        case PE_StaticParseTableParamListEmpty:
            break;

        // StaticParseTableParam -> '+filename' ':' FileName 
        case PE_StaticParseTableFileNameParam:
            break;

        // StaticParseTableParam -> '+classname' ':' ClassName 
        case PE_StaticParseTableClassNameParam:
            break;

        // Option -> '-canonical' CanonicalParam 
        case PE_CanonicalOption:
            break;

        // CanonicalParam -> '+filename' ':' FileName 
        case PE_CanonicalFileNameParam:
            break;

        // CanonicalParam -> <empty>
        case PE_CanonicalFileNameParamEmpty:
            break;

        // Option -> '-conflicts' ConflictReportParamList 
        case PE_ConflictReportOption:
            break;

        // ConflictReportParamList -> ConflictReportParam ConflictReportParamList 
        case PE_ConflictReportParamList:
            break;

        // ConflictReportParamList -> <empty>
        case PE_ConflictReportParamListEmpty:
            break;

        // ConflictReportParam -> '+filename' FileName 
        case PE_ConflictReportFileNameParam:
            break;

        // ConflictReportParam -> '+lines' 
        case PE_ConflictReportLinesParam:
            break;

        // ConflictReportParam -> '+labels' 
        case PE_ConflictReportLabelsParam:
            break;

        // ConflictReportParam -> '+points' 
        case PE_ConflictReportPointsParam:
            break;

        // Option -> '-nowarnings' 
        case PE_WarningOption:
            break;

        // Option -> '-notes' 
        case PE_NotesOption:
            break;

        // Option -> '-stats' 
        case PE_StatsOption:
            break;

        // Option -> '-help' HelpParamList 
        case PE_HelpOption:
            break;

        // HelpParamList -> HelpParam 
        case PE_HelpParamList:
            break;

        // HelpParamList -> <empty>
        case PE_HelpParamListEmpty:
            break;

        // HelpParam -> '+msg' ':' 'msgCode' 
        case PE_HelpMessageParam:
            break;

        // HelpParam -> '+option' ':' 'option' 
        case PE_HelpOptionParam:
            break;

        // Option -> '-quiet' 
        case PE_QuietModeOption:
            break;

        // Option -> '-version' 
        case PE_VersionOption:
            break;

        // Option -> '-clg' CmdLineGrammarParam 
        case PE_CmdLineGrammarOption:
            break;

        // CmdLineGrammarParam -> '+filename' ':' FileName 
        case PE_CmdLineGrammarFileNameParam1:
            break;

        // CmdLineGrammarParam -> ':' FileName 
        case PE_CmdLineGrammarFileNameParam2:
            break;

        // CmdLineGrammarParam -> <empty>
        case PE_CmdLineGrammarFileNameParamEmpty:
            break;

        // FileName -> 'fileName' 
        case PE_FileName1:
            break;

        // FileName -> 'className' 
        case PE_FileName2:
            break;

        // FileName -> 'msgCode' 
        case PE_FileName3:
            break;

        // ClassName -> 'className' 
        case PE_ClassName1:
            break;

        // ClassName -> 'msgCode' 
        case PE_ClassName2:
            break;

    }
    return true;
}
