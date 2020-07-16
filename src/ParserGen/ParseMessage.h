// Filename:  ParseMessage.h
// Content:   Class used for parser message reporting
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_GENERATOR_PARSEMESSAGE_H
#define INC_SGPARSER_GENERATOR_PARSEMESSAGE_H

#include "SGString.h"

#include <vector>

namespace SGParser {
namespace Generator {

// ***** Parse Massage

// Message Data structure
struct ParseMessage final
{
    // The type of message
    enum Type : unsigned
    {
        ErrorMessage    = 0x0000'0000,
        WarningMessage  = 0x0000'0001,
        NoteMessage     = 0x0000'0002,
        StatMessage     = 0x0000'0003
    };

    // Message display flags
    enum Display : unsigned
    {
        // Displays specific info
        DisplayType     = 0x0000'0010,
        DisplayId       = 0x0000'0020,
        DisplayFile     = 0x0000'0040,
        DisplayLine     = 0x0000'0080,
        DisplayOffset   = 0x0000'0100,
        DisplayName     = 0x0000'0200,
        DisplayMessage  = 0x0000'0400,
        // Displays all the info
        DisplayAll      = 0x0000'07F0,
        // Displays no info
        DisplayNone     = 0x0000'0800,
        // Default display is used for each message type
        DisplayDefault  = 0x0000'0000
    };

    // Masks
    enum Masks : unsigned 
    {
        // Internal type mask
        TypeMask        = 0x0000'000F,
        // Display mask
        DisplayMask     = 0x0000'0FF0
    };

    // Const for describing empty value for line or offset 
    static constexpr size_t NoValue = size_t(-1);

    // Message flags (stores type and display flags)
    unsigned Flags    = 0u;
    // Message Name
    String   Name     = "";
    // Actual message
    String   Message  = "";
    // Special message ID
    unsigned Id       = 0u;
    // Line number (NoValue for none)
    size_t   Line     = NoValue;
    // Offset number (NoValue for none)
    size_t   Offset   = NoValue;
    // File name
    String   FileName = "";

    // Constructors
    ParseMessage() = default;

    ParseMessage(Type type, const String& name, const String& message, unsigned id = 0u,
                 size_t line = NoValue, size_t offset = NoValue, const String& fileName = "",
                 unsigned flags = 0u) {
        SetParseMessage(type, name, message, id, line, offset, fileName, flags);
    }

    // Initializer
    void SetParseMessage(Type type, const String& name, const String& message, unsigned id = 0u,
                         size_t line = NoValue, size_t offset = NoValue,
                         const String& fileName = "", unsigned flags = 0u) {
        Flags    = type | (flags & ~TypeMask);
        Name     = name;
        Message  = message;
        Id       = id;
        Line     = line;
        Offset   = offset;
        FileName = fileName;
    }

    // Determines the message type
    Type GetMessageType() const noexcept { return Type(Flags & TypeMask); }
};


// ***** Parse Message Buffer

class ParseMessageBuffer final
{
public:
    using Messages = std::vector<ParseMessage>;

    // Messages to report on table construction
    enum : unsigned
    {
        // Report:
        MessageError                   = 0x0000'0001,
        MessageWarning                 = 0x0000'0002,
        MessageNote                    = 0x0000'0004,
        MessageStats                   = 0x0000'0008,

        // Don't report:
        // Disabled warnings
        NoMessageUnreachableProduction = 0x0000'0100,

        // Common flags
        MessageNone                    = 0x0000'0000,
        MessageAll                     = 0x0000'00FF,
        MessageStandard                = MessageAll | NoMessageUnreachableProduction,

        MessageQuickPrint              = 0x8000'0000
    };

public:
    // Add a message to the buffer
    bool     AddMessage(const ParseMessage& data);
    // Clear all the messages
    void     ClearMessages();
    // Get message info
    size_t   GetMessageCount(unsigned flags = MessageStandard) const noexcept;

    // *** Message Access functionality

    // Set message reporting flags
    void     SetMessageFlags(unsigned flags) noexcept { Flags = flags; }
    // Set message reporting flags
    unsigned GetMessageFlags() const noexcept         { return Flags; }

    // Set message reporting buffer
    void     SetMessageBuffer(Messages* pmessages, unsigned flags = MessageStandard) noexcept;
    // Obtain message buffer
    std::vector<ParseMessage>* GetMessageBuffer() const noexcept { return pMessages; }

    // *** Utility functions

    // Fills in the String buffer with the message data
    bool     PrintMessages(std::vector<String>& stringBuffer,
                           unsigned flags = MessageStandard) const;
    // Print out the single message
    void     PrintMessage(String& msgStr, const ParseMessage& source, unsigned flags) const;

private:
    unsigned  Flags     = MessageAll;

    // Messages (errors, warnings, notes, stats, etc.)
    Messages* pMessages = nullptr;
};

} // namespace Generator
} // namespace SGParser

#endif // INC_SGPARSER_GENERATOR_PARSEMESSAGE_H
