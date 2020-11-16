#include "Group.hpp"
#include "Graph.hpp"

namespace van_kampen
{
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

Diagramm::Diagramm(std::shared_ptr<Graph> graph)
    : graph_(graph) {}

std::vector<Transition> Diagramm::getCircuit()
{
    std::vector<Transition> result{};

    if (Node::isNonexistantNode(terminal_))
    {
        return {};
    }

    nodeId_t firstNodeId = graph_->nodes()[terminal_].getId();
    nodeId_t curNodeId = firstNodeId;
    do
    {
        if (graph_->nodes()[curNodeId].transitions().empty())
        {
            throw std::invalid_argument("diagram is not looped");
        }
        else
        {
            result.push_back(graph_->nodes()[curNodeId].transitions().back());
            curNodeId = result.back().to;
        }
    } while (curNodeId != firstNodeId);

    return result;
}

nodeId_t Diagramm::getTerminal() const noexcept { return terminal_; }
void Diagramm::setTerminal(nodeId_t n) noexcept { terminal_ = n; }

bool Diagramm::bindWord(std::vector<GroupElement> word, bool force)
{
    bool isSquare = word.size() == 4;
    double transitionPriority = 1.0 / static_cast<double>(word.size());
    std::vector<Transition> circleWord = getCircuit();
    if (circleWord.empty())
    {
        terminal_ = graph_->addNode();
        nodeId_t curNode = terminal_;
        for (std::size_t i = 0; i < word.size() - 1; ++i)
        {
            nodeId_t prevNode = curNode;
            curNode = graph_->node(prevNode).addTransitionToNewNode(word[i], isSquare);
            graph_->node(curNode).addTransition(prevNode, word[i].inversed(), isSquare);
            graph_->increaseNondirEdgePriority(prevNode, curNode, transitionPriority);
        }
        graph_->node(curNode).addTransition(terminal_, word.back(), isSquare);
        graph_->node(terminal_).addTransition(curNode, word.back().inversed(), isSquare);
        graph_->node(terminal_).swapLastAdditions();
        graph_->increaseNondirEdgePriority(curNode, terminal_, transitionPriority);
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
    std::size_t bestRotation = 0;

    for (std::size_t rotation = 0; rotation < word.size(); ++rotation)
    {
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
            for (std::size_t i = word.size() + 1; i < text.size(); ++i)
            {
                if (pi[i] > static_cast<int>(longestEntry) && reversedCircleWord[i - word.size() - 1].isInSquare)
                {
                    longestEntry = pi[i];
                    entryBegin = i - longestEntry - word.size();
                    bestRotation = rotation;
                }
            }
            for (std::size_t i = word.size() + 1; i < text.size(); ++i)
            {
                if (pi[i] > static_cast<int>(longestEntry))
                {
                    longestEntry = pi[i];
                    entryBegin = i - longestEntry - word.size();
                    bestRotation = rotation;
                }
            }
        }
        std::rotate(word.begin(), word.begin() + 1, word.end());
    }

    std::rotate(word.begin(), word.begin() + bestRotation, word.end());

    if (longestEntry == 0 || // TODO: some are very strict
        longestEntry == circleWord.size() ||
        entryBegin == 0 ||
        entryBegin + longestEntry == circleWord.size())
    {
        return false;
    }

    if (longestEntry == word.size())
    {
        return true;
    }

    std::size_t normalWordEntryBegin = circleWord.size() - longestEntry - entryBegin;
    nodeId_t branchFrom = circleWord[normalWordEntryBegin - 1].to;
    nodeId_t branchTo = circleWord[normalWordEntryBegin + longestEntry - 1].to;

    bool connectWithSquare = circleWord[normalWordEntryBegin - 1].isInSquare;

    if (longestEntry < 2 && !isSquare && !connectWithSquare && !force)
    {
        return false;
    }

    auto curNode = branchFrom;

    for (std::size_t i = longestEntry; i < word.size() - 1; ++i)
    {
        auto prevNode = curNode;
        curNode = graph_->node(prevNode).addTransitionToNewNode(word[i], isSquare);
        graph_->node(curNode).addTransition(prevNode, word[i].inversed(), isSquare);
        graph_->increaseNondirEdgePriority(curNode, prevNode, transitionPriority);
    }
    graph_->node(curNode).addTransition(branchTo, word.back(), isSquare);
    graph_->node(branchTo).addTransition(curNode, word.back().inversed(), isSquare);
    graph_->node(branchTo).swapLastAdditions();
    graph_->increaseNondirEdgePriority(curNode, branchTo, transitionPriority);

    for (std::size_t i = normalWordEntryBegin - 1; i < normalWordEntryBegin + longestEntry - 1; ++i)
    {
        graph_->increaseNondirEdgePriority(circleWord[i].to, circleWord[i + 1].to, transitionPriority);
    }

    return true;
}

bool Diagramm::merge(Diagramm &&other, std::size_t hint)
{
    if (graph_ != other.graph_)
    {
        throw std::invalid_argument("cannot merge diagrams based on different graphs");
    }
    auto doubleWord = [](std::vector<Transition> &word) {
        std::size_t sz = word.size();
        for (std::size_t i = 0; i < sz; ++i)
        {
            word.push_back(word[i]);
        }
    };
    std::vector<Transition> myCirc = getCircuit(),
                            otherCirc = other.getCircuit();
    std::reverse(otherCirc.begin(), otherCirc.end());
    doubleWord(myCirc);
    doubleWord(otherCirc);

    std::size_t longestMatch = 0,
                myLongestMatchBegin = 0,
                otherLongestMatchBegin = 0;

    bool hasHint = hint != 0;
    { // Find longest common substring
        for (std::size_t leng = 1; leng < myCirc.size(); ++leng)
        {
            if (hasHint)
                leng = hint;
            bool found = false;
            for (std::size_t i = 0; i + leng < myCirc.size(); ++i)
            {
                for (std::size_t j = 0; j + leng < otherCirc.size(); ++j)
                {
                    bool allMatched = true;
                    for (std::size_t k = 0; k < leng; ++k)
                    {
                        if (!myCirc[i + k].label.isOpposite(otherCirc[j + k].label))
                        {
                            allMatched = false;
                            break;
                        }
                    }
                    if (allMatched)
                    {
                        myLongestMatchBegin = i;
                        otherLongestMatchBegin = j;
                        found = true;
                        break;
                    }
                }
                if (found)
                {
                    break;
                }
            }
            if (found)
            {
                longestMatch = leng;
            }
            if (hasHint)
                break;
        }
    }

    if (!longestMatch ||
        longestMatch >= myCirc.size() / 2 ||
        longestMatch >= otherCirc.size() / 2)
    {
        return false;
    }

    nodeId_t myRootNode = myLongestMatchBegin == 0 ? myCirc.back().to : myCirc[myLongestMatchBegin - 1].to;
    nodeId_t otherRootNode = otherCirc[otherLongestMatchBegin].to;

    auto pathNeighbours = [&](std::size_t id) {
        std::unordered_set<nodeId_t> result = {};
        if (id > 0)
        {
            result.insert(otherCirc[otherLongestMatchBegin + id - 1].to);
        }
        if (id < longestMatch)
        {
            result.insert(otherCirc[otherLongestMatchBegin + id + 1].to);
        }
        return result;
    };

    nodeId_t prevMyPath = myRootNode;
    auto ng = pathNeighbours(0);
    graph_->mergeNodes(myRootNode, otherRootNode, ng);
    for (std::size_t i = 0; i < longestMatch; ++i)
    {
        nodeId_t myCur = myCirc[myLongestMatchBegin + i].to,
                 otherCur = otherCirc[(otherLongestMatchBegin + i + 1) % otherCirc.size()].to;
        prevMyPath = myCur;
        ng = pathNeighbours(i + 1);
        graph_->mergeNodes(myCur, otherCur, ng);
    }
    graph_->node(prevMyPath).swapLastAdditions();

    terminal_ = myRootNode;

    return true;
}
} // namespace van_kampen
