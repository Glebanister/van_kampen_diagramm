#pragma once

#include <Graph.hpp>

namespace van_kampmen
{
    class Graph;
    class Node;

    class GroupElement
    {
    public:
        std::string name = "";
        bool reversed = false;

        bool operator==(const GroupElement &other) const;

        bool isOpposite(const GroupElement &other);

        GroupElement inverse() const;
    };

    class Diagramm
    {
    public:
        Diagramm(Graph &graph);

        std::vector<std::pair<std::weak_ptr<Node>, GroupElement>> getWord();
        void bindWord(const std::vector<GroupElement> &word);
        void shuffleTerminal();
        std::shared_ptr<Node> getTerminal() const noexcept;

    private:
        std::shared_ptr<Node> terminal_ = nullptr;
        Graph &graph_;
        std::shared_ptr<Node> firstNode_ = nullptr;
    };
} // namespace van_kampmen