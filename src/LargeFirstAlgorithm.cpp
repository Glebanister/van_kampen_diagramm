#include "LargeFirstAlgorithm.hpp"
#include "VanKampenUtils.hpp"

namespace van_kampen
{

LargeFirstAlgorithm::LargeFirstAlgorithm()
    : diagramm_(graph_) {}

void LargeFirstAlgorithm::generate(const std::vector<std::vector<GroupElement>> &words)
{
    std::size_t totalIterations = words.size();
    if (cellsLimit)
    {
        totalIterations = std::min(totalIterations, cellsLimit);
    }
    ProcessLogger logger(totalIterations, std::clog, "Relations used", quiet);

    std::vector<bool> isAdded(words.size());
    using iterator = std::vector<std::vector<GroupElement>>::const_iterator;
    auto added = [&](iterator it) {
        return isAdded[it - begin(words)];
    };
    diagramm_.bindWord(words.back(), false, true);
    isAdded.back() = true;
    logger.iterate();
    bool oneAdded = false;
    bool force = false;
    auto add = [&](iterator it) {
        if (added(it))
        {
            throw std::logic_error("trying to add already added word");
        }
        if (diagramm_.bindWord(*it, force, false))
        {
            isAdded[it - begin(words)] = true;
            oneAdded = true;
            return true;
        }
        return false;
    };
    auto smallIt = words.begin();
    auto bigIt = prev(prev(words.end()));
    auto nextNotAdded = [&](iterator it) {
        while (it < end(words) && isAdded[it - begin(words)])
            ++it;
        return it;
    };
    auto prevNotAdded = [&](iterator it) {
        while (--it >= begin(words) && isAdded[it - begin(words)])
            ;
        return it;
    };
    while (true)
    {
        int rest = maximalSmallForOneBig;
        bool infinite = rest == 0;
        bool success = true;
        while (!add(bigIt))
        {
            if (rest-- == 0 && !infinite)
            {
                success = false;
                break;
            }

            if (add(smallIt) && logger.iterate() >= totalIterations)
                return;

            smallIt = nextNotAdded(smallIt + 1);
        }

        if (success && logger.iterate() >= totalIterations)
            return;

        bigIt = prevNotAdded(bigIt);
        if (smallIt >= bigIt)
        {
            smallIt = nextNotAdded(begin(words));
            bigIt = prevNotAdded(words.end());

            if (!oneAdded)
            {
                if (force && !quiet)
                {
                    if (!quiet)
                        std::clog << "can not bind " << totalIterations - logger.getIteration() << " relations, finishing";
                    return;
                }
                force = true;
            }
            oneAdded = false;
        }
    }
}

van_kampen::Diagramm &LargeFirstAlgorithm::diagramm()
{
    return diagramm_;
}
} // namespace van_kampen
