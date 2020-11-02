#pragma once

#include <vector>

#include "Group.hpp"

namespace van_kampen
{
    struct DiagrammGeneratingAlgorithm
    {
        virtual void generate(const std::vector<std::vector<van_kampen::GroupElement>> &words) = 0;
        virtual van_kampen::Diagramm &diagramm() = 0;
        virtual ~DiagrammGeneratingAlgorithm() = default;
        van_kampen::Graph &graph() { return *graph_; }

    protected:
        std::shared_ptr<van_kampen::Graph> graph_ = std::make_shared<van_kampen::Graph>();
    };
} // namespace van_kampen
