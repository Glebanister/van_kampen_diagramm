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
    bool isAdditionForced = false;
    ProcessLogger logger(totalIterations, std::clog, "Relations used", quiet);
    std::size_t increase = 0;
    auto iterateOverWordsOnce = [&]() {
        for (std::size_t i = 0; i < words.size(); ++i)
        {
            if (logger.getIteration() >= totalIterations)
            {
                break;
            }
            if (isAdded[i])
                continue;
            if (diagramm_.bindWord(words[i], isAdditionForced))
            {
                isAdded[i] = true;
                increase += 1;
                if (logger.iterate() >= totalIterations)
                {
                    break;
                }
            }
        }
    };
    increase = 1;
    while (increase)
    {
        increase = 0;
        iterateOverWordsOnce();
    }
    isAdditionForced = true;
    increase = 1;
    while (increase)
    {
        increase = 0;
        iterateOverWordsOnce();
    }
    if (logger.getIteration() < totalIterations && !quiet)
    {
        std::clog << "can not bind " << totalIterations - logger.getIteration() << " relations, finishing";
    }
}

Diagramm &IterativeAlgorithm::diagramm()
{
    return diagramm_;
}
} // namespace van_kampen
