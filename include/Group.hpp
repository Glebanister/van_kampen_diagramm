#pragma once

#include "Graph.hpp"
#include "Geometry.hpp"

namespace van_kampen
{
    class Graph;
    class Node;
    struct Transition;
    using nodeId_t = int;

    // Group element
    class GroupElement
    {
    public:
        // Unique identificator of group element
        std::string name = "";

        // Is group element reversed
        bool reversed = false;

        bool operator==(const GroupElement &other) const;

        // Returns if this group element opposite to other
        bool isOpposite(const GroupElement &other);

        // Returns reversed version of this group element
        GroupElement inversed() const;

        // Inverse this group element to opposite
        void inverse() noexcept;
    };

    using nodeId_t = int;

    // Transition in graph
    struct Transition
    {
        // Id of following node
        nodeId_t to;

        // Label on corresponding edge
        GroupElement label;

        // Middle point of edge
        Point edgeMedian;

        Transition(nodeId_t to, const GroupElement &elem);
    };

    class Diagramm
    {
    public:
        Diagramm(Graph &graph);

        std::vector<Transition> getCircuit();
        bool bindWord(const std::vector<GroupElement> &word);
        nodeId_t getTerminal() const noexcept;
        void setTerminal(nodeId_t) noexcept;

    private:
        nodeId_t terminal_ = -1;
        Graph &graph_;
    };
} // namespace van_kampen
