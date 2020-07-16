bool ParseHandler::Reduce(Parse<StackElement> &parse, unsigned productionID)
{
    switch (static_cast<ProductionEnum>(productionID))
    {
        // CmdLine -> FileNameOption OptionList 
        case ProductionEnum::CmdLineExp:
            break;

        // FileNameOption -> FileName 
        case ProductionEnum::GrammarNameExp:
            break;

        // FileNameOption -> <empty>
        case ProductionEnum::EmptyGrammarNameExp:
            break;

        // OptionList -> Option OptionList 
        case ProductionEnum::OptionListExp:
            break;

        // OptionList -> <empty>
        case ProductionEnum::EmptyOptionListExp:
            break;

        // Option -> '@' FileName 
        case ProductionEnum::ConfigFileOption:
            break;

        // Option -> '-lr' 
        case ProductionEnum::TableTypeLROption:
            break;

        // Option -> '-lalr' 
        case ProductionEnum::TableTypeLALROption:
            break;

        // Option -> '-clr' 
        case ProductionEnum::TableTypeCLROption:
            break;

        // Option -> '-parse' ParseDataParamList 
        case ProductionEnum::ParseDataOption:
            break;

        // ParseDataParamList -> ParseDataParam ParseDataParamList 
        case ProductionEnum::ParseDataParamList:
            break;

        // ParseDataParamList -> <empty>
        case ProductionEnum::ParseDataParamListEmpty:
            break;

        // ParseDataParam -> '+filename' ':' FileName 
        case ProductionEnum::ParseDataFileNameParam:
            break;

        // ParseDataParam -> '+string' ':' 'string' 
        case ProductionEnum::ParseDataStringParam:
            break;

        // ParseDataParam -> '+display' 
        case ProductionEnum::ParseDataDisplayParam:
            break;

        // Option -> '-enumfile' EnumFileParam 
        case ProductionEnum::EnumFileOption:
            break;

        // EnumFileParam -> '+filename' ':' FileName 
        case ProductionEnum::EnumFileFileNameParam:
            break;

        // EnumFileParam -> <empty>
        case ProductionEnum::EnumFileFileNameParamEmpty:
            break;

        // Option -> '-enumclasses' 
        case ProductionEnum::EnumClassesOption:
            break;

        // Option -> '-enumstrings' 
        case ProductionEnum::EnumStringsOption:
            break;

        // Option -> '-termenum' TermEnumParamList 
        case ProductionEnum::TermEnumOption:
            break;

        // TermEnumParamList -> TermEnumParam TermEnumParamList 
        case ProductionEnum::TermEnumParamList:
            break;

        // TermEnumParamList -> <empty>
        case ProductionEnum::TermEnumParamListEmpty:
            break;

        // TermEnumParam -> '+filename' ':' FileName 
        case ProductionEnum::TermEnumFileNameParam:
            break;

        // TermEnumParam -> '+classname' ':' ClassName 
        case ProductionEnum::TermEnumClassNameParam:
            break;

        // TermEnumParam -> '+prefix' ':' ClassName 
        case ProductionEnum::TermEnumPrefixParam:
            break;

        // Option -> '-nontermenum' NonTermEnumParamList 
        case ProductionEnum::NonTermEnumOption:
            break;

        // NonTermEnumParamList -> NonTermEnumParam NonTermEnumParamList 
        case ProductionEnum::NonTermEnumParamList:
            break;

        // NonTermEnumParamList -> <empty>
        case ProductionEnum::NonTermEnumParamListEmpty:
            break;

        // NonTermEnumParam -> '+filename' ':' FileName 
        case ProductionEnum::NonTermEnumFileNameParam:
            break;

        // NonTermEnumParam -> '+classname' ':' ClassName 
        case ProductionEnum::NonTermEnumClassNameParam:
            break;

        // NonTermEnumParam -> '+prefix' ':' ClassName 
        case ProductionEnum::NonTermEnumPrefixParam:
            break;

        // Option -> '-prodenum' ProdEnumParamList 
        case ProductionEnum::ProdEnumOption:
            break;

        // ProdEnumParamList -> ProdEnumParam ProdEnumParamList 
        case ProductionEnum::ProdEnumParamList:
            break;

        // ProdEnumParamList -> <empty>
        case ProductionEnum::ProdEnumParamListEmpty:
            break;

        // ProdEnumParam -> '+filename' ':' FileName 
        case ProductionEnum::ProdEnumFileNameParam:
            break;

        // ProdEnumParam -> '+classname' ':' ClassName 
        case ProductionEnum::ProdEnumClassNameParam:
            break;

        // ProdEnumParam -> '+prefix' ':' ClassName 
        case ProductionEnum::ProdEnumPrefixParam:
            break;

        // Option -> '-reducefunc' ReduceFuncParamList 
        case ProductionEnum::ReduceFuncOption:
            break;

        // ReduceFuncParamList -> ReduceFuncParam ReduceFuncParamList 
        case ProductionEnum::ReduceFuncParamList:
            break;

        // ReduceFuncParamList -> <empty>
        case ProductionEnum::ReduceFuncParamListEmpty:
            break;

        // ReduceFuncParam -> '+filename' ':' FileName 
        case ProductionEnum::ReduceFuncFileNameParam:
            break;

        // ReduceFuncParam -> '+classname' ':' ClassName 
        case ProductionEnum::ReduceFuncClassNameParam:
            break;

        // ReduceFuncParam -> '+stackname' ':' ClassName 
        case ProductionEnum::ReduceFuncStackNameParam:
            break;

        // ReduceFuncParam -> '+prefix' ':' ClassName 
        case ProductionEnum::ReduceFuncPrefixParam:
            break;

        // Option -> '-dfa' StaticDFAParamList 
        case ProductionEnum::StaticDFAOption:
            break;

        // StaticDFAParamList -> StaticDFAParam StaticDFAParamList 
        case ProductionEnum::StaticDFAParamList:
            break;

        // StaticDFAParamList -> <empty>
        case ProductionEnum::StaticDFAParamListEmpty:
            break;

        // StaticDFAParam -> '+filename' ':' FileName 
        case ProductionEnum::StaticDFAFileNameParam:
            break;

        // StaticDFAParam -> '+classname' ':' ClassName 
        case ProductionEnum::StaticDFAClassNameParam:
            break;

        // Option -> '-parsetable' StaticParseTableParamList 
        case ProductionEnum::StaticParseTableOption:
            break;

        // StaticParseTableParamList -> StaticParseTableParam StaticParseTableParamList 
        case ProductionEnum::StaticParseTableParamList:
            break;

        // StaticParseTableParamList -> <empty>
        case ProductionEnum::StaticParseTableParamListEmpty:
            break;

        // StaticParseTableParam -> '+filename' ':' FileName 
        case ProductionEnum::StaticParseTableFileNameParam:
            break;

        // StaticParseTableParam -> '+classname' ':' ClassName 
        case ProductionEnum::StaticParseTableClassNameParam:
            break;

        // Option -> '-canonical' CanonicalParam 
        case ProductionEnum::CanonicalOption:
            break;

        // CanonicalParam -> '+filename' ':' FileName 
        case ProductionEnum::CanonicalFileNameParam:
            break;

        // CanonicalParam -> <empty>
        case ProductionEnum::CanonicalFileNameParamEmpty:
            break;

        // Option -> '-conflicts' ConflictReportParamList 
        case ProductionEnum::ConflictReportOption:
            break;

        // ConflictReportParamList -> ConflictReportParam ConflictReportParamList 
        case ProductionEnum::ConflictReportParamList:
            break;

        // ConflictReportParamList -> <empty>
        case ProductionEnum::ConflictReportParamListEmpty:
            break;

        // ConflictReportParam -> '+filename' FileName 
        case ProductionEnum::ConflictReportFileNameParam:
            break;

        // ConflictReportParam -> '+lines' 
        case ProductionEnum::ConflictReportLinesParam:
            break;

        // ConflictReportParam -> '+labels' 
        case ProductionEnum::ConflictReportLabelsParam:
            break;

        // ConflictReportParam -> '+points' 
        case ProductionEnum::ConflictReportPointsParam:
            break;

        // Option -> '-nowarnings' 
        case ProductionEnum::WarningOption:
            break;

        // Option -> '-notes' 
        case ProductionEnum::NotesOption:
            break;

        // Option -> '-stats' 
        case ProductionEnum::StatsOption:
            break;

        // Option -> '-help' HelpParamList 
        case ProductionEnum::HelpOption:
            break;

        // HelpParamList -> HelpParam 
        case ProductionEnum::HelpParamList:
            break;

        // HelpParamList -> <empty>
        case ProductionEnum::HelpParamListEmpty:
            break;

        // HelpParam -> '+msg' ':' 'msgCode' 
        case ProductionEnum::HelpMessageParam:
            break;

        // HelpParam -> '+option' ':' 'option' 
        case ProductionEnum::HelpOptionParam:
            break;

        // Option -> '-quiet' 
        case ProductionEnum::QuietModeOption:
            break;

        // Option -> '-version' 
        case ProductionEnum::VersionOption:
            break;

        // Option -> '-clg' CmdLineGrammarParam 
        case ProductionEnum::CmdLineGrammarOption:
            break;

        // CmdLineGrammarParam -> '+filename' ':' FileName 
        case ProductionEnum::CmdLineGrammarFileNameParam1:
            break;

        // CmdLineGrammarParam -> ':' FileName 
        case ProductionEnum::CmdLineGrammarFileNameParam2:
            break;

        // CmdLineGrammarParam -> <empty>
        case ProductionEnum::CmdLineGrammarFileNameParamEmpty:
            break;

        // FileName -> 'fileName' 
        case ProductionEnum::FileName1:
            break;

        // FileName -> 'className' 
        case ProductionEnum::FileName2:
            break;

        // FileName -> 'msgCode' 
        case ProductionEnum::FileName3:
            break;

        // ClassName -> 'className' 
        case ProductionEnum::ClassName1:
            break;

        // ClassName -> 'msgCode' 
        case ProductionEnum::ClassName2:
            break;

    }
    return 1u;
}
