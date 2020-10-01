#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <deque>

#include "Group.hpp"

namespace van_kampen
{
    class GroupElement;
    class Diagramm;

    template <typename... ToPrint>
    void print(std::ostream &os, ToPrint &&... args)
    {
        (os << ... << args);
        os.flush();
    }
    // Van Kanpmen graph
    class Graph;
    using nodeId_t = int;
    struct Transition;

    // Van Kanpmen graph node
    class Node
    {
    public:
        // Add transition to existing node with label
        void addTransition(nodeId_t to, const GroupElement &label);

        void swapLastAdditions();

        // Add transition to new node
        nodeId_t addTransitionToNewNode(const GroupElement &label);

        // Highlight node on diagramm
        void highlightNode(bool) noexcept;

        nodeId_t getId() const noexcept;

        void setDiagramLabel(const std::string &);
        void setDiagramComment(const std::string &);
        void setDiagramLabel(std::string &&);
        void setDiagramComment(std::string &&);

        const std::deque<Transition> &transitions() const;

        static nodeId_t makeNonexistantNode() noexcept;
        static bool isNonexistantNode(nodeId_t) noexcept;

    private:
        Node(Graph &graph);
        // Print this node and all outgoing transitions
        void printSelfAndTransitions(std::ostream &os) const;

        std::deque<Transition> transitions_;
        bool isHighlighted_ = false;
        Graph &graph_;
        const nodeId_t id_;
        std::string label_ = "";
        std::string comment_ = "";

        friend class Graph;
    };

    class Graph
    {
    public:
        nodeId_t addNode();
        void printSelf(std::ostream &os);
        const std::deque<Node> &nodes() const;
        Node &node(nodeId_t it);

    private:
        std::deque<Node> nodes_;
    };
} // namespace van_kampen
