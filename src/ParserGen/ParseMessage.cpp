// Filename:  ParseMessage.h
// Content:   Class used for parser message reporting
// Provided AS IS under MIT License; see LICENSE file in root folder.

#include "ParseMessage.h"

namespace SGParser
{
namespace Generator
{

// *** Message Buffer functions

// Add a message to the buffer
bool ParseMessageBuffer::AddMessage(const ParseMessage& msg) {
    // Print out message if debug PrintToScreen flag set
    if (Flags & MessageQuickPrint) {
        String msgstr;
        PrintMessage(msgstr, msg, 0u);
        std::printf("%s\n", msgstr.data());
    }

    // Make sure the message buffer is valid
    if (pMessages) {
        pMessages->push_back(msg);
        return true;
    }

    return false;
}


// Clear all the messages
void ParseMessageBuffer::ClearMessages() {
    if (pMessages)
        pMessages->clear();
}


// Set the message buffer to point to a user specified buffer
void ParseMessageBuffer::SetMessageBuffer(Messages* pmessages, unsigned flags) noexcept {
    pMessages = pmessages;
    Flags     = pMessages ? flags : MessageNone;
}


// Print out the single message
void ParseMessageBuffer::PrintMessage(String& msgStr, const ParseMessage& source,
                                      [[maybe_unused]] unsigned flags) const {
    unsigned displayFlags = 0u;
    bool     line         = false;
    String   type;
    String   msg;

    // Output the type of message
    switch (source.GetMessageType()) {
        case ParseMessage::ErrorMessage:
            type         = "Error   : ";
            displayFlags = ParseMessage::DisplayType | ParseMessage::DisplayName |
                           ParseMessage::DisplayMessage | ParseMessage::DisplayFile |
                           ParseMessage::DisplayLine | ParseMessage::DisplayOffset;
            break;

        case ParseMessage::WarningMessage:
            type         = "Warning : ";
            displayFlags = ParseMessage::DisplayType | ParseMessage::DisplayName |
                           ParseMessage::DisplayMessage | ParseMessage::DisplayFile |
                           ParseMessage::DisplayLine | ParseMessage::DisplayOffset;
            break;

        case ParseMessage::NoteMessage:
            type         = "Note    : ";
            displayFlags = ParseMessage::DisplayType | ParseMessage::DisplayName |
                           ParseMessage::DisplayMessage;
            break;

        case ParseMessage::StatMessage:
            type         = "Stat    : ";
            displayFlags = ParseMessage::DisplayType | ParseMessage::DisplayMessage;
            break;
    }

    if ((source.Flags & ParseMessage::DisplayMask) != ParseMessage::DisplayDefault)
        displayFlags = (source.Flags & ParseMessage::DisplayMask);

    if (displayFlags & ParseMessage::DisplayType)
        msg += type;

    if (displayFlags & ParseMessage::DisplayName && !source.Name.empty())
        msg += source.Name + " - ";

    // If the line was recorded than output it
    if (displayFlags & ParseMessage::DisplayFile && !source.FileName.empty()) {
        line = true;
        msg += StringWithFormat("('%s'", source.FileName.data());
    }

    if (displayFlags & ParseMessage::DisplayLine && source.Line != ParseMessage::NoValue) {
        msg += (line ? ", " : "(") + StringWithFormat("Ln:%zu", source.Line + 1u);
        line = true;
    }
    // same goes for the offset
    if (displayFlags & ParseMessage::DisplayOffset && source.Offset != ParseMessage::NoValue)
        msg += (line ? ", " : "(") + StringWithFormat("Col:%zu) ", source.Offset + 1u);
    else if (line)
        msg += ") ";

    if (displayFlags & ParseMessage::DisplayMessage)
        msg += source.Message;

    msgStr.swap(msg);
}


// Fills in the string buffer with the message data
bool ParseMessageBuffer::PrintMessages(std::vector<String>& stringBuffer,
                                       [[maybe_unused]] unsigned flags) const {
    // Make sure we have a valid message buffer
    if (!pMessages)
        return false;

    stringBuffer.clear();

    // Build the list of message strings
    for (const auto& message: *pMessages) {
        String msg;
        PrintMessage(msg, message, 0u);
        stringBuffer.push_back(msg);
    }

    return true;
}


// Get message count matching the message flags
size_t ParseMessageBuffer::GetMessageCount(unsigned flags) const noexcept {
    // Make sure we have a valid message buffer
    if (!pMessages)
        return 0u;

    if (flags == MessageAll)
        return pMessages->size();

    // Message count
    size_t count = 0u;

    for (const auto& message: *pMessages) {
        // Determine the message type and increment the
        // count if it matches what the user asked for
        switch (message.GetMessageType()) {
            case ParseMessage::ErrorMessage:   if (flags & MessageError)   ++count; break;
            case ParseMessage::WarningMessage: if (flags & MessageWarning) ++count; break;
            case ParseMessage::NoteMessage:    if (flags & MessageNote)    ++count; break;
            case ParseMessage::StatMessage:    if (flags & MessageStats)   ++count; break;
        }
    }

    return count;
}

} // namespace Generator
} // namespace SGParser
