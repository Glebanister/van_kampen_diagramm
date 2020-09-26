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

Diagramm::Diagramm(Graph &graph)
    : graph_(graph)
{
    terminal_ = std::make_shared<Node>(graph_);
    firstNode_ = terminal_;
    graph_.addNode(terminal_);
}

std::vector<std::pair<std::weak_ptr<Node>, GroupElement>> Diagramm::getWord()
{
    std::vector<std::pair<std::weak_ptr<Node>, GroupElement>> result{};
    auto begin = terminal_;
    auto curNode = begin;
    auto getNextNode = [&](std::shared_ptr<Node> node) {
        if (!node)
        {
            return std::shared_ptr<Node>();
        }
        if (!(node->begin().isValid()))
        {
            return std::shared_ptr<Node>();
        }
        return (*node->begin()).first;
    };
    curNode = getNextNode(begin);
    if (!curNode)
    {
        return result;
    }
    result.emplace_back(*begin->begin());
    while (curNode != begin)
    {
        result.emplace_back(*curNode->begin());
        curNode = getNextNode(curNode);
        if (!curNode)
        {
            throw std::invalid_argument("can't read word");
        }
    }
    return result;
}

void Diagramm::shuffleTerminal()
{
    static int seed = 0;
    auto nextBigWord = getWord();
    if (nextBigWord.size() == 0)
    {
        return;
    }
    terminal_->highlightNode(false);
    terminal_ = nextBigWord[(rand() + seed++) % nextBigWord.size()].first.lock();
    terminal_->highlightNode(true);
}

std::shared_ptr<Node> Diagramm::getTerminal() const noexcept
{
    return terminal_;
}

void Diagramm::bindWord(const std::vector<GroupElement> &word)
{
    class TerminalShuffler
    {
    public:
        TerminalShuffler(Diagramm &dg)
            : diagramm(dg) {}

        ~TerminalShuffler()
        {
            diagramm.shuffleTerminal();
        }

    private:
        Diagramm &diagramm;
    } shuffer(*this);

    auto circleWord = getWord();
    if (circleWord.empty())
    {
        auto curNode = std::weak_ptr(terminal_);
        for (std::size_t i = 0; i < word.size() - 1; ++i)
        {
            auto prevNode = curNode;
            curNode = prevNode.lock()->addTransitionToNewNode(word[i]);
            curNode.lock()->addTransition(prevNode, word[i].inversed());
        }
        curNode.lock()->addTransition(terminal_, word.back());
        terminal_->addTransition(curNode.lock(), word.back().inversed());
        terminal_->swapLastAdditions();
        return;
    }

    auto reversedCircleWord = circleWord;
    for (auto &letter : reversedCircleWord)
    {
        letter.second.inverse();
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
        for (auto &letter : reversedCircleWord)
        {
            addToken(letter.second);
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
                (rand() % 15 && pi[i] == static_cast<int>(longestEntry)))
            {
                longestEntry = pi[i];
                entryBegin = i - longestEntry - word.size();
            }
        }
    }

    if (longestEntry == 0 ||
        longestEntry == circleWord.size() ||
        entryBegin == 0 ||
        longestEntry == word.size() ||
        entryBegin + longestEntry == circleWord.size())
    {
        return;
    }

    std::size_t normalWordEntryBegin = circleWord.size() - longestEntry - entryBegin;
    auto branchFrom = circleWord[normalWordEntryBegin - 1].first;
    auto branchTo = circleWord[normalWordEntryBegin + longestEntry - 1].first;

    auto curNode = branchFrom;

    for (std::size_t i = longestEntry; i < word.size() - 1; ++i)
    {
        auto prevNode = curNode;
        curNode = prevNode.lock()->addTransitionToNewNode(word[i]);
        curNode.lock()->addTransition(prevNode, word[i].inversed());
    }
    curNode.lock()->addTransition(branchTo, word.back());
    branchTo.lock()->addTransition(curNode, word.back().inversed());
    branchTo.lock()->swapLastAdditions();
}
} // namespace van_kampmen
