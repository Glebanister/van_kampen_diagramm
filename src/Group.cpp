#include "Group.hpp"
#include "Graph.hpp"

namespace van_kampen
{

Transition::Transition(nodeId_t nodeTo, const GroupElement &elem)
    : to(nodeTo), label(elem) {}

bool GroupElement::operator==(const GroupElement &other) const
{
    return name == other.name && reversed == other.reversed;
}

bool GroupElement::isOpposite(const GroupElement &other)
{
    return name == other.name && reversed != other.reversed;
}

GroupElement GroupElement::inversed() const
{
    return {name, !reversed};
}

void GroupElement::inverse() noexcept
{
    reversed ^= true;
}

Diagramm::Diagramm(Graph &graph)
    : graph_(graph) {}

std::vector<Transition> Diagramm::getCircuit()
{
    std::vector<Transition> result{};

    if (Node::isNonexistantNode(terminal_))
    {
        return {};
    }

    nodeId_t firstNodeId = graph_.nodes()[terminal_].getId();
    nodeId_t curNodeId = firstNodeId;
    do
    {
        if (graph_.nodes()[curNodeId].transitions().empty())
        {
            throw std::invalid_argument("diagram is not looped");
        }
        else
        {
            result.push_back(graph_.nodes()[curNodeId].transitions().back());
            curNodeId = result.back().to;
        }
    } while (curNodeId != firstNodeId);

    return result;
}

nodeId_t Diagramm::getTerminal() const noexcept { return terminal_; }
void Diagramm::setTerminal(nodeId_t n) noexcept { terminal_ = n; }

bool Diagramm::bindWord(const std::vector<GroupElement> &word)
{
    class TerminalShuffler
    {
    public:
        TerminalShuffler(Diagramm &dg, Graph &g)
            : diagramm(dg), graph(g) {}

        ~TerminalShuffler()
        {
            if (Node::isNonexistantNode(diagramm.getTerminal()))
            {
                return;
            }
            diagramm.setTerminal(graph.node(diagramm.getTerminal()).transitions().back().to);
        }

    private:
        Diagramm &diagramm;
        Graph &graph;
    } shuffler(*this, graph_);

    std::vector<Transition> circleWord = getCircuit();
    if (circleWord.empty())
    {
        terminal_ = graph_.addNode();
        nodeId_t curNode = terminal_;
        for (std::size_t i = 0; i < word.size() - 1; ++i)
        {
            nodeId_t prevNode = curNode;
            curNode = graph_.node(prevNode).addTransitionToNewNode(word[i]);
            graph_.node(curNode).addTransition(prevNode, word[i].inversed());
        }
        graph_.node(curNode).addTransition(terminal_, word.back());
        graph_.node(terminal_).addTransition(curNode, word.back().inversed());
        graph_.node(terminal_).swapLastAdditions();
        return true;
    }

    auto reversedCircleWord = circleWord;
    for (auto &letter : reversedCircleWord)
    {
        letter.label.inverse();
    }

    std::reverse(reversedCircleWord.begin(), reversedCircleWord.end());

    std::size_t longestEntry = 0;
    std::size_t entryBegin = 0;

    { // knuth morris pratt
        std::vector<std::string> text;
        auto addToken = [&](const GroupElement &g) {
            text.emplace_back(g.reversed ? (g.name + "!") : g.name);
        };
        for (auto &letter : word)
        {
            addToken(letter);
        }
        text.emplace_back("\0");
        for (Transition &letter : reversedCircleWord)
        {
            addToken(letter.label);
        }
        std::size_t n = text.size();
        std::vector<int> pi(n);
        for (std::size_t i = 1; i < n; ++i)
        {
            int j = pi[i - 1];
            for (; j > 0 && text[i] != text[j]; j = pi[j - 1])
                ;
            if (text[i] == text[j])
                ++j;
            pi[i] = j;
        }
        for (std::size_t i = word.size(); i < text.size(); ++i)
        {
            if (pi[i] > static_cast<int>(longestEntry) ||
                ((rand() % 2 == 0) && pi[i] == static_cast<int>(longestEntry)))
            {
                longestEntry = pi[i];
                entryBegin = i - longestEntry - word.size();
            }
        }
    }

    if (longestEntry == 0 || // TODO: some are very strict
        longestEntry == circleWord.size() ||
        entryBegin == 0 ||
        longestEntry == word.size() ||
        entryBegin + longestEntry == circleWord.size())
    {
        return false;
    }

    std::size_t normalWordEntryBegin = circleWord.size() - longestEntry - entryBegin;
    auto branchFrom = circleWord[normalWordEntryBegin - 1].to;
    auto branchTo = circleWord[normalWordEntryBegin + longestEntry - 1].to;

    auto curNode = branchFrom;

    for (std::size_t i = longestEntry; i < word.size() - 1; ++i)
    {
        auto prevNode = curNode;
        curNode = graph_.node(prevNode).addTransitionToNewNode(word[i]);
        graph_.node(curNode).addTransition(prevNode, word[i].inversed());
    }
    graph_.node(curNode).addTransition(branchTo, word.back());
    graph_.node(branchTo).addTransition(curNode, word.back().inversed());
    graph_.node(branchTo).swapLastAdditions();
    return true;
}

bool Diagramm::merge(Diagramm &&)
{
    /*

1. Find longest common substring
2. 

*/
    return false;
}
} // namespace van_kampen
