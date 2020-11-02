#include "MergingAlgorithm.hpp"

namespace van_kampen
{

MergingAlgorithm::MergingAlgorithm()
    : result_(graph_) {}

void MergingAlgorithm::generate(const std::vector<std::vector<van_kampen::GroupElement>> &words)
{
    std::vector<van_kampen::Diagramm> diagrams;
    diagrams.reserve(words.size());
    int left = limit ? limit : words.size();
    for (auto &word : words)
    {
        if (!left--)
        {
            // TODO: Implement limitation
        }
        diagrams.push_back(van_kampen::Diagramm{graph_});
        diagrams.back().bindWord(word, false);
    }
    van_kampen::ProcessLogger log{diagrams.size(), std::cout, "Merging", quiet};
    while (diagrams.size() > 1)
    {
        std::vector<std::pair<van_kampen::Diagramm, std::vector<van_kampen::Transition>>> circuits;
        for (std::size_t i = 0; i < diagrams.size(); ++i)
        {
            circuits.push_back({diagrams[i], diagrams[i].getCircuit()});
        }
        auto compareCircuitPrefix = [circuits](std::size_t first,
                                               std::size_t second) {
            std::size_t i = 0;
            for (; i < circuits[first].second.size() &&
                   i < circuits[second].second.size() &&
                   circuits[first].second[i].label == circuits[second].second[i].label;
                 ++i)
                ;
            if (i == circuits[first].second.size())
            {
                return true;
            }
            if (i == circuits[second].second.size())
            {
                return false;
            }
            return circuits[first].second[i].label.name.front() < circuits[second].second[i].label.name.front();
        };
        std::vector<std::size_t> order(circuits.size());
        std::generate(order.begin(), order.end(), [&, x = 0]() mutable { return x++; });
        diagrams.clear();
        std::stable_sort(order.begin(), order.end(), compareCircuitPrefix);
        bool atleastOne = false;
        for (std::size_t i = 0; i + 1 < circuits.size(); i += 2)
        {
            auto &&cur = circuits[order[i]].first;
            auto &&next = circuits[order[i + 1]].first;
            if (!cur.merge(std::move(next)))
            {
                diagrams.push_back(std::move(cur));
                diagrams.push_back(std::move(next));
            }
            else
            {
                log.iterate();
                atleastOne = true;
                diagrams.push_back(std::move(cur));
            }
        }
        if (!atleastOne)
        {
            throw std::logic_error("cannot build diagram");
        }
    }
    result_ = diagrams[0];
}

Diagramm &MergingAlgorithm::diagramm()
{
    return result_;
}
} // namespace van_kampen
