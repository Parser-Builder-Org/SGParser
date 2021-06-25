// Filename:  CmdLineParser.cpp
// Content:   Command line parser program
// Provided AS IS under MIT License; see LICENSE file in root folder.

#include "CmdLineParser.h"
#include "CmdLineProdEnum.h"
#include "FileInputStream.h"
#include "FileOutputStream.h"

#include <filesystem>

namespace SGParser
{
namespace Generator
{
namespace Yacc
{
namespace // anonymous
{

// *** Generic Parse Handler

// Parse handler used to parse the test data
class GenericParseHandler final : public ParseHandler<ParseStackGenericElement>
{
public:
    std::vector<Production*> Productions; // List of productions
    ParseMessageBuffer       Messages;    // Stores a set of reduction messages if stats are enabled

    // Overridden reduce function
    bool Reduce(Parse<ParseStackGenericElement> &parse, unsigned productionID) override;
};

// Generic reduce function used when parsing test data from the command line
bool GenericParseHandler::Reduce(Parse<ParseStackGenericElement>& parse, unsigned productionID) {
    // If the flag for stats is set then this function will report the reductions
    if (Messages.GetMessageFlags() & ParseMessageBuffer::MessageStats) {
        String buffer = "Reduce ";

        if (Productions.size() > size_t(productionID))
            buffer += Productions[productionID]->Name;
        else
            buffer += StringFromNumber(productionID);

        buffer += " : " + parse[0].Str;

        const ParseMessage msg{ParseMessage::StatMessage, "Reduction", buffer, 0u,
                               parse[0].Line, parse[0].Offset, "",
                               ParseMessage::DisplayType | ParseMessage::DisplayMessage |
                               ParseMessage::DisplayLine | ParseMessage::DisplayOffset};
        Messages.AddMessage(msg);
    }

    return true;
}

} // anonymous namespace


// *** Command line parse handler

// Adds an option to the options set
void CmdLineParseHandler::SetOption(const String& option) {
    // Should check to see if it already exists or not
    Options[option].Name = option;
}


// Adds a parameter to a specific option
void CmdLineParseHandler::SetOptionParam(const String& option, const String& param,
                                         const String& value) {
    // Should check to see if it already exists or not
    Options[option].Params[param].Value = value;
}


// Checks to see if an option parameter exists
bool CmdLineParseHandler::CheckOptionParam(const String& optionName,
                                           const String& paramName) const {
    // If the option is found than search for the parameter
    if (const auto option = Options.find(optionName); option != Options.end()) {
        const auto& poption = option->second;
        return (poption.Params.find(paramName) != poption.Params.end());
    }
    return false;
}


// Get the option parameter value and store it in the destination string
bool CmdLineParseHandler::GetOptionParam(const String& optionName, const String& paramName,
                                         String& dest) const {
    // If the option exists then try to find the parameter
    if (const auto ioption = Options.find(optionName); ioption != Options.end()) {
        const auto& poption = ioption->second;
        // If the parameter exists then set the destination to its value and return success
        if (const auto iparam = poption.Params.find(paramName); iparam != poption.Params.end()) {
            dest = iparam->second.Value;
            return true;
        }
    }
    return false;
}

// Generate pseudo-copyright header string based on GrammarFileName
String CmdLineParseHandler::CopyrightHeader() const {
    return "// This is a generated file.\n// Copyright is in `" +
           std::filesystem::path{GrammarFileName}.filename().string() +
           "` - see that file for details.\n\n";
}

// Add a duplicate parameter message
void CmdLineParseHandler::AddDuplicateOptionMessage([[maybe_unused]] char* poption) {
    ++ErrorCount;
}


// Add a duplicate parameter message
void CmdLineParseHandler::AddDuplicateParamMessage([[maybe_unused]] char* poption,
                                                   [[maybe_unused]] char* pparam) {
    ++ErrorCount;
}


// Check to see if the parameter is being duplicated
int CmdLineParseHandler::HandleDuplicateStringParam([[maybe_unused]] String& source,
                                                    [[maybe_unused]] char* poption,
                                                    [[maybe_unused]] char* pparam) {
    return 0;
}


// Check to see if the parameter is being duplicated
int CmdLineParseHandler::HandleDuplicateIntegerParam([[maybe_unused]] int source,
                                                     [[maybe_unused]] char* poption,
                                                     [[maybe_unused]] char* pparam) {
    return 0;
}


// Main reduce function
bool CmdLineParseHandler::Reduce(Parse<ParseStackGenericElement>& parse, unsigned productionID) {
    if (Messages.GetMessageFlags() & ParseMessageBuffer::MessageStats) {
        const auto msgstr = "Reduce " + Productions[productionID]->Name + " : " + parse[0].Str;
        const ParseMessage msg{ParseMessage::StatMessage, "Reduction", msgstr, 0u,
                               parse[0].Line, parse[0].Offset, "",
                               ParseMessage::DisplayType | ParseMessage::DisplayMessage |
                               ParseMessage::DisplayLine | ParseMessage::DisplayOffset};
        Messages.AddMessage(msg);
    }

    switch (productionID) {
        // CmdLine -> FileNameOption OptionList
        case CL_CmdLineExp:
            break;

        // FileNameOption -> 'FileName'
        case CL_GrammarNameExp:
            GrammarFileName = parse[0].Str;
            break;

        // FileNameOption -> <empty>
        case CL_EmptyGrammarNameExp:
            break;

        // OptionList -> Option OptionList
        case CL_OptionListExp:
            break;

        // OptionList -> <empty>
        case CL_EmptyOptionListExp:
            break;

        // Option ->
        case CL_ConfigFileOption:
            SetOptionParam("ConfigFile", "Filename", parse[1].Str);
            break;

        // Option -> '-lr'
        case CL_TableTypeLROption:
            SetOptionParam("TableType", "Type", "LR");
            break;

        // Option -> '-lalr'
        case CL_TableTypeLALROption:
            SetOptionParam("TableType", "Type", "LALR");
            break;

        // Option -> '-clr'
        case CL_TableTypeCLROption:
            SetOptionParam("TableType", "Type", "CLR");
            break;

        // Option -> '-parse' ParseDataParamList
        case CL_ParseDataOption:
            SetOption("ParseData");
            break;

        // Option -> '-namespaces' NamespaceParam
        case CL_NamespaceOption:
            SetOption("Namespaces");
            break;

            // Option -> '-canonical' EnumFileParamList
        case CL_EnumFileOption:
            SetOption("EnumFile");
            break;

        // Option -> '-enumclasses'
        case CL_EnumClassesOption:
            SetOption("EnumClasses");
            break;

        // Option -> '-enumstrings'
        case CL_EnumStringsOption:
            SetOption("EnumStrings");
            break;

        // Option -> '-termenum' TermEnumParamList
        case CL_TermEnumOption:
            SetOption("TermEnum");
            break;

        // Option -> '-nontermenum' NonTermEnumParamList
        case CL_NonTermEnumOption:
            SetOption("NonTermEnum");
            break;

        // Option -> '-prodenum' ProdEnumParamList
        case CL_ProdEnumOption:
            SetOption("ProdEnum");
            break;

        // Option -> '-reducefunc' ReduceFuncParamList
        case CL_ReduceFuncOption:
            SetOption("ReduceFunc");
            break;

        // Option -> '-dfa' StaticDFAParamList
        case CL_StaticDFAOption:
            SetOption("StaticDFA_Table");
            break;

        // Option -> '-parsetable' StaticParseTableParamList
        case CL_StaticParseTableOption:
            SetOption("StaticParse_Table");
            break;

        // Option -> '-canonical' CanonicalParamList
        case CL_CanonicalOption:
            SetOption("Canonical");
            break;

        // Option -> '-conflicts' ConflictReportParamList
        case CL_ConflictReportOption:
            SetOption("ConflictReport");
            break;

        // Option -> '-nowarnings'
        case CL_WarningOption:
            SetOptionParam("Message", "NoWarnings", "1");
            break;

        // Option -> '-notes'
        case CL_NotesOption:
            SetOptionParam("Message", "Notes", "1");
            break;

        // Option -> '-stats'
        case CL_StatsOption:
            SetOptionParam("Message", "Stats", "1");
            break;

        // Option -> '-help' HelpParamList
        case CL_HelpOption:
            SetOption("Help");
            break;

        // Option -> '-clg' CmdLineGrammarParamList
        case CL_CmdLineGrammarOption:
            SetOption("CmdLineGrammar");
            break;

        // Option -> '-quiet'
        case CL_QuietModeOption:
            SetOption("Quiet");
            break;

        // Option -> '-version'
        case CL_VersionOption:
            SetOption("Version");
            break;

        // ParseDataParamList -> ParseDataInputParam ParseDataParamList
        case CL_ParseDataParamList:
            break;

        // ParseDataParamList -> <empty>
        case CL_ParseDataParamListEmpty:
            break;

        // ParseDataInputParam -> '+filename' ':' FileName
        case CL_ParseDataFileNameParam:
            SetOptionParam("ParseData", "Filename", parse[2].Str);
            break;

        // ParseDataInputParam -> '+string' ':' 'string'
        case CL_ParseDataStringParam:
            SetOptionParam("ParseData", "StringData", parse[2].Str);
            break;

        // ParseDataDisplayParam -> '+display'
        case CL_ParseDataDisplayParam:
            SetOptionParam("ParseData", "PrintReductions", "1");
            break;

        // NamespaceParam -> '+nsname' ':' ClassName
        case CL_NamespaceClassNameParam:
            SetOptionParam("Namespaces", "Classname", parse[2].Str);
            break;

        // NamespaceParam -> <empty>
        case CL_NamespaceClassNameParamEmpty:
            break;

        // EnumFileParam -> '+filename' ':' FileName
        case CL_EnumFileFileNameParam:
            SetOptionParam("EnumFile", "Filename", parse[2].Str);
            break;

        // EnumFileParam -> <empty>
        case CL_EnumFileFileNameParamEmpty:
            break;

        // TermEnumParamList -> TermEnumParam TermEnumParamList
        case CL_TermEnumParamList:
            break;

        // TermEnumParamList -> <empty>
        case CL_TermEnumParamListEmpty:
            break;

        // TermEnumParam -> '+filename' ':' FileName
        case CL_TermEnumFileNameParam:
            SetOptionParam("TermEnum", "Filename", parse[2].Str);
            break;

        // TermEnumParam -> '+classname' ':' 'ClassName'
        case CL_TermEnumClassNameParam:
            SetOptionParam("TermEnum", "Classname", parse[2].Str);
            break;

        // TermEnumParam -> '+prefix' ':' 'ClassName'
        case CL_TermEnumPrefixParam:
            SetOptionParam("TermEnum", "Prefix", parse[2].Str);
            break;

        // NonTermEnumParamList -> NonTermEnumParam NonTermEnumParamList
        case CL_NonTermEnumParamList:
            break;

        // NonTermEnumParamList -> <empty>
        case CL_NonTermEnumParamListEmpty:
            break;

        // NonTermEnumParam -> '+filename' ':' FileName
        case CL_NonTermEnumFileNameParam:
            SetOptionParam("NonTermEnum", "Filename", parse[2].Str);
            break;

        // NonTermEnumParam -> '+classname' ':' 'ClassName'
        case CL_NonTermEnumClassNameParam:
            SetOptionParam("NonTermEnum", "Classname", parse[2].Str);
            break;

        // NonTermEnumParam -> '+prefix' ':' 'ClassName'
        case CL_NonTermEnumPrefixParam:
            SetOptionParam("NonTermEnum", "Prefix", parse[2].Str);
            break;

        // ProdEnumParamList -> LexEnumParam LexEnumParamList
        case CL_ProdEnumParamList:
            break;
        // ProdEnumParamListEmpty -> <empty>
        case CL_ProdEnumParamListEmpty:
            break;

        // ProdEnumParam -> '+filename' ':' 'FileName'
        case CL_ProdEnumFileNameParam:
            SetOptionParam("ProdEnum", "Filename", parse[2].Str);
            break;

        // ProdEnumParam -> '+classname' ':' 'ClassName'
        case CL_ProdEnumClassNameParam:
            SetOptionParam("ProdEnum", "Classname", parse[2].Str);
            break;

        // ProdEnumParam -> '+prefix' ':' 'ClassName'
        case CL_ProdEnumPrefixParam:
            SetOptionParam("ProdEnum", "Prefix", parse[2].Str);
            break;

        // ReduceFuncParamList -> ReduceFuncParam ReduceFuncParamList
        case CL_ReduceFuncParamList:
            break;

        // ReduceFuncParamList -> <empty>
        case CL_ReduceFuncParamListEmpty:
            break;

        // ReduceFuncParam -> '+filename' ':' FileName
        case CL_ReduceFuncFileNameParam:
            SetOptionParam("ReduceFunc", "Filename", parse[2].Str);
            break;

        // ReduceFuncParam -> '+classname' ':' 'ClassName'
        case CL_ReduceFuncClassNameParam:
            SetOptionParam("ReduceFunc", "Classname", parse[2].Str);
            break;

        // ReduceFuncParam -> '+stackname' ':' 'ClassName'
        case CL_ReduceFuncStackNameParam:
            SetOptionParam("ReduceFunc", "Stackname", parse[2].Str);
            break;

        // ReduceFuncParam -> '+prefix' ':' 'ClassName'
        case CL_ReduceFuncPrefixParam:
            SetOptionParam("ReduceFunc", "Prefix", parse[2].Str);
            break;

        // StaticDFAParamList -> StaticDFAParam StaticDFAParamList
        case CL_StaticDFAParamList:
            break;

        case CL_StaticDFAParamListEmpty:
            break;

        // StaticDFAParam -> '+filename' ':' FileName
        case CL_StaticDFAFileNameParam:
            SetOptionParam("StaticDFA", "Filename", parse[2].Str);
            break;

        // StaticDFAParam -> '+classname' ':' 'ClassName'
        case CL_StaticDFAClassNameParam:
            SetOptionParam("StaticDFA", "Classname", parse[2].Str);
            break;

        // StaticParseTableParamList -> StaticParseTableParam StaticParseTableParamList
        case CL_StaticParseTableParamList:
            break;

        // StaticParseTableParam -> '+filename' ':' FileName
        case CL_StaticParseTableFileNameParam:
            SetOptionParam("StaticParseTable", "Filename", parse[2].Str);
            break;

        // StaticParseTableParam -> '+classname' ':' 'ClassName'
        case CL_StaticParseTableClassNameParam:
            SetOptionParam("StaticParseTable", "Classname", parse[2].Str);
            break;

        // CanonicalParam -> '+filename' ':' FileName
        case CL_CanonicalFileNameParam:
            SetOptionParam("Canonical", "Filename", parse[2].Str);
            break;

        // CanonicalParam -> <empty>
        case CL_CanonicalFileNameParamEmpty:
            break;

        // ConflictReportParamList -> ConflictReportParam ConflictReportParamList
        case CL_ConflictReportParamList:
            break;

        // ConflictReportParamList -> <empty>
        case CL_ConflictReportParamListEmpty:
            break;

        // ConflictReportParam -> '+filename' ':' FileName
        case CL_ConflictReportFileNameParam:
            SetOptionParam("ConflictReport", "Filename", parse[2].Str);
            break;

        // ConflictReportParam -> '+lines'
        case CL_ConflictReportLinesParam:
            SetOptionParam("ConflictReport", "Lines", "1");
            break;

        // ConflictReportParam -> '+labels'
        case CL_ConflictReportLabelsParam:
            SetOptionParam("ConflictReport", "Labels", "1");
            break;

        // ConflictReportParam -> '+points'
        case CL_ConflictReportPointsParam:
            SetOptionParam("ConflictReport", "Points", "1");
            break;

        // HelpParamList -> HelpParam
        case CL_HelpParamList:
            break;

        // HelpParamList ->
        case CL_HelpParamListEmpty:
            break;

        // HelpParam -> '+option' ':' 'option'
        case CL_HelpOptionParam:
            break;

        // HelpParam -> '+msg' ':' 'msgCode'
        case CL_HelpMessageParam:
            break;

        // CmdLineGrammarParam -> '+filename' ':' FileName
        case CL_CmdLineGrammarFileNameParam1:
            SetOptionParam("CmdLineGrammar", "Filename", parse[1].Str);
            break;

        // CmdLineGrammarParam -> ':' FileName
        case CL_CmdLineGrammarFileNameParam2:
            SetOptionParam("CmdLineGrammar", "Filename", parse[0].Str);
            break;
    }
    return true;
}


const char* CmdLineParseHandler::GetHelpText() const noexcept {
    return
        "Simple Grammar Parser Generator v1.0\n\n"
        "Command line (case insensitive): parser [grammar file] [-options [+params]]\n\n"

        "Option                Description with parameters\n"
        "--------------------  --------------------------------------------------------\n"
        "@<filename>           Read command line options from file\n"
        "-lr, -lalr, -clr      Create an LR(1), LALR(1), or Compact LR(1) parse table\n"
        "-p[arse]              Parse a test file\n"
        "                          [+f[ilename]:<testfile>]     specify test file\n"
        "                          [+str[ing]:<teststring>]     specify test string\n"
        "                          [+d[isplay]]                 display the reductions\n"
        "-term[inal]enum       Create a enum of all terminals\n"
        "                          [+f[ilename]:<targetfile>]   enum output file\n"
        "                          [+c[lassname]:<classname>]   terminal enum id-name\n"
        "                          [+p[refix]:<termprefix>]     enum element prefix\n"
        "-nonterm[inal]enum    Create a enum of all non terminals\n"
        "                          [+f[ilename]:<targetfile>]   enum output file\n"
        "                          [+c[lassname]:<classname>]   nonterminal enum idname\n"
        "                          [+p[refix]:<nontermprefix>]  enum element prefix\n"
        "-prod[uction]enum     Create the production enum\n"
        "                          [+f[ilename]:<targetfile>]   enum output file\n"
        "                          [+c[lassname]:<classname>]   production enum id-name\n"
        "                          [+p[refix]:<prodprefix>]     production name prefix\n"
        "-enumfile             Default enumeration file to use\n"
        "                          [[+f[ilename]]:<targetfile>] filename for all files\n"
        "-enumclasses          Use 'enum class' instead of 'enum'\n"
        "-enumstrings          Create string literals for enumeration stringification\n"
        "-ns,-namespaces       Enclose generated code into a namespace\n"
        "                          [+nsname:<namespacename>]    namespace name\n"
        "-rf,-reducefunc       Make reduce function\n"
        "                          [+f[ilename]:<targetfile>]   function output file\n"
        "                          [+c[lassname]:<classname>]   ParseHandler classname\n"
        "                          [+s[tackname]:<stackname>]   StackElement classname\n"
        "                          [+p[refix]:<prodprefix>]     production name prefix\n"
        "-dfa                  Create a StaticDFA structure\n"
        "                          [+f[ilename]:<targetfile>]   DFA table output file\n"
        "                          [+c[lassname]:<classname>]   staticDFA object name\n"
        "-pt,-parsetable       Create a StaticParseTable structure\n"
        "                          [+f[ilename]:<targetfile>]   Parse table output file\n"
        "                          [+c[lassname]:<classname>]   StaticParseTable name\n"
        "-cd,-canonical[data]  Store the canonical data to a file\n"
        "                          [[+f[ilename]]:<targetfile>] destination output file\n"
        "-cr,-conflicts        Create the extended conflict report\n"
        "                          [+f[ilename]:<targetfile>]  destination output file\n"
        "                          [+lines]                    toggle line numbers\n"
        "                          [+labels]                   toggle labels\n"
        "                          [+points]                   toggle points\n"
        "                          [+d[isplay]]                output to the screen\n"
        "-nowarn[ings]         Do not display warning messages \n"
        "-notes                Display note messages - toggle\n"
        "-stats                Display status messages - toggle\n"
        "-h[elp],-?            Display help \n"
        "                          [+msg:<message code>]       specific message code \n"
        "                          [+option:<option>]          name of a specific option\n"
        "-q[uiet]              Quiet mode\n"
        "-prog[ress]           Display programs progress as it executes\n"
        "-ver[sion]            Display the program version\n"
        "-clg                  Output the internal command line grammar\n\n"

        "*All option parameters [+parameter] are optional\n";
}


struct OutputBuffer final
{
    std::vector<String> Buffer;
    bool                Quiet;

    void Add(const String& str) {
        if (Quiet)
            Buffer.push_back(str);
        else
            std::printf("%s\n", str.data());
    }
};


// *** Execution function

void CmdLineParseHandler::Execute(Generator::StdGrammarParseData& parseData) {
    // Parser classes
    DFAGen                          dfa;
    ParseTableGen                   parseTable;
    Parse<ParseStackGenericElement> parse;
    DFATokenizer<GenericToken>      tokenizer;
    FileInputStream                 tokenFileInput;
    GenericParseHandler             parseHandler;

    // Message data
    std::vector<ParseMessage> loadGrammarMessages;
    ParseMessageBuffer*       pmessages = &parseData.GetMessageBuffer();

    bool         writeEnums = false;
    auto         fileFlags = FileOutputStream::Mode::Truncate;
    String       enumFilename;
    String       copyrightHeader;
    OutputBuffer output;

    // *** Preprocessing options

    // Message filtering

    // Errors are always reported
    unsigned messageFlags = ParseMessageBuffer::MessageError;

    if (CheckOptionParam("Message", "Stats"))
        messageFlags |= ParseMessageBuffer::MessageStats;
    if (!CheckOptionParam("Message", "NoWarnings"))
        messageFlags |= ParseMessageBuffer::MessageWarning;

    // Quiet Mode
    if (!CheckOption("QuietMode")) {
        output.Quiet = false;
        messageFlags |= ParseMessageBuffer::MessageQuickPrint;
    } else
        output.Quiet = true;

    // Configuration File
    if (CheckOption("ConfigFile")) {
        // Parse the configuration file
    }

    // Version
    if (CheckOption("Version"))
        output.Add("\nSimple Grammar Parser Generator [Version 1.0]\n"
                   "(C) Copyright 2003-2020 Status Games Corp.\n\n");

    // Global enumeration filename
    if (CheckOption("EnumFile")) {
        writeEnums = !(CheckOption("ProdEnum") || CheckOption("TermEnum") ||
                     CheckOption("NonTermEnum"));
        enumFilename = "ParseEnum.h";
        // Set filename to default if empty
        GetOptionParam("EnumFile", "Filename", enumFilename);
    }

    // Use namespaces
    const bool useNamespaces = CheckOption("Namespaces");
    String     namespaceName;

    if (useNamespaces) {
        GetOptionParam("Namespaces", "Classname", namespaceName);
        if (namespaceName.empty())
            namespaceName = "Generated"; // Default namespace name
        output.Add("Use namespace " + namespaceName);
    }

    // Use 'enum class'
    const bool useEnumClasses = CheckOption("EnumClasses");

    if (useEnumClasses)
        output.Add("Use 'enum class' instead of 'enums'");

    // Create string literals for enumeration stringification
    const bool createEnumStrings = CheckOption("EnumStrings");

    if (createEnumStrings)
        output.Add("Create string literals for enumeration stringification");

    // Help
    if (CheckOption("Help")) {
        // Display help and return
        output.Add(GetHelpText());
        goto output_results;
    }

    // *** Parse the grammar file

    // If no grammar file was passed than finish up
    if (GrammarFileName.empty())
        goto output_results;

    // Generate the pseudo-copyright header string
    copyrightHeader = CopyrightHeader();

    // Store all errors in the load message list
    pmessages->SetMessageBuffer(&loadGrammarMessages,
                                messageFlags | ParseMessageBuffer::MessageQuickPrint);

    // Open the tokenizer input file
    if (!tokenFileInput.Open(GrammarFileName)) {
        // ERROR: Opening file
        if (pmessages->GetMessageFlags() & ParseMessageBuffer::MessageError) {
            const auto str = "Failed to open the user grammar file - '" + GrammarFileName + "' ";
            const ParseMessage msg{ParseMessage::ErrorMessage, "FL0001E", str};
            pmessages->AddMessage(msg);
        }
        goto finished_executing;
    }

    // Set the grammar message buffer
    parseData.GetGrammar().GetMessageBuffer().SetMessageBuffer(&loadGrammarMessages,
                               messageFlags | ParseMessageBuffer::MessageQuickPrint);
    // Need to set buffer for lexemes too
    parseData.GetLex().GetMessageBuffer().SetMessageBuffer(&loadGrammarMessages,
                           messageFlags | ParseMessageBuffer::MessageQuickPrint);

    // If the grammar failed to load than output errors
    if (!parseData.LoadGrammar(&tokenFileInput))
        goto finished_executing;

    // Make a new dfa from the user grammar
    if (!parseData.MakeDFA(dfa)) {
        output.Add("Failed to make the DFA for the user grammar");
        goto finished_executing;
    }

    // Canonical Data setup
    // Enable the conancical debug data structure
    if (CheckOption("Canonical"))
        parseData.GetGrammar().GetDebugData().Flags |= GrammarDebugData::Canonical;

    parseData.GetGrammar().GetDebugData().Flags |= GrammarDebugData::PrintProgress;

    // Conflict Reporting setup
    // Enable the conflict reporting mechanism
    if (CheckOption("ConflictReport")) {
        parseData.GetGrammar().GetDebugData().Flags |= GrammarDebugData::ConflictReport;

        if (!CheckOptionParam("ConflictReport", "Lines"))
            parseData.GetGrammar().GetDebugData().Flags |=
                GrammarDebugData::ConflictReportNoLineNumbers;

        if (!CheckOptionParam("ConflictReport", "Labels"))
            parseData.GetGrammar().GetDebugData().Flags |=
                GrammarDebugData::ConflictReportNoLabels;

        if (!CheckOptionParam("ConflictReport", "Points"))
            parseData.GetGrammar().GetDebugData().Flags |=
                GrammarDebugData::ConflictReportNoPoints;
    }

    // *** Make a parse table

    // TableType setup
    // If it wasn't set than set it to the default type - CLR
    {
        String tableTypeStr;
        auto   tableType = ParseTableType::CLR;

        if (GetOptionParam("TableType", "Type", tableTypeStr)) {
            if (tableTypeStr == "CLR")
                tableType = ParseTableType::CLR;
            else if (tableTypeStr == "LALR")
                tableType = ParseTableType::LALR;
            else if (tableTypeStr == "LR")
                tableType = ParseTableType::LR;
        }

        if (!parseData.MakeParseTable(parseTable, tableType)) {
            output.Add("Failed to make the " + tableTypeStr + " parse table");
            goto finished_executing;
        }
    }

    // Canonical Data
    if (CheckOption("Canonical")) {
        FileOutputStream file;
        String           filename       = "CanonicalData.txt";
        const auto&      canonicalItems = parseData.GetGrammar().GetDebugData().CanonicalItems;

        // Set filename to default if empty
        GetOptionParam("Canonical", "Filename", filename);

        // Dumb the canonical set to the file
        if (file.Open(filename, FileOutputStream::Mode::Truncate)) {
            TextOutputStream tstream{file};
            for (size_t i = 0u; i < canonicalItems.size(); ++i) {
                tstream.WriteText(StringWithFormat("\nItem %zu - 0x%02X\n", i, unsigned(i)));
                for (const auto& item : canonicalItems[i])
                    tstream.WriteText(item + "\n");
            }
            output.Add("Wrote the canonical data to '" + filename + "'");
        } else {
            // ERROR: Opening file
            if (pmessages->GetMessageFlags() & ParseMessageBuffer::MessageError) {
                const auto str = "Failed to open '" + filename + "' file";
                const ParseMessage msg{ParseMessage::ErrorMessage, "FL0001E", str};
                pmessages->AddMessage(msg);
            }
        }
    }

    // Canonical Data
    if (CheckOption("ConflictReport")) {
        FileOutputStream file;
        String           filename  = "ConflictReport.txt";

        // Get the filename from command line if it exists
        GetOptionParam("ConflictReport", "Filename", filename);

        // Dumb the canonical set to the file
        if (file.Open(filename, FileOutputStream::Mode::Truncate)) {
            TextOutputStream tstream{file};
            tstream.WriteText(parseData.GetGrammar().GetDebugData().Conflicts);
            output.Add("Wrote the conflict report to '" + filename + "'");
        } else {
            // ERROR: Opening file
            if (pmessages->GetMessageFlags() & ParseMessageBuffer::MessageError) {
                const auto str = "Failed to open '" + filename + "' file";
                const ParseMessage msg{ParseMessage::ErrorMessage, "FL0001E", str};
                pmessages->AddMessage(msg);
            }
        }
    }

    // ParseHandler reduce function
    if (CheckOption("ReduceFunc")) {
        FileOutputStream file;
        String prodSwitch;
        String filename      = "ParseHandlerReduce.cpp";
        String classname     = "ParseHandler";
        String stackname     = "StackElement";
        String enumclassname = useEnumClasses ? "ProductionEnum" : "";
        String prefix        = useEnumClasses ? "" : "PE_";

        // Get the data from the command line
        GetOptionParam("ReduceFunc", "Filename", filename);
        GetOptionParam("ReduceFunc", "Classname", classname);
        GetOptionParam("ReduceFunc", "Stackname", stackname);
        GetOptionParam("ReduceFunc", "Prefix", prefix);

        // If production enumaration should be created and 'enum class' should be used,
        // then get an option param for defining enum class name from 'ProdEnum' option
        // Find a more subtle solution
        if ((CheckOption("ProdEnum") || writeEnums) && useEnumClasses)
            GetOptionParam("ProdEnum", "Classname", enumclassname);

        GrammarOutputC grammarOut{&parseData.GetGrammar(), namespaceName,
                                  useEnumClasses, createEnumStrings};

        // Create the production switch
        grammarOut.CreateProductionSwitch(prodSwitch, classname, stackname, prefix, enumclassname);

        // Open the file
        if (file.Open(filename, FileOutputStream::Mode::Truncate)) {
            TextOutputStream tstream{file};
            // Dump the pseudo-copyright header
            tstream.WriteText(copyrightHeader);
            // Dump the string
            tstream.WriteText(prodSwitch);

            output.Add("Wrote the reduce function to '" + filename + "'");
        } else {
            // ERROR: Opening file
            if (pmessages->GetMessageFlags() & ParseMessageBuffer::MessageError) {
                const auto str = "Failed to open '" + filename + "' file";
                const ParseMessage msg{ParseMessage::ErrorMessage, "FL0001E", str};
                pmessages->AddMessage(msg);
            }
        }
    }

    // Production enumeration
    if (CheckOption("ProdEnum") || writeEnums) {
        FileOutputStream file;
        String prodEnum;
        String filename  = "ProductionEnum.h";
        String classname = "ProductionEnum";
        String prefix    = useEnumClasses ? "" : "PE_";
        auto   fflags    = FileOutputStream::Mode::Truncate;

        // Set the default value
        if (!GetOptionParam("ProdEnum", "Filename", filename) && !enumFilename.empty()) {
            fflags   = fileFlags;
            filename = enumFilename;
        }
        GetOptionParam("ProdEnum", "Classname", classname);
        GetOptionParam("ProdEnum", "Prefix", prefix);

        GrammarOutputC grammarOut{&parseData.GetGrammar(), namespaceName,
                                  useEnumClasses, createEnumStrings};

        // Create the production switch
        grammarOut.CreateProductionEnum(prodEnum, classname, prefix);

        // Open the file
        if (file.Open(filename, fflags)) {
            TextOutputStream tstream{file};

            // If it is the beginning of the generated file...
            if (fflags == FileOutputStream::Mode::Truncate) {
                // ...dump the pseudo-copyright header
                tstream.WriteText(copyrightHeader);
            } else {
                // Otherwise, add two empty lines for indentation
                tstream.WriteText("\n\n");
            }

            // Dump the string
            tstream.WriteText(prodEnum);

            output.Add("Wrote the production enumeration to '" + filename + "'");

            if (filename == enumFilename)
                fileFlags = FileOutputStream::Mode::Append;
        } else {
            // ERROR: Opening file
            if (pmessages->GetMessageFlags() & ParseMessageBuffer::MessageError) {
                const auto str = "Failed to open '" + filename + "' file";
                const ParseMessage msg{ParseMessage::ErrorMessage, "FL0001E", str};
                pmessages->AddMessage(msg);
            }
        }
    }

    // Terminal enumeration
    if (CheckOption("TermEnum") || writeEnums) {
        FileOutputStream file;
        String termEnum;
        String filename  = "TermEnum.h";
        String classname = "TermEnum";
        String prefix    = useEnumClasses ? "" : "TE_";
        auto   fflags    = FileOutputStream::Mode::Truncate;

        // Set the default value
        if (!GetOptionParam("TermEnum", "Filename", filename) && !enumFilename.empty()) {
            fflags   = fileFlags;
            filename = enumFilename;
        }
        GetOptionParam("TermEnum", "Classname", classname);
        GetOptionParam("TermEnum", "Prefix", prefix);

        // Create the terminal enumeration
        GrammarOutputC grammarOut{&parseData.GetGrammar(), namespaceName,
                                  useEnumClasses, createEnumStrings};
        grammarOut.CreateTerminalEnum(termEnum, parseData.GetLex(), classname, prefix);

        // Open the file
        if (file.Open(filename, fflags)) {
            TextOutputStream tstream{file};

            // If it is the beginning of the generated file...
            if (fflags == FileOutputStream::Mode::Truncate) {
                // ...dump the pseudo-copyright header
                tstream.WriteText(copyrightHeader);
            } else {
                // Otherwise, add two empty lines for indentation
                tstream.WriteText("\n\n");
            }

            // Dump the string
            tstream.WriteText(termEnum);

            output.Add("Wrote the terminal enumeration to '" + filename + "'");

            if (filename == enumFilename)
                fileFlags = FileOutputStream::Mode::Append;
        } else {
            // ERROR: Opening file
            if (pmessages->GetMessageFlags() & ParseMessageBuffer::MessageError) {
                const auto str = "Failed to open '" + filename + "' file";
                const ParseMessage msg{ParseMessage::ErrorMessage, "FL0001E", str};
                pmessages->AddMessage(msg);
            }
        }
    }

    // NonTerm enumeration
    if (CheckOption("NonTermEnum") || writeEnums) {
        FileOutputStream file;
        String nontermEnum;
        String filename  = "NonTermEnum.h";
        String classname = "NonTermEnum";
        String prefix    = useEnumClasses ? "" : "NTE_";
        auto   fflags    = FileOutputStream::Mode::Truncate;

        // Set the default value
        if (!GetOptionParam("NonTermEnum", "Filename", filename) && !enumFilename.empty()) {
            fflags   = fileFlags;
            filename = enumFilename;
        }
        GetOptionParam("NonTermEnum", "Classname", classname);
        GetOptionParam("NonTermEnum", "Prefix", prefix);

        // Create the non terminal enumeration
        GrammarOutputC grammarOut{&parseData.GetGrammar(), namespaceName,
                                  useEnumClasses, createEnumStrings};
        grammarOut.CreateNonterminalEnum(nontermEnum, classname, prefix);

        // Open the file
        if (file.Open(filename, fflags)) {
            TextOutputStream tstream{file};

            // If it is the beginning of the generated file...
            if (fflags == FileOutputStream::Mode::Truncate) {
                // ...dump the pseudo-copyright header
                tstream.WriteText(copyrightHeader);
            } else {
                // Otherwise, add two empty lines for indentation
                tstream.WriteText("\n\n");
            }

            // Dump the string
            tstream.WriteText(nontermEnum);

            output.Add("Wrote the nonterminal enumeration to '" + filename + "'");

            if (filename == enumFilename)
                fileFlags = FileOutputStream::Mode::Append;
        } else {
            // ERROR: Opening file
            if (pmessages->GetMessageFlags() & ParseMessageBuffer::MessageError) {
                const auto str = "Failed to open '" + filename + "' file";
                const ParseMessage msg{ParseMessage::ErrorMessage, "FL0001E", str};
                pmessages->AddMessage(msg);
            }
        }
    }

    // StaticDFA
    if (CheckOption("StaticDFA")) {
        FileOutputStream file;
        String staticDFA;
        String filename  = "StaticDFA.h";
        String classname = "StaticDFAData";

        // Set the default value
        GetOptionParam("StaticDFA", "Filename", filename);
        GetOptionParam("StaticDFA", "Classname", classname);

        // Create the production switch
        dfa.CreateStaticDFA(staticDFA, classname, namespaceName);

        // Open the file
        if (file.Open(filename, FileOutputStream::Mode::Truncate)) {
            TextOutputStream tstream{file};
            // Dump the pseudo-copyright header
            tstream.WriteText(copyrightHeader);
            // Dump the string
            tstream.WriteText(staticDFA);

            output.Add("Wrote the static DFA structure to '" + filename + "'");
        } else {
            // ERROR: Opening file
            if (pmessages->GetMessageFlags() & ParseMessageBuffer::MessageError) {
                const auto str = "Failed to open '" + filename + "' file";
                const ParseMessage msg{ParseMessage::ErrorMessage, "FL0001E", str};
                pmessages->AddMessage(msg);
            }
        }
    }

    // StaticParseTable
    if (CheckOption("StaticParseTable")) {
        FileOutputStream file;
        String staticParseTable;
        String filename  = "StaticParseTable.h";
        String classname = "StaticParseTableData";

        // Set the default value
        GetOptionParam("StaticParseTable", "Filename", filename);
        GetOptionParam("StaticParseTable", "Classname", classname);

        // Create the production switch
        parseTable.CreateStaticParseTable(staticParseTable, classname, namespaceName);

        // Open the file
        if (file.Open(filename, FileOutputStream::Mode::Truncate)) {
            TextOutputStream tstream{file};
            // Dump the pseudo-copyright header
            tstream.WriteText(copyrightHeader);
            // Dump the string
            tstream.WriteText(staticParseTable);

            output.Add("Wrote the StaticParseTable structure to '" + filename + "'");
        } else {
            // ERROR: Opening file
            if (pmessages->GetMessageFlags() & ParseMessageBuffer::MessageError) {
                const auto str = "Failed to open '" + filename + "' file";
                const ParseMessage msg{ParseMessage::ErrorMessage, "FL0001E", str};
                pmessages->AddMessage(msg);
            }
        }
    }

    // Test an expression
    // Outputs the reductions
    if (CheckOption("ParseData")) {
        InputStream*          pparseDataInput;
        MemBufferInputStream  stringStream;
        FileInputStream       file;

        // Store all parsing errors in one common buffer
        String strdata;
        if (GetOptionParam("ParseData", "StringData", strdata)) {
            // Setup the expression input stream
            stringStream.SetInputString(strdata);
            pparseDataInput = &stringStream;
        } else {
            String filename = "ParseData.txt";
            GetOptionParam("ParseData", "Filename", filename);

            if (!file.Open(filename)) {
                // ERROR: Opening file
                if (pmessages->GetMessageFlags() & ParseMessageBuffer::MessageError) {
                    const auto str = "Failed to open '" + filename + "' file";
                    const ParseMessage msg{ParseMessage::ErrorMessage, "FL0001E", str};
                    pmessages->AddMessage(msg);
                }
                goto finished_parsing;
            }
            pparseDataInput = &file;
        }

        // Create the tokenizer and set the input stream
        if (!tokenizer.Create(&dfa, pparseDataInput)) {
            output.Add("Failed to create and initialize the tokenizer");
            goto finished_parsing;
        }

        // Create and initialize the parser
        if (!parse.Create(&parseTable, &tokenizer)) {
            output.Add("Failed to create and initialize the parse class");
            goto finished_parsing;
        }

        parseData.GetGrammar().CreateProductionVector(parseHandler.Productions);

        // Set the message buffer
        if (CheckOptionParam("ParseData", "PrintReductions"))
            parseHandler.Messages.SetMessageBuffer(&loadGrammarMessages,
                                                   ParseMessageBuffer::MessageStats |
                                                   ParseMessageBuffer::MessageQuickPrint);

        // Parse the expression
        if (!parse.DoParse(parseHandler)) {
            output.Add("Parse Error - failed to parse the test expression");
            goto finished_parsing;
        }

        output.Add("Successfully parsed the test input");

    finished_parsing:;
    }

finished_executing :

{
    // Message count variables
    const size_t errorCount   = pmessages->GetMessageCount(ParseMessageBuffer::MessageError);
    const size_t warningCount = pmessages->GetMessageCount(ParseMessageBuffer::MessageWarning);
    const size_t noteCount    = pmessages->GetMessageCount(ParseMessageBuffer::MessageNote);

    // Get the error data
    std::vector<String> msgStrings;
    String              buffer;

    // If there are any messages from loading the grammar write them
    pmessages->PrintMessages(msgStrings);
    output.Buffer.insert(output.Buffer.end(), msgStrings.begin(), msgStrings.end());

    // Output the final message count information
    if (noteCount > 0u)
        buffer = StringWithFormat("\n%zu error(s), %zu warning(s), %zu note(s)\n",
                                  errorCount, warningCount, noteCount);
    else
        buffer = StringWithFormat("\n%zu error(s), %zu warning(s)\n", errorCount, warningCount);

    output.Add(buffer);
}

output_results:

    // Quiet mode
    if (output.Quiet) {
        FileOutputStream file;
        String           filename = "ParserOutput.txt";

        GetOptionParam("QuietMode", "Filename", filename);

        // Open the file and dump the string
        if (file.Open(filename, FileOutputStream::Mode::Truncate)) {
            TextOutputStream tstream{file};
            for (const auto& str : output.Buffer)
                tstream.WriteText(str);
        }
    }
}

} // namespace Yacc
} // namespace Generator
} // namespace SGParser
