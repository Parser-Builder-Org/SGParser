// Filename:  MainSGYacc.cpp
// Content:   Simple Grammar Command line parser program (SGYacc)
// Provided AS IS under MIT License; see LICENSE file in root folder.

#include "CmdLineGrammar.h"
#include "CmdLineParser.h"
#include "SGString.h"

#include <cstdio>

// *** Main SGYacc program loop

int main(int argc, char* argv[])
{
    using namespace SGParser;
    using namespace Generator;
    using namespace Yacc;

    // *** Initialize the YACC grammar and parser

    // Parser data
    Generator::StdGrammarParseData  parseData;
    DFAGen                          dfa;
    ParseTableGen                   parseTable;
    Parse<ParseStackGenericElement> parse;
    DFATokenizer<GenericToken>      tokenizer;
    CmdLineParseHandler             cmdLineParseHandler;

    // Message data
    std::vector<ParseMessage> loadMessages;
    std::vector<ParseMessage> parseMessages;

    // Input data
    MemBufferInputStream exprStringInput;

    // Command line
    String cmdLine;
    String str;
    int    error = 0;
    bool   result;

    // Build a string of the command line options
    for (int i = 1; i < argc; ++i)
        cmdLine += String{argv[i]} + " ";

    // *** Internal parser initialization and creation

    // Store all errors in the load message list
    parseData.GetMessageBuffer().SetMessageBuffer(&loadMessages, ParseMessageBuffer::MessageError);
    parseData.GetLex().GetMessageBuffer().SetMessageBuffer(&loadMessages,
                                                           ParseMessageBuffer::MessageError);
    parseData.GetGrammar().GetMessageBuffer().SetMessageBuffer(&loadMessages,
                                                               ParseMessageBuffer::MessageError);

    // Load the command line grammar, if failed than output errors
    MemBufferInputStream grammarStream{cmdLineGrammar};
    if (!parseData.LoadGrammar(&grammarStream)) {
        str   = "Failed to load the command line grammar";
        error = 2;
        goto finished_parsing;
    }

    // Setup the dfa message buffer
    dfa.GetMessageBuffer().SetMessageBuffer(&loadMessages, ParseMessageBuffer::MessageError);

    // Make a new dfa from the command line grammar file
    if (!parseData.MakeDFA(dfa)) {
        str   = "Failed to create the DFA";
        error = 3;
        goto finished_parsing;
    }

    // Make a parse table
    if (!parseData.MakeParseTable(parseTable, ParseTableType::CLR)) {
        str   = "Failed to create the parse table";
        error = 4;
        goto finished_parsing;
    }

    // *** Execute the command line

    if (cmdLine.empty()) {
        std::printf("%s", cmdLineParseHandler.GetHelpText());
        goto finished_parsing;
    }

    parseMessages = loadMessages;

    // Store all parsing errors in one common buffer
    parseData.GetMessageBuffer().SetMessageBuffer(&parseMessages,
                                                  ParseMessageBuffer::MessageError);

    // Setup the command line input stream
    exprStringInput.SetInputString(cmdLine);

    // Create the tokenizer, set the input stream
    // Make sure we track the position
    tokenizer.Create(&dfa, &exprStringInput);

    // Create and initialize the parser
    if (!parse.Create(&parseTable, &tokenizer)) {
        str   = "Failed to create the parser";
        error = 10;
        goto finished_parsing;
    }

    // Set the message buffer
    cmdLineParseHandler.Messages.SetMessageBuffer(&parseMessages,
                                                  ParseMessageBuffer::MessageError |
                                                  ParseMessageBuffer::MessageWarning);
    parseData.GetGrammar().CreateProductionVector(cmdLineParseHandler.Productions);

    // *** Parse the expression

    result = parse.DoParse(cmdLineParseHandler);

    // Errors were encountered while parsing the command line input
    if (!result || cmdLineParseHandler.ErrorCount > 0u) {
        str   = "Failed to parse the command line input";
        error = 11;
        goto finished_parsing;
    }

    // *** Execute the commands

    // Use the parse data that was already created
    parseData.ClearGrammar();
    cmdLineParseHandler.Execute(parseData);

    // *** Output the results

finished_parsing:

    if (error) {
        // Output the error stage
        std::printf("%s\n", str.data());

        if (parseData.GetMessageBuffer().GetMessageCount()) {
            // Get the error data
            const auto errorCount =
                parseData.GetMessageBuffer().GetMessageCount(ParseMessageBuffer::MessageError);
            const auto warningCount =
                parseData.GetMessageBuffer().GetMessageCount(ParseMessageBuffer::MessageWarning);
            const auto noteCount =
                parseData.GetMessageBuffer().GetMessageCount(ParseMessageBuffer::MessageNote);

            // Create a vector of messages strings
            std::vector<String> msgStrings;
            parseData.GetMessageBuffer().PrintMessages(msgStrings);

            // Print out all the messages
            for (const auto& msg: msgStrings)
                std::printf("%s\n", msg.data());

            // Output the final message count information
            std::printf("\n%zu error(s), %zu warning(s)", errorCount, warningCount);

            if (noteCount > 0u)
                std::printf(", %zu note(s)\n", noteCount);
            else
                std::printf("\n");
        }
    }

    return error;
}
