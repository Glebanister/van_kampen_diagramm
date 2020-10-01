#pragma once

#include <Graph.hpp>

namespace van_kampen
{
    class Graph;
    class Node;
    struct Transition;
    using nodeId_t = int;

    class GroupElement
    {
    public:
        std::string name = "";
        bool reversed = false;

        bool operator==(const GroupElement &other) const;
        bool isOpposite(const GroupElement &other);

        GroupElement inversed() const;
        void inverse() noexcept;
    };

    using nodeId_t = int;

    struct Transition
    {
        nodeId_t to;
        GroupElement label;

        Transition(nodeId_t to, const GroupElement &elem);
    };

    class Diagramm
    {
    public:
        Diagramm(Graph &graph);

        std::vector<Transition> getCircuit();
        bool bindWord(const std::vector<GroupElement> &word);
        void shuffleTerminal();
        nodeId_t getTerminal() const noexcept;

    private:
        nodeId_t terminal_ = -1;
        Graph &graph_;
    };
} // namespace van_kampen
