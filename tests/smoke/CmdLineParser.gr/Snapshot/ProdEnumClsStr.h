//////////////////////////// ProductionEnum ////////////////////////////

enum class ProductionEnum
{
    /*0*/ Accept,
    /*1*/ CmdLineExp,
    /*2*/ GrammarNameExp,
    /*3*/ EmptyGrammarNameExp,
    /*4*/ OptionListExp,
    /*5*/ EmptyOptionListExp,
    /*6*/ ConfigFileOption,
    /*7*/ TableTypeLROption,
    /*8*/ TableTypeLALROption,
    /*9*/ TableTypeCLROption,
    /*10*/ ParseDataOption,
    /*11*/ ParseDataParamList,
    /*12*/ ParseDataParamListEmpty,
    /*13*/ ParseDataFileNameParam,
    /*14*/ ParseDataStringParam,
    /*15*/ ParseDataDisplayParam,
    /*16*/ EnumFileOption,
    /*17*/ EnumFileFileNameParam,
    /*18*/ EnumFileFileNameParamEmpty,
    /*19*/ EnumClassesOption,
    /*20*/ EnumStringsOption,
    /*21*/ TermEnumOption,
    /*22*/ TermEnumParamList,
    /*23*/ TermEnumParamListEmpty,
    /*24*/ TermEnumFileNameParam,
    /*25*/ TermEnumClassNameParam,
    /*26*/ TermEnumPrefixParam,
    /*27*/ NonTermEnumOption,
    /*28*/ NonTermEnumParamList,
    /*29*/ NonTermEnumParamListEmpty,
    /*30*/ NonTermEnumFileNameParam,
    /*31*/ NonTermEnumClassNameParam,
    /*32*/ NonTermEnumPrefixParam,
    /*33*/ ProdEnumOption,
    /*34*/ ProdEnumParamList,
    /*35*/ ProdEnumParamListEmpty,
    /*36*/ ProdEnumFileNameParam,
    /*37*/ ProdEnumClassNameParam,
    /*38*/ ProdEnumPrefixParam,
    /*39*/ ReduceFuncOption,
    /*40*/ ReduceFuncParamList,
    /*41*/ ReduceFuncParamListEmpty,
    /*42*/ ReduceFuncFileNameParam,
    /*43*/ ReduceFuncClassNameParam,
    /*44*/ ReduceFuncStackNameParam,
    /*45*/ ReduceFuncPrefixParam,
    /*46*/ StaticDFAOption,
    /*47*/ StaticDFAParamList,
    /*48*/ StaticDFAParamListEmpty,
    /*49*/ StaticDFAFileNameParam,
    /*50*/ StaticDFAClassNameParam,
    /*51*/ StaticParseTableOption,
    /*52*/ StaticParseTableParamList,
    /*53*/ StaticParseTableParamListEmpty,
    /*54*/ StaticParseTableFileNameParam,
    /*55*/ StaticParseTableClassNameParam,
    /*56*/ CanonicalOption,
    /*57*/ CanonicalFileNameParam,
    /*58*/ CanonicalFileNameParamEmpty,
    /*59*/ ConflictReportOption,
    /*60*/ ConflictReportParamList,
    /*61*/ ConflictReportParamListEmpty,
    /*62*/ ConflictReportFileNameParam,
    /*63*/ ConflictReportLinesParam,
    /*64*/ ConflictReportLabelsParam,
    /*65*/ ConflictReportPointsParam,
    /*66*/ WarningOption,
    /*67*/ NotesOption,
    /*68*/ StatsOption,
    /*69*/ HelpOption,
    /*70*/ HelpParamList,
    /*71*/ HelpParamListEmpty,
    /*72*/ HelpMessageParam,
    /*73*/ HelpOptionParam,
    /*74*/ QuietModeOption,
    /*75*/ VersionOption,
    /*76*/ CmdLineGrammarOption,
    /*77*/ CmdLineGrammarFileNameParam1,
    /*78*/ CmdLineGrammarFileNameParam2,
    /*79*/ CmdLineGrammarFileNameParamEmpty,
    /*80*/ FileName1,
    /*81*/ FileName2,
    /*82*/ FileName3,
    /*83*/ ClassName1,
    /*84*/ ClassName2
};

constexpr char const* const StringifyEnumProductionEnum[] =
{
    /*0*/ "Accept",
    /*1*/ "CmdLineExp",
    /*2*/ "GrammarNameExp",
    /*3*/ "EmptyGrammarNameExp",
    /*4*/ "OptionListExp",
    /*5*/ "EmptyOptionListExp",
    /*6*/ "ConfigFileOption",
    /*7*/ "TableTypeLROption",
    /*8*/ "TableTypeLALROption",
    /*9*/ "TableTypeCLROption",
    /*10*/ "ParseDataOption",
    /*11*/ "ParseDataParamList",
    /*12*/ "ParseDataParamListEmpty",
    /*13*/ "ParseDataFileNameParam",
    /*14*/ "ParseDataStringParam",
    /*15*/ "ParseDataDisplayParam",
    /*16*/ "EnumFileOption",
    /*17*/ "EnumFileFileNameParam",
    /*18*/ "EnumFileFileNameParamEmpty",
    /*19*/ "EnumClassesOption",
    /*20*/ "EnumStringsOption",
    /*21*/ "TermEnumOption",
    /*22*/ "TermEnumParamList",
    /*23*/ "TermEnumParamListEmpty",
    /*24*/ "TermEnumFileNameParam",
    /*25*/ "TermEnumClassNameParam",
    /*26*/ "TermEnumPrefixParam",
    /*27*/ "NonTermEnumOption",
    /*28*/ "NonTermEnumParamList",
    /*29*/ "NonTermEnumParamListEmpty",
    /*30*/ "NonTermEnumFileNameParam",
    /*31*/ "NonTermEnumClassNameParam",
    /*32*/ "NonTermEnumPrefixParam",
    /*33*/ "ProdEnumOption",
    /*34*/ "ProdEnumParamList",
    /*35*/ "ProdEnumParamListEmpty",
    /*36*/ "ProdEnumFileNameParam",
    /*37*/ "ProdEnumClassNameParam",
    /*38*/ "ProdEnumPrefixParam",
    /*39*/ "ReduceFuncOption",
    /*40*/ "ReduceFuncParamList",
    /*41*/ "ReduceFuncParamListEmpty",
    /*42*/ "ReduceFuncFileNameParam",
    /*43*/ "ReduceFuncClassNameParam",
    /*44*/ "ReduceFuncStackNameParam",
    /*45*/ "ReduceFuncPrefixParam",
    /*46*/ "StaticDFAOption",
    /*47*/ "StaticDFAParamList",
    /*48*/ "StaticDFAParamListEmpty",
    /*49*/ "StaticDFAFileNameParam",
    /*50*/ "StaticDFAClassNameParam",
    /*51*/ "StaticParseTableOption",
    /*52*/ "StaticParseTableParamList",
    /*53*/ "StaticParseTableParamListEmpty",
    /*54*/ "StaticParseTableFileNameParam",
    /*55*/ "StaticParseTableClassNameParam",
    /*56*/ "CanonicalOption",
    /*57*/ "CanonicalFileNameParam",
    /*58*/ "CanonicalFileNameParamEmpty",
    /*59*/ "ConflictReportOption",
    /*60*/ "ConflictReportParamList",
    /*61*/ "ConflictReportParamListEmpty",
    /*62*/ "ConflictReportFileNameParam",
    /*63*/ "ConflictReportLinesParam",
    /*64*/ "ConflictReportLabelsParam",
    /*65*/ "ConflictReportPointsParam",
    /*66*/ "WarningOption",
    /*67*/ "NotesOption",
    /*68*/ "StatsOption",
    /*69*/ "HelpOption",
    /*70*/ "HelpParamList",
    /*71*/ "HelpParamListEmpty",
    /*72*/ "HelpMessageParam",
    /*73*/ "HelpOptionParam",
    /*74*/ "QuietModeOption",
    /*75*/ "VersionOption",
    /*76*/ "CmdLineGrammarOption",
    /*77*/ "CmdLineGrammarFileNameParam1",
    /*78*/ "CmdLineGrammarFileNameParam2",
    /*79*/ "CmdLineGrammarFileNameParamEmpty",
    /*80*/ "FileName1",
    /*81*/ "FileName2",
    /*82*/ "FileName3",
    /*83*/ "ClassName1",
    /*84*/ "ClassName2"
};
