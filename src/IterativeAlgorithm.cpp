#include "IterativeAlgorithm.hpp"
#include "VanKampenUtils.hpp"

namespace van_kampen
{
IterativeAlgorithm::IterativeAlgorithm()
    : diagramm_(graph_) {}

void IterativeAlgorithm::generate(const std::vector<std::vector<GroupElement>> &words_)
{
    auto words = words_;
    std::reverse(words.begin(), words.end());
    std::size_t totalIterations = words.size();
    if (cellsLimit)
    {
        totalIterations = std::min(totalIterations, cellsLimit);
    }
    std::vector<bool> isAdded(words.size());
    bool force = false;
    ProcessLogger logger(totalIterations, std::clog, "Relations used", quiet);
    std::reverse(words.begin(), words.end());
    std::reverse(isAdded.begin(), isAdded.end());
    auto iterateOverWordsOnce = [&]() {
        for (std::size_t i = 0; i < words.size(); ++i)
        {
            if (isAdded[i])
                continue;
            if (diagramm_.bindWord(words[i], force))
            {
                isAdded[i] = true;
                if (logger.iterate() >= totalIterations)
                {
                    break;
                }
            }
        }
    };
    iterateOverWordsOnce();
    force = true;
    iterateOverWordsOnce();
}

Diagramm &IterativeAlgorithm::diagramm()
{
    return diagramm_;
}
} // namespace van_kampen
