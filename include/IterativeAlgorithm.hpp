#pragma once

#include "DiagramGeneratingAlgorithm.hpp"

namespace van_kampen
{
    struct IterativeAlgorithm : DiagrammGeneratingAlgorithm
    {
        IterativeAlgorithm();

        void generate(const std::vector<std::vector<van_kampen::GroupElement>> &words_) override;

        van_kampen::Diagramm &diagramm() override;

        std::size_t cellsLimit = 0;
        bool quiet = false;

    private:
        van_kampen::Diagramm diagramm_;
    };
} // namespace van_kampen
