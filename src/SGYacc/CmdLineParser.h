// Filename:  CmdLineParser.h
// Content:   Command line parser program
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_GENERATOR_YACC_CMDLINEPARSER_H
#define INC_SGPARSER_GENERATOR_YACC_CMDLINEPARSER_H

#include "Parser.h"
#include "Grammar.h"
#include "ParseData.h"

#include <unordered_map>
#include <vector>

namespace SGParser
{
namespace Generator
{
namespace Yacc
{

// *** Command Line structures

struct CmdLineParam final
{
    String Name;
    String Value;
};

// Base option
struct CmdLineOption final
{
    String                                   Name;
    std::unordered_map<String, CmdLineParam> Params;
};

// *** YACC Command Line parse handler

class CmdLineParseHandler final : public ParseHandler<ParseStackGenericElement>
{
public:
    std::vector<Production*>           Productions;
    ParseMessageBuffer                 Messages;
    std::unordered_map<String, String> OptionInfoSet;
    size_t                             ErrorCount = 0u;

    CmdLineParseHandler() = default;

    // Option functionality
    const char* GetHelpText() const noexcept;

    // Check to see if an option exists
    bool CheckOption(const String& optionName) const {
        return (Options.find(optionName) != Options.end());
    }

    // Check to see if an option parameter exists
    bool CheckOptionParam(const String& optionName, const String& paramName) const;

    // Return the option parameter String
    bool GetOptionParam(const String& optionName, const String& paramName, String& dest) const;

    // Standard ParseHandler functions
    void Execute(StdGrammarParseData& parseData);
    bool Reduce(Parse<ParseStackGenericElement>& parse, unsigned productionID) override;

private:
    String                                    GrammarFileName;
    std::unordered_map<String, CmdLineOption> Options;

    void SetOption(const String& option);

    void SetOptionParam(const String& option, const String& param, const String& value);

    void AddDuplicateOptionMessage(char* poption);
    void AddDuplicateParamMessage(char* poption, char* pparam);

    int  HandleDuplicateStringParam(String& source, char* poption, char* pparam);
    int  HandleDuplicateIntegerParam(int source, char* poption, char* pparam);

    // Generate pseudo-copyright header string based on GrammarFileName
    String CopyrightHeader() const;
};

} // namespace Yacc
} // namespace Generator
} // namespace SGParser

#endif // INC_SGPARSER_GENERATOR_YACC_CMDLINEPARSER_H
