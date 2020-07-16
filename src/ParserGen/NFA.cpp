// Filename:  NFA.cpp
// Content:   NFA and DFA class implementation source file
// Provided AS IS under MIT License; see LICENSE file in root folder.
//
// Contains a NFA class. Can create a 1-char accepting NFA, then concatenate, or, and kleene star
// NFAs. The NFA class also has a regular expression --> R.E. parser.
// Should be possible to make it yet much faster by using sets of characters for states.

#include "NFA.h"
#include "Tokenizer.h"

using namespace SGParser;
using namespace Generator;

// ***** NonDeterministic Finite Automaton - NFA

// Create an NFA that will accept only one character string containing character c
bool NFA::Create(unsigned c, unsigned lexemeID) {
    // Make sure this NFA is empty
    if (pStartState)
        return false;

    LexemeId             = lexemeID;
    // Make a start node and accepting node...
    pStartState          = NewState(0u);
    const auto ptempNode = NewState(1u);

    // ...with one link between them on c.
    AddLink(pStartState, c, ptempNode);

    return true;
}


// Creates a Multi-character NFA
// Builds an NFA that will accept any one of characters provided (only length 1)
bool NFA::Create(const std::vector<unsigned>& c, unsigned lexemeID) {
    // Make sure this NFA is empty
    if (pStartState)
        return false;

    LexemeId             = lexemeID;
    // Make a start node and accepting node...
    pStartState          = NewState(0u);
    const auto ptempNode = NewState(1u);

    // ...with many links between them on c[i].
    for (const auto ch: c)
        AddLink(pStartState, ch, ptempNode);

    return true;
}


// Create a NFA from another NFA
// Given a NFA n, makes a complete deep copy of it. All the nodes are actually
// duplicated. This routine handles remapping the LinkPtr arrays to the new
// set of addresses by constructing a temporary hash table between the old
// node addresses and the new node addresses
// Assigns a new lexeme Id if its value !=0
bool NFA::Create(const NFA& n, unsigned newLexemeId) {
    // Make sure this NFA is empty
    if (pStartState)
        return false;
    // Can't copy an empty NFA
    if (!n.pStartState)
        return false;

    // Make a list of all nodes in n
    NodeSet graph;

    TraverseGraph(n.pStartState, graph);

    // Make a new node in this graph for all these nodes. Make a hash
    // that maps from the old pointer address to the new pointer address
    std::map<NFANode*, NFANode*> pointerMap;

    for (const auto igraph: graph)
        pointerMap[igraph] = new NFANode;

    // Go through all of the nodes in the old graph and copy them to
    // the new graph. Translate the pointers in the LinkPtr field
    // using the pointerMap created above
    for (const auto n1: graph) {
        const auto n2 = pointerMap[n1];

        n2->AcceptingState = n1->AcceptingState;
        n2->Id             = n1->Id;
        n2->LinkChar       = n1->LinkChar;

        for (const auto ptr: n1->LinkPtr)
            n2->LinkPtr.push_back(pointerMap[ptr]);
    }

    // Set the start node equal to n1's start node
    pStartState = pointerMap[n.pStartState];

    // Copy/translate the final state std::vector
    for (const auto fs: n.FinalState) {
        const auto ptr = pointerMap[fs];

        if (newLexemeId && ptr->AcceptingState)
            ptr->AcceptingState = newLexemeId;

        FinalState.push_back(ptr);
    }

    LexemeId = newLexemeId ? newLexemeId : n.LexemeId;

    return true;
}


void NFA::Destroy() {
    if (!pStartState)
        return;

    NodeSet graph;

    TraverseGraph(pStartState, graph);

    for (const auto igraph: graph)
        delete igraph;

    FinalState.clear();
    pStartState = nullptr;
}


// Moves NFA's data to this NFA
void NFA::MoveData(NFA& source) {
    // Destroy this NFA's data if it exists
    Destroy();

    // Move data
    pStartState = source.pStartState;
    FinalState  = source.FinalState;

    // And empty out source
    source.pStartState = nullptr;
    source.FinalState.clear();
}


// Concatenates two NFAs
// Combines two NFAs to form a new NFA that only accepts strings of the form
// "L(this)L(b)" where L(a) is any string from the language of this NFA and L(b)
// is any string from the language of NFA b
void NFA::Concat(NFA& b) {
    // Same as us
    if (this == &b)
        return;

    if (!pStartState) {
        if (!b.pStartState)
            return;
        pStartState = b.pStartState;
    } else {
        // Connect all of final states to the start state of b
        // Those final states are no longer final states
        for (const auto fs: FinalState) {
            AddLink(fs, NFANode::Epsilon, b.pStartState);
            fs->AcceptingState = 0u;
        }
        FinalState.clear();
    }

    // This seems to be necessary
    const auto pnewStartState = NewState(0u);
    AddLink(pnewStartState, NFANode::Epsilon, pStartState);
    pStartState = pnewStartState;

    // Final state
    const auto pfinalState = NewState(1u);

    // Connect all of b's final states to the new final state
    // Those states are no longer final states
    for (const auto fs: b.FinalState) {
        AddLink(fs, NFANode::Epsilon, pfinalState);
        fs->AcceptingState = 0u;
    }

    // And release b's data
    b.pStartState = nullptr;
    b.FinalState.clear();
}


// Or's two NFAs
// Combines two NFAs to form a new NFA that accepts the language which is
// the union of the languages of NFA a and NFA b
void NFA::Or(NFA& b) {
    // Same as us
    if (this == &b)
        return;
    if (!pStartState && !b.pStartState)
        return;

    const auto pnewStartState = NewState(0u);
    const auto pfinalState    = NewState(0u);

    // Connect the start state to the start states of this and b
    // with epsilon transitions
    if (pStartState) {
        AddLink(pnewStartState, NFANode::Epsilon, pStartState);

        // Connect all the final states of a and b to the new final state
        // Make the final states of a and b no longer final states
        for (const auto fs: FinalState) {
            AddLink(fs, NFANode::Epsilon, pfinalState);
            fs->AcceptingState = 0u;
        }
        FinalState.clear();
    }
    if (b.pStartState) {
        AddLink(pnewStartState, NFANode::Epsilon, b.pStartState);

        for (const auto fs: b.FinalState) {
            AddLink(fs, NFANode::Epsilon, pfinalState);
            fs->AcceptingState = 0u;
        }
        b.FinalState.clear();
    }

    // Set new start state
    pStartState = pnewStartState;
    // Make the state final
    pfinalState->AcceptingState = LexemeId;
    FinalState.push_back(pfinalState);

    // Release b's data
    b.pStartState = nullptr;
}


// Kleene star operator (also + operator for "one or more occurrences")
// include Epsilon = 0 makes it '+' operator, = 1 makes it '*' operator
void NFA::Kleene(KleeneType type) {
    if (!pStartState)
        return;

    // Make a new start and final state
    const auto pnewStartState = NewState(0u);
    const auto pfinalState    = NewState(0u);

    // Connect the start state directly to the final state
    // with an epsilon transition to represent the fact
    // that epsilon is a string in the Kleen closure
    if (type == KleeneType::ConnectEmpty || type == KleeneType::ConnectBoth)
        AddLink(pnewStartState, NFANode::Epsilon, pfinalState);

    // Connect the new start state to the start state
    AddLink(pnewStartState, NFANode::Epsilon, pStartState);

    // Connect all final states of a to the new final
    // state and back to the start state of a
    // These states are no longer final states
    for (const auto fs: FinalState) {
        AddLink(fs, NFANode::Epsilon, pfinalState);
        if (type == KleeneType::ConnectBack || type == KleeneType::ConnectBoth)
            AddLink(fs, NFANode::Epsilon, pStartState);
        fs->AcceptingState = 0u;
    }
    FinalState.clear();

    // Set new start state
    pStartState = pnewStartState;
    // Make the state final
    pfinalState->AcceptingState = LexemeId;
    FinalState.push_back(pfinalState);
}


// CombineNFAs
// Basically just Concat(), except with > 2 inputs
void NFA::CombineNFAs(const std::vector<NFA*>& NFAList) {
    // Create a new start state
    const auto pnewStartState = NewState(0u);

    // If this NFA is valid than treat it as though it was one of the others
    if (pStartState)
        AddLink(pnewStartState, NFANode::Epsilon, pStartState);

    pStartState = pnewStartState;

    // Connect the start state to the start state of each of the NFAs in the list
    // Also add each NFAs final states to our final state list
    for (const auto nfa: NFAList) {
        // Should probably check to see if any of the NFA's are equal to this one

        AddLink(pStartState, NFANode::Epsilon, nfa->pStartState);
        FinalState.insert(FinalState.end(), nfa->FinalState.begin(), nfa->FinalState.end());

        // Clear the NFA
        nfa->pStartState = nullptr;
        nfa->FinalState.clear();
    }
}


// Debugging routine that prints all the nodes in the NFA
void NFA::PrintFA() const {
    NodeSet graph;
    TraverseGraph(pStartState, graph);

    size_t i = 0u;
    for (const auto pn: graph) {
        std::printf("NFANode %zu (%p) [%u]: accepting = %u:", i,
                    (void*)pn, pn->Id, pn->AcceptingState);

        for (size_t j = 0u; j < pn->LinkChar.size(); ++j)
            std::printf("\n%c --> %p", char(pn->LinkChar[j]), (void*)pn->LinkPtr[j]);

        std::printf("\n");
        ++i;
    }
}


// Similar to PrintFA, except formats the output as a file
// suitable for display by the UC Berkeley Dotty Graphviz program
void NFA::PrintFADotty(String& str) const {
    NodeSet graph;

    NumberFA();
    TraverseGraph(pStartState, graph);
    
    String dest = "digraph G {\n";

    for (const auto pn: graph) {
        if (pn->AcceptingState)
            dest += StringWithFormat("n%u [label=\"n%u: a%u\" peripheries=3]\n",
                                     pn->Id, pn->Id, pn->AcceptingState);

        for (size_t j = 0u; j < pn->LinkChar.size(); ++j) {
            String     label;
            const auto x = pn->LinkChar[j];

            if (x >= 32u && x <= 126u)
                label = StringWithFormat("'%c'", char(x));
            else
                label = StringWithFormat("%u", x);

            dest += StringWithFormat("n%u -> n%u [label=\"%s\"]\n",
                                     pn->Id, pn->LinkPtr[j]->Id, label.data());
        }
    }

    dest += "}\n";

    str.swap(dest);
}


// Assigns integer numbers to each node in the NFA
void NFA::NumberFA(void) const {
    NodeSet graph;

    TraverseGraph(pStartState, graph);

    unsigned i = 0u;
    for (const auto igraph: graph)
        igraph->Id = i++;
}


// Constructs a list of all nodes reachable from a given starting node
void NFA::TraverseGraph(NFANode* seed, NodeSet& visited) const {
    // Insert node if it's not already there
    visited.insert(seed);

    for (const auto node: seed->LinkPtr) {
        // If the node is not already inserted
        if (visited.find(node) == visited.end()) {
            // Traverse the rest of the graph (traversing will also insert the node)
            TraverseGraph(node, visited);
        }
    }
}


// Constructs a new node and returns a pointer to it
NFANode* NFA::NewState(unsigned accepting) {
    const auto pnewnode = new NFANode;

    // If it's a final state, add it to the list
    if (!accepting)
        pnewnode->AcceptingState = 0u;
    else {
        pnewnode->AcceptingState = LexemeId;
        FinalState.push_back(pnewnode);
    }

    return pnewnode;
}


// Creates a link between two nodes on a character c
void    NFA::AddLink(NFANode* modify, unsigned c, NFANode* target) {
    modify->LinkChar.push_back(c);
    modify->LinkPtr.push_back(target);
}
