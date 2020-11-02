#pragma once

#include "DiagramGeneratingAlgorithm.hpp"
#include "VanKampenUtils.hpp"

namespace van_kampen
{
    struct MergingAlgorithm : DiagrammGeneratingAlgorithm
    {
        MergingAlgorithm();

        void generate(const std::vector<std::vector<GroupElement>> &words) override;

        Diagramm &diagramm() override;

        std::size_t limit = 0;
        bool quiet = false;
        Diagramm result_;
    };
} // namespace van_kampen
