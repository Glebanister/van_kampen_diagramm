#include "Group.hpp"
#include "Graph.hpp"

namespace van_kampmen
{
bool GroupElement::operator==(const GroupElement &other) const
{
    return name == other.name && reversed == other.reversed;
}

bool GroupElement::isOpposite(const GroupElement &other)
{
    return name == other.name && reversed != other.reversed;
}

GroupElement GroupElement::inverse() const
{
    return {name, !reversed};
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
    auto nextBigWord = getWord();
    terminal_->highlightNode(false);
    terminal_ = nextBigWord[rand() % nextBigWord.size()].first.lock();
    terminal_->highlightNode(true);
}

std::shared_ptr<Node> Diagramm::getTerminal() const noexcept
{
    return terminal_;
}

void Diagramm::bindWord(const std::vector<GroupElement> &word)
{
    std::size_t wordLetterId = 0;
    auto curNode = terminal_;
    std::size_t notSplitterIters = 0;

    auto startNode = curNode;
    auto bigWord = getWord();

    std::size_t commonPrefix = 0;
    for (; commonPrefix < std::min(bigWord.size(), word.size()); ++commonPrefix)
    {
        if (bigWord[commonPrefix].second.name != word[commonPrefix].name ||
            bigWord[commonPrefix].second.reversed != word[commonPrefix].reversed)
        {
            break;
        }
    }

    if (commonPrefix + 1 == bigWord.size())
    {
        return;
    }

    for (; wordLetterId < word.size() && wordLetterId < bigWord.size(); ++wordLetterId)
    {
        if (!(word[wordLetterId] == bigWord[wordLetterId].second))
        {
            break;
        }
        curNode = bigWord[wordLetterId].first.lock();
        ++notSplitterIters;
    }

    if (notSplitterIters == word.size())
    {
        return;
    }

    bigWord.erase(bigWord.begin(), bigWord.begin() + notSplitterIters);
    std::size_t longestCommonSuffixLen = 0;
    for (std::size_t i = 0; i < bigWord.size() && i < word.size(); ++i)
    {
        if (!bigWord[bigWord.size() - 1 - longestCommonSuffixLen].second.isOpposite(word[word.size() - 1 - longestCommonSuffixLen]))
        {
            break;
        }
        ++longestCommonSuffixLen;
    }
    std::size_t freeLettersCnt = 0;
    for (; freeLettersCnt + longestCommonSuffixLen + notSplitterIters < word.size() - 1; ++freeLettersCnt)
    {
        auto prevNode = curNode;
        auto inversed = word[wordLetterId].inverse();
        curNode = curNode->addTransitionToNewNode(word[wordLetterId++]).lock();
        curNode->addTransition(prevNode, inversed);
    }
    if (bigWord.empty())
    {
        bigWord.emplace_back(terminal_, word.back());
    }
    int id = bigWord.size() - longestCommonSuffixLen - 1;
    id = std::max(0, id);
    curNode->addTransition(bigWord[id].first,
                           word[wordLetterId]);

    // if (id > 0)
    // {
    bigWord[id].first.lock()->addTransition(curNode, word[wordLetterId].inverse());
    auto lastBigWordLetter = bigWord[id].first.lock();
    if (lastBigWordLetter == startNode)
    {
        lastBigWordLetter->swapLastAdditions();
    }
    // }
    shuffleTerminal();
}
} // namespace van_kampmen
