// Filename:  Lex.cpp
// Content:   Lex and Lexemes class, converts REs & labels to DFA
// Provided AS IS under MIT License; see LICENSE file in root folder.

#include "Lex.h"
#include "RegExprProdEnum.h" // Include regular expression production enumeration for parsing
#include "RegExpTokenizer.h"

namespace SGParser
{
namespace Generator
{

struct RegExprParseToken final : public TokenCode
{
    unsigned ch = 0u;

    // We don't track positions in regular expression
    using PosTracker      = NullPosTracker;
    using InputCharReader = TokenCharReaderBase<TokenizerBase::ByteReader, PosTracker>;
    using TokenCharReader =
              TokenCharReaderBase<TokenizerBase::BufferRangeByteReader, NullPosTracker>;
    using Tokenizer       = RegExpTokenizer<RegExprParseToken>;

    // Read-in from tokenizer function
    void CopyFromTokenizer(CodeType code, const Tokenizer& tokenizer) {
        Code = code;
        ch   = tokenizer.GetTokenChar();
    }
};


// *** Creates an NFA Regular Expression

// Constructs an NFA from a regular expression (using regular expression parser)
struct RegExprNFAParseElement final : public ParseStackElement<RegExprParseToken>
{
    enum class RegExprDataType
    {
        Null,
        NFA,
        Vector,
        Char
    };

    RegExprDataType            DataType = RegExprDataType::Null;

    union
    {
        NFA*                   pNFA     = nullptr;
        std::vector<unsigned>* pVec;
        unsigned               ch;
    };

    using ParseStackElement::SetErrorData;
    using ParseStackElement::Cleanup;

    void ShiftToken(TokenType& token, TokenStream<TokenType>& stream) {
        ch = token.ch;
        // If we are shifting token, parser will be looking at the next character next
        // So we can do this
        // Essentially, we are ignoring the next token code, and turning it into a character
        if (ch == '\\' && token.Code != TokenCode::TokenEOF) {
            TokenType tok;
            stream.GetNextToken(tok);
            ch = tok.ch;
            // Process characters with special meanings
            switch (ch) {
                case 'n': ch = '\n'; break;
                case 't': ch = '\t'; break;
                case 'v': ch = '\v'; break;
                case 'r': ch = '\r'; break;
                case 'f': ch = '\f'; break;
                case 'b': ch = '\b'; break;
            }
        }
    }

    void Destroy() {
        switch (DataType) {
            case RegExprDataType::Null:                break;
            case RegExprDataType::NFA:    delete pNFA; break;
            case RegExprDataType::Vector: delete pVec; break;
            case RegExprDataType::Char:                break;
        }
        DataType = RegExprDataType::Null;
    }
};


// *** Regular expression parse handler for NFA

class RegExprNFAParseHandler final : public ParseHandler<RegExprNFAParseElement>
{
public:
    unsigned                LexemeID   = NFA::InvalidLexeme;
    Lex*                    pLex       = nullptr;
    std::map<String, NFA*>* pMacroNFAs = nullptr;

    bool Reduce(Parse<RegExprNFAParseElement>& parse, unsigned productionID) override {
        switch (productionID) {
            // RegExp --> RegExp  A
            case RE_RegExpConcat:
                // Make a NFA from A and B. Put in parseStack
                parse[0].pNFA->Concat(*parse[1].pNFA);
                parse[1].Destroy();
                break;

                // RegExp --> A
            case RE_RegExp:
                break;

                // A --> A '|' B
            case RE_AOr:
                // Make a NFA from A and B. Put in parseStack
                parse[0].pNFA->Or(*parse[2].pNFA);
                parse[2].Destroy();
                break;

                // A --> B
            case RE_A:
                break;

                // B --> B '*'
            case RE_BStar:
                // Make a NFA from A*
                parse[0].pNFA->Kleene(NFA::KleeneType::ConnectBoth);
                break;

                // B --> B '+'
            case RE_BPlus:
                // Make a NFA from A+
                parse[0].pNFA->Kleene(NFA::KleeneType::ConnectBack);
                break;

                // B --> B '?'
            case RE_BQuestion:
                // Make a NFA for one optional character
                parse[0].pNFA->Kleene(NFA::KleeneType::ConnectEmpty);
                break;

                // B --> C
            case RE_B:
                break;

                // C --> '(' RegExp ')'
            case RE_CParen:
                parse[0].pNFA     = parse[1].pNFA;
                parse[0].DataType = parse[1].DataType;
                break;

                // C --> character
            case RE_CChar:
                // Make an NFA from the character. Assign to parseStack
                parse[0].pNFA     = new NFA{parse[0].ch, LexemeID};
                parse[0].DataType = RegExprNFAParseElement::RegExprDataType::NFA;
                break;

                // C --> '.'
            case RE_CDot: {
                // Construct a list of all characters but '\n'
                std::vector<unsigned> charList;
                charList.reserve(253u);
                for (unsigned k = 1u; k <= 255u; k++)
                    if (k != '\n' && k != '\r')
                        charList.push_back(k);

                // ...and then make the NFA
                parse[0].pNFA     = new NFA{charList, LexemeID};
                parse[0].DataType = RegExprNFAParseElement::RegExprDataType::NFA;
                break;
            }

                // C --> '[' GroupSet ']'
            case RE_CGroupSet: {
                // Make a NFA out of the sequence
                const auto pgroupSet = parse[1].pVec;
                std::vector<unsigned> charList;

                // Construct a list of all the characters represented by the pgroupSet...
                for (size_t i = 0u; i < pgroupSet->size(); i += 2u)
                    for (unsigned j = (*pgroupSet)[i]; j <= (*pgroupSet)[i + 1u]; ++j)
                        charList.push_back(j);

                // Delete std::vector
                parse[1].Destroy();

                // ...and then make the NFA
                parse[0].pNFA     = new NFA{charList, LexemeID};
                parse[0].DataType = RegExprNFAParseElement::RegExprDataType::NFA;
                break;
            }

                // C --> '[' '^' GroupSet ']'
            case RE_CNotGroupSet: {
                // Make a NFA out of the sequence
                const auto            pgroupSet = parse[2].pVec;
                std::vector<unsigned> charList;

                // Construct a list of all the characters NOT represented by the pgroupSet...
                // NOTE: Depends on 0..255 ASCII
                for (unsigned k = 1u; k <= 255u; ++k) {
                    bool f = false;
                    for (size_t i = 0u; i < pgroupSet->size(); i += 2u)
                        for (unsigned j = (*pgroupSet)[i]; j <= (*pgroupSet)[i + 1u]; ++j)
                            if (j == k)
                                f = true;

                    if (!f)
                        charList.push_back(k);
                }
                // Delete std::vector
                parse[2].Destroy();

                // ...and then make the NFA
                parse[0].pNFA     = new NFA{charList, LexemeID};
                parse[0].DataType = RegExprNFAParseElement::RegExprDataType::NFA;
                break;
            }

                // C --> '{' CharSet '}'
            case RE_CCharSet: {
                // Generate a string from the char std::vector
                SG_ASSERT(parse[1].pVec);
                String macroName;
                macroName.reserve(parse[1].pVec->size());
                for (auto const& ord : *parse[1].pVec) {
                    macroName.push_back(char(ord));
                }

                // Delete the character std::vector
                parse[1].Destroy();

                // Try to find it in macro list
                SG_ASSERT(pMacroNFAs != nullptr);
                auto const& map = *pMacroNFAs;
                auto const& iter = map.find(macroName);
                if (iter == map.end()) {
                    std::string names;
                    for (auto const& [name, _] : map)
                        names += " '" + name + '\'';
                    pLex->CheckForErrorAndReport("Macro '%s' not defined; there are %zu known macros:%s",
                                                 macroName.data(), map.size(), names.data());
                    // Bad!
                    return false;
                }

                // Get the NFA from the Macros list
                const auto pnfa = iter->second;

                if (!pnfa) {
                    pLex->CheckForErrorAndReport("Invalid regular expression in macro '%s' used",
                                                 macroName.data());
                    // Bad!
                    return false;
                }

                parse[0].pNFA     = new NFA{*pnfa, LexemeID};
                parse[0].DataType = RegExprNFAParseElement::RegExprDataType::NFA;
                break;
            }

            // GroupSet --> character
            case RE_GroupSetChar: {
                // Start off a new std::vector of char pairs
                const auto code   = parse[0].ch;

                parse[0].pVec     = new std::vector<unsigned>;
                parse[0].DataType = RegExprNFAParseElement::RegExprDataType::Vector;
                parse[0].pVec->push_back(code);
                parse[0].pVec->push_back(code);
                break;
            }

                // GroupSet --> character GroupSet
            case RE_GroupSetCharGroupSet:
                // Add the char to the end of the CharSet std::vector
                parse[1].pVec->push_back(parse[0].ch);
                parse[1].pVec->push_back(parse[0].ch);
                parse[0].pVec     = parse[1].pVec;
                parse[0].DataType = parse[1].DataType;
                break;

                // GroupSet --> character '-' character
            case RE_GroupSetCharList: {
                // Start off a new std::vector of char pairs
                const auto code   = parse[0].ch;
                const auto code2  = parse[2].ch;

                parse[0].pVec     = new std::vector<unsigned>;
                parse[0].DataType = RegExprNFAParseElement::RegExprDataType::Vector;
                parse[0].pVec->push_back(code);
                parse[0].pVec->push_back(code2);
                break;
            }

                // GroupSet --> character '-' character GroupSet
            case RE_GroupSetCharListGroupSet: {
                // Add the char to the end of the CharSet std::vector
                const auto code   = parse[0].ch;
                const auto code2  = parse[2].ch;

                parse[3].pVec->push_back(code);
                parse[3].pVec->push_back(code2);
                parse[0].pVec     = parse[3].pVec;
                parse[0].DataType = parse[3].DataType;
                break;
            }

                // CharSet --> character
            case RE_CharSet: {
                // Get the code and add it to the new std::vector
                const auto code   = parse[0].ch;
                // Create a new std::vector of chars
                parse[0].pVec     = new std::vector<unsigned>;
                parse[0].DataType = RegExprNFAParseElement::RegExprDataType::Vector;
                parse[0].pVec->push_back(code);
                break;
            }

                // CharSet --> CharSet character
            case RE_CharSetChar:
                // Add the char to the end of the CharSet std::vector
                parse[0].pVec->push_back(parse[1].ch);
                break;

            default:
                parse[0].pNFA     = nullptr;
                parse[0].DataType = RegExprNFAParseElement::RegExprDataType::Null;
                break;
        }

        return true;
    }
};


// *** RegExprDFA parse functionality

struct RegExprDFANode final
{
    enum class NodeType
    {
        Null,
        Epsilon,
        And,
        Or,
        Star,
        Plus,
        Question,
        Char
    };

    NodeType                     Type           = NodeType::Null;
    unsigned                     ID             = 0u;
    unsigned                     Position       = 0u;
    std::vector<unsigned>        Chars;
    unsigned                     AcceptingState = 0u;

    bool                         Nullable       = false;
    std::vector<unsigned>        FirstPos;
    std::vector<unsigned>        LastPos;

    RegExprDFANode*              pParent        = nullptr;
    std::vector<RegExprDFANode*> Children;

    // Default constructor
    RegExprDFANode() = default;

    // Copy constructor - calls the make copy function
    RegExprDFANode(const RegExprDFANode& node) {
        MakeCopy(node);
    }

    explicit RegExprDFANode(NodeType type)
        : Type{type} {
    }

    RegExprDFANode(NodeType type, unsigned pos, unsigned data)
        : Type{type},
          Position{pos} {
        Chars.push_back(data);
    }

    RegExprDFANode(NodeType type, unsigned pos, const std::vector<unsigned>& data)
        : Type{type},
          Position{pos},
          Chars{data} {
    }

    // Creates a copy of the node using a depth-first traversal
    void MakeCopy(const RegExprDFANode& node) {
        Type           = node.Type;
        Chars          = node.Chars;
        Position       = node.Position;
        pParent        = nullptr;
        AcceptingState = node.AcceptingState;

        for (const auto child : node.Children)
            Add(new RegExprDFANode{*child});
    }

    // Adds a child to the node
    void Add(RegExprDFANode* pchild) {
        Children.push_back(pchild);
        pchild->pParent = this;
        pchild->Setup();
    }

    // Adds a child list to the node
    void Add(const std::vector<RegExprDFANode*>& children) {
        for (const auto child : children)
            Add(child);
    }

    // Setups and initializes the DFA node
    void Setup() {
        switch (Type) {
            case NodeType::Epsilon:
                Nullable = true;
                break;

            case NodeType::Char:
                Nullable = false;
                FirstPos.push_back(Position);
                LastPos = FirstPos;
                break;

            case NodeType::Or:
                Nullable = Children[0u]->Nullable && Children[1u]->Nullable;
                FirstPos.resize(Children[0u]->FirstPos.size() + Children[1u]->FirstPos.size());
                std::merge(Children[0u]->FirstPos.begin(), Children[0u]->FirstPos.end(),
                           Children[1u]->FirstPos.begin(), Children[1u]->FirstPos.end(),
                           FirstPos.begin());
                break;

            case NodeType::And:
                // Nullable
                Nullable = Children[0u]->Nullable && Children[1u]->Nullable;
                // FirstPos
                if (Children[0u]->Nullable) {
                    FirstPos.resize(Children[0u]->FirstPos.size() + Children[1u]->FirstPos.size());
                    std::merge(Children[0u]->FirstPos.begin(), Children[0u]->FirstPos.end(),
                               Children[1u]->FirstPos.begin(), Children[1u]->FirstPos.end(),
                               FirstPos.begin());
                } else
                    FirstPos = Children[0u]->FirstPos;
                // LastPos
                if (Children[1u]->Nullable) {
                    LastPos.resize(Children[0u]->FirstPos.size() + Children[1u]->FirstPos.size());
                    std::merge(Children[0u]->FirstPos.begin(), Children[0u]->FirstPos.end(),
                               Children[1u]->FirstPos.begin(), Children[1u]->FirstPos.end(),
                               LastPos.begin());
                } else
                    LastPos = Children[1u]->FirstPos;
                break;

            case NodeType::Star:
                Nullable = true;
                FirstPos = Children[0u]->FirstPos;
                LastPos  = FirstPos;
                break;

            case NodeType::Question:
                Nullable = true;
                FirstPos = Children[0u]->FirstPos;
                break;

            case NodeType::Plus:
                Nullable = false;
                FirstPos = Children[0u]->FirstPos;
                break;

            default:
                break;
        }
    }
};

#ifdef SG_BUILD_DEBUG

// Compute the follow position for a given node
void FollowPos(const RegExprDFANode& node, std::vector<unsigned>& nodelist) {
    bool done  = false;
    auto pnode = node.pParent;

    while (pnode && !done) {
        if (pnode->LastPos == node.LastPos) {
            pnode = pnode->pParent;
            continue;
        }

        switch (pnode->Type) {
            case RegExprDFANode::NodeType::And:
                if (std::find(pnode->Children[0u]->LastPos.begin(),
                              pnode->Children[0u]->LastPos.end(), node.Position) !=
                    pnode->Children[0u]->LastPos.end())
                    nodelist.insert(nodelist.end(), pnode->Children[1u]->FirstPos.begin(),
                                    pnode->Children[1u]->FirstPos.end());
                else
                    done = true;
                break;

            case RegExprDFANode::NodeType::Star:
                if (std::find(pnode->LastPos.begin(), pnode->LastPos.end(), node.Position) !=
                    pnode->LastPos.end())
                    nodelist.insert(nodelist.end(), pnode->FirstPos.begin(),
                                    pnode->FirstPos.end());
                else
                    done = true;
                break;

            default:
                break;
        }

        pnode = pnode->pParent;
    }
}


void PrintTree(const RegExprDFANode& node, String& str) {
    for (const auto child : node.Children)
        PrintTree(*child, str);

    if (node.Type == RegExprDFANode::NodeType::Char) {
        for (const auto ch : node.Chars)
            str += CharT(ch);
        str += ":";
        str += CharT('0' + node.Position);
        str += " ";
    }
}

#endif // SG_BUILD_DEBUG


// Constructs a DFA from a regular expression (using regular expression parser)
struct RegExprDFAParseElement final : public ParseStackElement<RegExprParseToken>
{
    enum class RegExprDataType
    {
        Null,
        Char,
        Chars,
        Node
    };

    RegExprDataType            DataType = RegExprDataType::Null;

    union
    {
        unsigned               ch       = 0u;
        std::vector<unsigned>* pChars;
        RegExprDFANode*        pNode;
    };

    using ParseStackElement::SetErrorData;
    using ParseStackElement::Cleanup;

    void ShiftToken(TokenType& token, TokenStream<TokenType>& stream) {
        ch = token.ch;
        // If we are shifting token, parser will be looking at the next character next
        // So we can do this
        // Essentially, we are ignoring the next token code, and turning it into a character
        if (ch == '\\' && token.Code != TokenCode::TokenEOF) {
            TokenType tempToken;
            ch = stream.GetNextToken(tempToken).ch;
            // Process characters with special meanings
            switch (ch) {
                case 'n': ch='\n'; break;
                case 't': ch='\t'; break;
                case 'v': ch='\v'; break;
                case 'r': ch='\r'; break;
                case 'f': ch='\f'; break;
                case 'b': ch='\b'; break;
                default:           break;
            }
        }
    }

    void Destroy() {
        switch (DataType) {
            case RegExprDataType::Chars: delete pChars; break;
            case RegExprDataType::Node:  delete pNode;  break;
            default:                                    break;
        }
        DataType = RegExprDataType::Null;
    }
};


// *** Regular expression parse handler

class RegExprDFAParseHandler final : public ParseHandler<RegExprDFAParseElement>
{
public:
    unsigned                                          LexemeID          = 0u;
    unsigned                                          PositionCount     = 0u;
    Lex*                                              pLex              = nullptr;
    std::map<String, DFASyntaxTree<RegExprDFANode>*>* pMacroSyntaxTrees = nullptr;
    std::vector<RegExprDFANode*>                      Nodes;

    bool Reduce(Parse<RegExprDFAParseElement>& parse, unsigned productionID) override {
        switch (productionID) {
            // RegExp --> RegExp  A
            case RE_RegExpConcat: {
                const auto pnewNode = new RegExprDFANode{RegExprDFANode::NodeType::And};
                pnewNode->Add(parse[0].pNode);
                pnewNode->Add(parse[1].pNode);

                parse[0].pNode    = pnewNode;
                parse[0].DataType = RegExprDFAParseElement::RegExprDataType::Node;
                break;
            }

                // RegExp --> A
            case RE_RegExp:
                break;

                // A --> A '|' B
            case RE_AOr: {
                const auto pnewNode = new RegExprDFANode{RegExprDFANode::NodeType::Or};
                pnewNode->Add(parse[0].pNode);
                pnewNode->Add(parse[2].pNode);

                parse[0].pNode    = pnewNode;
                parse[0].DataType = RegExprDFAParseElement::RegExprDataType::Node;
                break;
            }
                // A --> B
            case RE_A:
                break;

                // B --> B '*'
            case RE_BStar: {
                const auto pnewNode = new RegExprDFANode{RegExprDFANode::NodeType::Star};
                pnewNode->Add(parse[0].pNode);

                parse[0].pNode    = pnewNode;
                parse[0].DataType = RegExprDFAParseElement::RegExprDataType::Node;
                break;
            }
                // B --> B '+'
            case RE_BPlus: {
                const auto pnewNode = new RegExprDFANode{RegExprDFANode::NodeType::Plus};
                pnewNode->Add(parse[0].pNode);

                parse[0].pNode    = pnewNode;
                parse[0].DataType = RegExprDFAParseElement::RegExprDataType::Node;
                break;
            }

                // B --> B '?'
            case RE_BQuestion: {
                const auto pnewNode = new RegExprDFANode{RegExprDFANode::NodeType::Question};
                pnewNode->Add(parse[0].pNode);

                parse[0].pNode    = pnewNode;
                parse[0].DataType = RegExprDFAParseElement::RegExprDataType::Node;
                break;
            }

                // B --> C
            case RE_B:
                break;

                // C --> '(' RegExp ')'
            case RE_CParen:
                parse[0].pNode    = parse[1].pNode;
                parse[0].DataType = parse[1].DataType;
                break;

                // C --> character
            case RE_CChar:
                parse[0].pNode    = new RegExprDFANode{RegExprDFANode::NodeType::Char,
                                                       PositionCount++, parse[0].ch};
                parse[0].DataType = RegExprDFAParseElement::RegExprDataType::Node;

                Nodes.push_back(parse[0].pNode);
                break;

                // C --> '.'
            case RE_CDot: {
                // Construct a list of all characters but '\n'
                std::vector<unsigned> charList;
                charList.reserve(253u);
                for (unsigned k = 1u; k <= 255u; ++k)
                    if (k != '\n' && k != '\r')
                        charList.push_back(k);

                parse[0].pNode    = new RegExprDFANode{RegExprDFANode::NodeType::Char,
                                                       PositionCount++, charList};
                parse[0].DataType = RegExprDFAParseElement::RegExprDataType::Node;

                Nodes.push_back(parse[0].pNode);
                break;
            }

                // C --> '[' GroupSet ']'
            case RE_CGroupSet: {
                const auto pgroupSet = parse[1].pChars;
                std::vector<unsigned> charList;

                // Construct a list of all the characters represented by the groupSet...
                for (size_t i = 0u; i < pgroupSet->size(); i += 2u)
                    for (unsigned j = (*pgroupSet)[i]; j <= (*pgroupSet)[i + 1u]; ++j)
                        charList.push_back(j);

                // Delete std::vector
                parse[1].Destroy();

                parse[0].pNode    = new RegExprDFANode{RegExprDFANode::NodeType::Char,
                                                       PositionCount++, charList};
                parse[0].DataType = RegExprDFAParseElement::RegExprDataType::Node;

                Nodes.push_back(parse[0].pNode);
                break;
            }

                // C --> '[' '^' GroupSet ']'
            case RE_CNotGroupSet: {
                // Make a NFA out of the sequence
                const auto pgroupSet = parse[2].pChars;
                std::vector<unsigned> charList;

                // Construct a list of all the characters NOT represented by the groupSet...
                // NOTE: Depends on 0..255 ASCII
                for (unsigned k = 1u; k <= 255u; ++k) {
                    bool f = false;
                    for (size_t i = 0u; i < pgroupSet->size(); i += 2u)
                        for (unsigned j = (*pgroupSet)[i]; j <= (*pgroupSet)[i + 1u]; ++j)
                            if (j == k)
                                f = true;

                    if (!f)
                        charList.push_back(k);
                }
                // Delete std::vector
                parse[2].Destroy();

                parse[0].pNode    = new RegExprDFANode{RegExprDFANode::NodeType::Char,
                                                       PositionCount++, charList};
                parse[0].DataType = RegExprDFAParseElement::RegExprDataType::Node;

                Nodes.push_back(parse[0].pNode);
                break;
            }

                // C --> '{' CharSet '}'
            case RE_CCharSet: {
                // Generate a string from the char std::vector
                SG_ASSERT(parse[1].pChars);
                String macroName;
                macroName.reserve(parse[1].pChars->size());
                for (auto const& ord : *parse[1].pChars) {
                    macroName.push_back(char(ord));
                }

                // Delete the character std::vector
                parse[1].Destroy();

                // Try to find it in macro list
                SG_ASSERT(pMacroSyntaxTrees != nullptr);
                auto const& map = *pMacroSyntaxTrees;
                auto const& iter = map.find(macroName);
                if (iter == map.end()) {
                    std::string names;
                    for (auto const& [name, _] : map)
                        names += " '" + name + '\'';
                    pLex->CheckForErrorAndReport("Macro '%s' not defined; there are %zu known macros:%s",
                                                 macroName.data(), map.size(), names.data());
                    // Bad!
                    return false;
                }

                // Invariant: macroName should also be in pLex->Macros:
                SG_ASSERT(pLex != nullptr);
                SG_ASSERT(pLex->Macros.find(macroName) != pLex->Macros.end());

                // Get the macro node from the Macros list
                SG_ASSERT(iter->second);
                auto& macroTree = *iter->second;

                if (!macroTree.pRoot) {
                    pLex->CheckForErrorAndReport("Invalid regular expression in macro '%s' used",
                                                 macroName.data());
                    // Bad!
                    return false;
                }

                // Change all the character positions
                parse[0].pNode    = macroTree.pRoot;
                parse[0].DataType = RegExprDFAParseElement::RegExprDataType::Node;

                // Take care of nodes
                Nodes.insert(Nodes.end(), macroTree.CharNodes.begin(), macroTree.CharNodes.end());

                // clear the temporary tree
                macroTree.pRoot = nullptr;
                break;
            }

            // GroupSet --> character
            case RE_GroupSetChar: {
                // Start off a new std::vector of char pairs
                const auto code   = parse[0].ch;

                parse[0].pChars   = new std::vector<unsigned>;
                parse[0].DataType = RegExprDFAParseElement::RegExprDataType::Chars;
                parse[0].pChars->push_back(code);
                parse[0].pChars->push_back(code);
                break;
            }

                // GroupSet --> character GroupSet
            case RE_GroupSetCharGroupSet:
                // Add the char to the end of the CharSet std::vector
                parse[1].pChars->push_back(parse[0].ch);
                parse[0].pChars   = parse[1].pChars;
                parse[0].DataType = parse[1].DataType;
                break;

                // GroupSet --> character '-' character
            case RE_GroupSetCharList: {
                // Start off a new std::vector of char pairs
                const auto code   = parse[0].ch;
                const auto code2  = parse[2].ch;

                parse[0].pChars   = new std::vector<unsigned>;
                parse[0].DataType = RegExprDFAParseElement::RegExprDataType::Chars;
                parse[0].pChars->push_back(code);
                parse[0].pChars->push_back(code2);
                break;
            }

                // GroupSet --> character '-' character GroupSet
            case RE_GroupSetCharListGroupSet: {
                // Add the char to the end of the CharSet std::vector
                const auto code   = parse[0].ch;
                const auto code2  = parse[2].ch;

                parse[3].pChars->push_back(code);
                parse[3].pChars->push_back(code2);
                parse[0].pChars   = parse[3].pChars;
                parse[0].DataType = parse[3].DataType;
                break;
            }

                // CharSet --> character
            case RE_CharSet: {
                // Get the code and add it to the new std::vector
                const auto code   = parse[0].ch;
                // Create a new std::vector of chars
                parse[0].pChars   = new std::vector<unsigned>;
                parse[0].DataType = RegExprDFAParseElement::RegExprDataType::Chars;
                parse[0].pChars->push_back(code);
                break;
            }

                // CharSet --> CharSet character
            case RE_CharSetChar:
                // Add the char to the end of the CharSet std::vector
                parse[0].pChars->push_back(parse[1].ch);
                break;

            default:
                parse[0].DataType = RegExprDFAParseElement::RegExprDataType::Null;
                break;
        }

        return true;
    }
};


// *** Lexeme info

// Clear all lex data
void Lex::Clear() noexcept {
    Macros.clear();
    MacroNames.clear();
    Lexemes.clear();
    Expressions.clear();
    ExpressionNames.clear();
    TokenLexemes.clear();
    LexemeNameToToken.clear();
    LexemeAliasToToken.clear();
}


// *** Utility Functions

// Creates a hard-coded regular expression grammar
void Lex::CreateRegExpGrammar(Grammar& grammar) const {
    // Define a grammar. Notice the first production is the "augment"
    // production. You've always got to have it since it tells the
    // parser the start symbol and creates an augmented grammar

    // RegExp'      -> RegExp
    // RegExp       -> RegExp A (concatenation operator is implicit)
    // RegExp       -> A

    // A            -> A '|' B
    // A            -> B

    // B            -> B '*'
    // B            -> B '+'
    // B            -> B '?'
    // B            -> C

    // C            -> '(' RegExp ')'
    // C            -> character
    // C            -> '.'
    // C            -> '[' GroupSet ']'
    // C            -> '[' '^' GroupSet ']'
    // C            -> '{' CharSet '}'

    // GroupSet     -> character '-' character
    // GroupSet     -> character
    // GroupSet     -> character '-' character GroupSet
    // GroupSet     -> character GroupSet

    // CharSet      -> character
    // CharSet      -> CharSet character

    // The terminals are . | * + $ - ^ { } ( ) [ ] character

    std::map<String, unsigned> grammarSymbols =
    {
        // Nonterminals (9 total)
        {"RegExp'",   1u},
        {"RegExp",    2u},
        {"A",         3u},
        {"B",         4u},
        {"C",         5u},
        {"D",         6u},
        {"E",         7u},
        {"CharSet",   8u},
        {"GroupSet",  9u},

        // Terminals (14 total)
        {"character", ('c' + TokenCode::TokenFirstID) | ProductionMask::Terminal},
        {"+",         ('+' + TokenCode::TokenFirstID) | ProductionMask::Terminal},
        {"?",         ('?' + TokenCode::TokenFirstID) | ProductionMask::Terminal},
        {".",         ('.' + TokenCode::TokenFirstID) | ProductionMask::Terminal},
        {"|",         ('|' + TokenCode::TokenFirstID) | ProductionMask::Terminal},
        {"*",         ('*' + TokenCode::TokenFirstID) | ProductionMask::Terminal},
        {"(",         ('(' + TokenCode::TokenFirstID) | ProductionMask::Terminal},
        {")",         (')' + TokenCode::TokenFirstID) | ProductionMask::Terminal},
        {"[",         ('[' + TokenCode::TokenFirstID) | ProductionMask::Terminal},
        {"]",         (']' + TokenCode::TokenFirstID) | ProductionMask::Terminal},
        {"{",         ('{' + TokenCode::TokenFirstID) | ProductionMask::Terminal},
        {"}",         ('}' + TokenCode::TokenFirstID) | ProductionMask::Terminal},
        {"-",         ('-' + TokenCode::TokenFirstID) | ProductionMask::Terminal},
        {"^",         ('^' + TokenCode::TokenFirstID) | ProductionMask::Terminal},
        {"$",         ('$' + TokenCode::TokenFirstID) | ProductionMask::Terminal}
    };

    // Productions structure
    static constexpr size_t productionCount = 20u;
    static constexpr struct
    {
        const char* pName;
        const char* pSymbol;
        unsigned    Length;
        const char* pRight[4u];
    }
    init[productionCount] =
    {
        {"RegExpConcat",             "RegExp",   2u, {"RegExp", "A"}                            }, // 0
        {"RegExp",                   "RegExp",   1u, {"A"}                                      }, // 1
        {"AOr",                      "A",        3u, {"A", "|", "B"}                            }, // 2
        {"A",                        "A",        1u, {"B"}                                      }, // 3
        {"BStar",                    "B",        2u, {"B", "*"}                                 }, // 4
        {"BPlus",                    "B",        2u, {"B", "+"}                                 }, // 5
        {"BQuestion",                "B",        2u, {"B", "?"}                                 }, // 6
        {"B",                        "B",        1u, {"C"}                                      }, // 7
        {"CParen",                   "C",        3u, {"(", "RegExp", ")"}                       }, // 8
        {"CChar",                    "C",        1u, {"character"}                              }, // 9
        {"CDot",                     "C",        1u, {"."}                                      }, // 10
        {"CGroupSet",                "C",        3u, {"[", "GroupSet", "]"}                     }, // 11
        {"CNotGroupSet",             "C",        4u, {"[", "^", "GroupSet", "]"}                }, // 12
        {"CCharSet",                 "C",        3u, {"{", "CharSet", "}"}                      }, // 13
        {"GroupSetChar",             "GroupSet", 1u, {"character"}                              }, // 14
        {"GroupSetCharGroupSet",     "GroupSet", 2u, {"character", "GroupSet"}                  }, // 15
        {"GroupSetCharList",         "GroupSet", 3u, {"character", "-", "character"}            }, // 16
        {"GroupSetCharListGroupSet", "GroupSet", 4u, {"character", "-", "character", "GroupSet"}}, // 17
        {"CharSet",                  "CharSet",  1u, {"character"}                              }, // 18
        {"CharSetChar",              "CharSet",  2u, {"CharSet", "character"}                   }  // 19
    };

    std::vector<Production> productions;

    // Create productions
    for (size_t i = 0u; i < productionCount; ++i) {
        unsigned arr[4];
        // Convert names into integers
        for (size_t j = 0u; j < 4u; ++j)
            if (init[i].pRight[j])
                arr[j] = grammarSymbols[init[i].pRight[j]];
        // Add production
        Production p{init[i].pName, grammarSymbols[init[i].pSymbol], arr, init[i].Length};
        productions.push_back(p);
    }

    // And initialize grammar
    grammar.Clear();
    grammar.AddGrammarSymbols(grammarSymbols);
    grammar.AddProductions(productions);
}


// Converts action parameters into numbers
bool Lex::ConvertActionParam() {
    bool error = false;

    // Go through lexemes and convert action parameters into numbers from string
    for (auto& lexeme : Lexemes)
        if (!lexeme.ActionParam.empty()) {
            if (ExpressionNames.find(lexeme.ActionParam) != ExpressionNames.end())
                lexeme.Info.Action = (lexeme.Info.Action & LexemeInfo::ActionMask) |
                                     ExpressionNames[lexeme.ActionParam];
            else {
                // ERROR: Expression Lexemes[i].ActionParam not defined
                // in action parameter for lexeme Lexemes[i].Name
                CheckForErrorAndReport("Expression %s not defined", lexeme.ActionParam.data());
                error = true;
            }
        }

    return !error;
}


bool Lex::MakeDFA(DFAGen& dfa, DFAConstructType type) {
    switch (type) {
        case DFAConstructType::NFA:        return MakeDFAUsingNFA(dfa);
        case DFAConstructType::SyntaxTree: return MakeDFAUsingSyntaxTree(dfa);
    }
    return false;
}


// *** Make a DFA

// Construct a DFA out of the lexemes
bool Lex::MakeDFAUsingNFA(DFAGen& dfa) {
    // Store the macro NFAs
    std::map<String, NFA*> macroNFAs;

    // Go through all macros (in declaration-order) and create NFA's for them.
    // ("In declaration-order" so a lexically-later macro can refer to a lexically-earlier macro.)
    SG_ASSERT(MacroNames.size() == Macros.size());
    for (const String& name : MacroNames) {
        auto const iter = Macros.find(name);
        SG_ASSERT(iter != Macros.end());
        auto const& regExp = iter->second;

        // Create a new nfa to be filled
        const auto pnfa = new NFA;

        // if the nfa failed to initialize then quite
        if (MakeNFA(*pnfa, regExp, macroNFAs))
            macroNFAs[name] = pnfa;
        else
            delete pnfa; // Should we report an error?
    }

    // If there are lexemes, but not a single expression
    // make an expression for them (special case)
    if (Lexemes.size() && !Expressions.size())
        Expressions.push_back({0u, unsigned(Lexemes.size())});

    // Go through all expression lexemes and construct a list of DFA
    std::vector<NFA*> lexemeNFAList;
    DFAGen            dfa2;

    for (size_t i = 0u; i < Expressions.size(); ++i) {
        // Store NFAs for this expression
        const auto EndLexeme = Expressions[i].StartLexeme + Expressions[i].LexemeCount;
        for (unsigned j = Expressions[i].StartLexeme; j < EndLexeme; ++j) {
            const auto pnfa = new NFA{j + TokenCode::TokenFirstID};

            // if the nfa failed to initialize then quit
            if (!MakeNFA(*pnfa, Lexemes[j].RegularExpression, macroNFAs)) {
                // ERROR: Making an NFA from RE failed, probably syntax error in RE
                CheckForErrorAndReport("Failed to make an NFA from the '%s' expression.",
                                       Lexemes[j].RegularExpression.data());

                // This seems to be necessary here in case of error
                delete pnfa;
                for (const auto nfa : lexemeNFAList)
                    delete nfa;

                return false;
            }

            lexemeNFAList.push_back(pnfa);
        }

        // Combine all the individual NFAs into one
        NFA combinedNFA{1u};
        combinedNFA.CombineNFAs(lexemeNFAList);

        for (const auto nfa: lexemeNFAList)
            delete nfa;
        lexemeNFAList.clear();

        // Make a DFA from the huge NFA
        // Should really have a more complex character-counting scheme!
        if (i == 0u)
            dfa.Create(combinedNFA, Lexemes, 256u);
        else {
            // And combine it to form an expression
            dfa2.Create(combinedNFA, Lexemes, unsigned(dfa.GetCharCount() - 1u));
            if (!dfa.Combine(dfa2)) {
                // ERROR: Failed to combine DFAs, probably inconsistent states
                CheckForErrorAndReport("Failed to combine DFAs.");
                return false;
            }
        }
    }

    // Free all the macro NFA's
    for (const auto& [_, nfa] : macroNFAs)
        delete nfa;

    return true;
}


// Lexemes is a complete list of all the R.E.'s in the form name, R.E.
// LexemeNFAList is a shorter list containing the NFAs generated so far
bool Lex::MakeNFA(NFA& nfa, const String& regExp, std::map<String, NFA*>& macroNFAs) {
    if (!NFAParseTable.IsValid()) {
        // Create the hard coded Regular Expression grammar
        Grammar grammar;
        CreateRegExpGrammar(grammar);

        // And make parse table out of it
        if (!NFAParseTable.Create(grammar))
            return false;  // Should probably throw exception
    }

    // Make an input stream with the regular expression string we've got
    MemBufferInputStream inputStream{regExp};

    // Create a tokenizer to get input. Since we're giving it a nullptr lexeme list,
    // it will just return each character exactly as it appears in the input string
    RegExpTokenizer<RegExprParseToken> tokenizer{&inputStream};

    // Initialize Parsing
    Parse<RegExprNFAParseElement> parse{&NFAParseTable, &tokenizer};
    RegExprNFAParseHandler        parseHandler;

    // Set the custom parse handler custom data
    parseHandler.LexemeID   = nfa.GetLexemeID();
    parseHandler.pLex       = this;
    parseHandler.pMacroNFAs = &macroNFAs;

    // Let the parse class parse the regular expression using
    // using the custom parseHandler reduce callback
    if (!parse.DoParse(parseHandler)) {
        const ParseMessage msg{ParseMessage::ErrorMessage, "",
                               "NFA: Regular expression parsing error"};
        Messages.AddMessage(msg);
        return false;
    }

    // Move result NFA data over to us, and destroy result NFA
    nfa.MoveData(*parse[0].pNFA);
    parse[0].Destroy();

    return true;
}


template <class T>
bool Lex::MakeDFA(DFAGen& dfa, DFASyntaxTree<T>& tree, std::vector<Lexeme>& lexemes) {
    if (dfa.IsValid())
        return false;

    // Character map for lookup into the transition table
    auto& charTable = dfa.CharTable;
    std::vector<std::vector<unsigned>> followTable;

    // Create the follow table
    followTable.resize(tree.CharNodes.size());
    for (size_t k = 0u; k < tree.CharNodes.size(); ++k) {
        const auto charNode = tree.CharNodes[k];
        FollowPos(*charNode, followTable[k]);

        for (const auto ch: charNode->Chars)
            if (ch != 1u)  // Magic number detected
                if (!charTable.HasValue(ch))
                    charTable.SetValue(ch, unsigned(dfa.GetCharCount()));
    }

    auto& transitionTable = dfa.TransitionTable;
    std::vector<std::vector<unsigned>> dfaStates;

    dfaStates.push_back(tree.pRoot->FirstPos);

    // And allocate the transition table for the first state
    transitionTable.resize(1u);
    transitionTable[0u].resize(dfa.GetCharCount(), DFA::EmptyTransition);

    for (unsigned state = 0u; state < unsigned(dfaStates.size()); ++state) {
        // Calculate Move on all characters
        std::vector<unsigned>                     nextState;
        std::map<unsigned, std::vector<unsigned>> charIndexs;

        for (const auto dsi: dfaStates[state])
            for (const auto ch: tree.CharNodes[dsi]->Chars)
                if (ch != 1u) { // Magic number detected
                    auto&       ci = charIndexs[ch];
                    const auto& ft = followTable[dsi];
                    ci.insert(ci.end(), ft.begin(), ft.end());
                }

        // Go through all characters
        for (const auto& [state, stateVec] : charIndexs) {
            nextState = stateVec;

            // Sort the indexes and make sure there are no duplicates
            // Maybe faster to use set_union
            std::sort(nextState.begin(), nextState.end());
            const auto lastIt = std::unique(nextState.begin(), nextState.end());
            nextState.erase(lastIt, nextState.end());

            // If U is not a state in dfaStates, add it
            // Linear search is ok, because states are sorted
            size_t j = 0u;
            for (; j < dfaStates.size(); ++j)
                if (dfaStates[j] == nextState)
                    break;

            // if not found
            if (j == dfaStates.size()) {
                // Add a new state
                dfaStates.push_back(nextState);
                // And allocate transition table for the state
                const auto size = transitionTable.size();
                transitionTable.resize(size + 1u);
                transitionTable[size].resize(dfa.GetCharCount(), DFA::EmptyTransition);
            }

            dfa.SetTransitionState(state, state, unsigned(j));
        }
    }

    // Make an array that tells us which states are accepting states
    // Zero means non-accepting, nonzero means accepting. The particular
    // nonzero value is the lowest lexeme ID number associated with the nodes
    auto&      acceptState = dfa.AcceptStates;
    const auto stateCount  = transitionTable.size();

    acceptState.resize(stateCount, 0u);

    for (size_t i = 0u; i < stateCount; ++i) {
        auto& accState = acceptState[i];
        // Does this state contain an accepting node?
        for (size_t j = 0u; j < dfaStates[i].size(); ++j) {
            const auto astate = tree.CharNodes[dfaStates[i][j]]->AcceptingState;
            if (astate) {
                // Set pAcceptState[i] equal to the lexeme ID if either:
                //   1. It's the only non-zero lexeme ID we've seen
                //   2. Or it's appears the latest in file - so it has the lowest lexeme ID
                if (accState == 0u)
                    accState = astate;
                else if (accState < astate) {
                    // NOTE: Lexeme %s takes precedence over %s on state %d
                    CheckForErrorAndReport("Lexeme '%s' takes precedence over '%s' on state %zu",
                                           lexemes[astate].Name.data(),
                                           lexemes[accState].Name.data(), i);
                    accState = astate;
                } else { // accState > astate
                    // NOTE: Lexeme %s takes precedence over %s on state %d (other way around)
                    CheckForErrorAndReport("Lexeme '%s' takes precedence over '%s' on state %zu",
                                           lexemes[accState].Name.data(),
                                           lexemes[astate].Name.data(), i);
                }
            }
        }
    }

    // Create a lexeme information structure
    dfa.LexemeInfos.resize(lexemes.size() + 2u);

    // First 2 elements are default tokens
    dfa.LexemeInfos[0u].TokenCode = TokenCode::TokenError;
    dfa.LexemeInfos[0u].Action    = LexemeInfo::ActionNone;
    dfa.LexemeInfos[1u].TokenCode = TokenCode::TokenEOF;
    dfa.LexemeInfos[1u].Action    = LexemeInfo::ActionNone;

    // And copy data to it
    for (size_t i = 0u; i < lexemes.size(); ++i)
        dfa.LexemeInfos[i + 2u] = lexemes[i].Info;

    // Only one expression created
    dfa.ExpressionStartStates.push_back(DFA::StateType{0u});

    return true;
}


bool Lex::MakeDFAUsingSyntaxTree(DFAGen& dfa) {
    if (dfa.IsValid())
        return false;

    // Go through all macros (in declaration-order) and create DFASyntaxTree's for them.
    // ("In declaration-order" so a lexically-later macro can refer to a lexically-earlier macro.)
    std::map<String, DFASyntaxTree<RegExprDFANode>*> macroSyntaxTrees;
    SG_ASSERT(MacroNames.size() == Macros.size());
    for (const String& name : MacroNames) {
        auto const iter = Macros.find(name);
        SG_ASSERT(iter != Macros.end());
        auto const& regExp = iter->second;

        // Create a new syntax tree
        const auto ptree = new DFASyntaxTree<RegExprDFANode>;

        // if the nfa failed to initialize then quite
        if (MakeSyntaxTree(*ptree, regExp, macroSyntaxTrees))
            macroSyntaxTrees[name] = ptree;
        else
            delete ptree; // Should we report an error?
    }

    // If there are lexemes, but not a single expression
    // make an expression for them (special case)
    if (Lexemes.size() && !Expressions.size())
        Expressions.push_back({0u, unsigned(Lexemes.size())});

    DFASyntaxTree<RegExprDFANode> syntaxTree;
    const String expr = "(a|b)*(abb)+c";

    MakeSyntaxTree(syntaxTree, expr, macroSyntaxTrees);

    const DFAGen testdfa;

    [[maybe_unused]] unsigned r;
    r = testdfa.TestString("aab");
    r = testdfa.TestString("abbbababb");
    r = testdfa.TestString("abb");

    return true;
}


template <class T>
bool Lex::MakeSyntaxTree(DFASyntaxTree<T>& tree, const String& regExpr,
                         std::map<String, DFASyntaxTree<T>*>& macroSyntaxTrees) {
    // Cache the parse table
    static ParseTableGen parseTable;

    if (!parseTable.IsValid()) {
        Grammar grammar;
        // Create the hard coded Regular Expression grammar
        CreateRegExpGrammar(grammar);

        // And make parse table out of it
        if (!parseTable.Create(grammar))
            return false; // Should probably throw exception
    }

    // Make an input stream with the regular expression string we've got
    MemBufferInputStream inputStream{regExpr};

    // Create a tokenizer to get input. Since we're giving it a nullptr lexeme list,
    // it will just return each character exactly as it appears in the input string
    RegExpTokenizer<RegExprParseToken> tokenizer{&inputStream};

    // Initialize Parsing
    Parse<RegExprDFAParseElement> parse{&parseTable, &tokenizer};
    RegExprDFAParseHandler        parseHandler;

    // Set the custom parse handler custom data
    parseHandler.LexemeID          = 0u;
    parseHandler.PositionCount     = 0u;
    parseHandler.pLex              = this;
    parseHandler.pMacroSyntaxTrees = &macroSyntaxTrees;

    // Let the parse class parse the regular expression using
    // using the custom parseHandler reduce callback
    if (!parse.DoParse(parseHandler)) {
        const ParseMessage msg{ParseMessage::ErrorMessage, "",
                               "DFA Syntax Tree: Regular expression parsing error"};
        Messages.AddMessage(msg);
        return false;
    }

    // Append the final accepting node
    // Make the fake accept character node
    const auto pnodeAccept      = new RegExprDFANode{RegExprDFANode::NodeType::Char,
                                                     parseHandler.PositionCount++, 1u};
    pnodeAccept->AcceptingState = 1u;
    // Make the root node
    const auto pnodeRoot        = new RegExprDFANode{RegExprDFANode::NodeType::And};
    pnodeRoot->pParent          = nullptr;
    pnodeRoot->Add(parse[0].pNode);
    pnodeRoot->Add(pnodeAccept);
    pnodeRoot->Setup();

    // Add the accept node to the nodes list
    parseHandler.Nodes.push_back(pnodeAccept);

    // Set the destination tree data
    tree.pRoot     = pnodeRoot;
    tree.CharNodes = parseHandler.Nodes;

    return true;
}


void Lex::CheckForErrorAndReport(const char* message...) {
    if (Messages.GetMessageFlags() & ParseMessageBuffer::MessageError) {
        va_list argList;
        va_start(argList, message);
        const auto messageStr = StringWithVAFormat(message, argList);
        va_end(argList);

        const ParseMessage msg{ParseMessage::ErrorMessage, "", messageStr};
        Messages.AddMessage(msg);
    }
}

} // namespace Generator
} // namespace SGParser
