#pragma once

#include "DiagramGeneratingAlgorithm.hpp"

namespace van_kampen
{
    struct LargeFirstAlgorithm : DiagrammGeneratingAlgorithm
    {
        LargeFirstAlgorithm();

        void generate(const std::vector<std::vector<van_kampen::GroupElement>> &words) override;

        van_kampen::Diagramm &diagramm() override;

        std::size_t cellsLimit = 0;
        bool quiet = false;
        int maximalSmallForOneBig = 10;

    private:
        van_kampen::Diagramm diagramm_;
    };
} // namespace van_kampen
