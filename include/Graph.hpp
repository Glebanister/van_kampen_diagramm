#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <deque>

#include "Group.hpp"
#include "Geometry.hpp"

namespace van_kampen
{
    enum class graphOutputFormat
    {
        // Format is not defined
        UNDEFINED,

        // .dot format for graphviz
        DOT,

        // .nb format for wolfram notebook
        WOLFRAM_NOTEBOOK,

        // .edges - list of graph edges
        TXT_EDGES,
    };

    class GroupElement;
    class Diagramm;
    class Graph;

    struct Transition;

    using nodeId_t = int;

    // Van Kanpmen graph node
    class Node
    {
    public:
        // Node position in 2-dimentional space
        Point position;

        // Add transition to existing node with label
        void addTransition(nodeId_t to, const GroupElement &label, bool inSquare);

        // Swap last two additions order
        void swapLastAdditions();

        // Add transition to new node
        nodeId_t addTransitionToNewNode(const GroupElement &label, bool inSquare);

        // Set is node highlighted on diagram
        void highlightNode(bool) noexcept;

        nodeId_t getId() const noexcept;

        void setDiagramLabel(const std::string &);
        void setDiagramComment(const std::string &);
        void setDiagramLabel(std::string &&);
        void setDiagramComment(std::string &&);

        const std::deque<Transition> &transitions() const;
        std::deque<Transition> &transitions();

        static nodeId_t makeNonexistantNode() noexcept;
        static bool isNonexistantNode(nodeId_t) noexcept;

    private:
        Node(Graph &graph);

        // Print this node
        void printSelf(std::ostream &os, graphOutputFormat) const;
        // Print all outgoing transitions
        void printTransitions(std::ostream &os, graphOutputFormat, bool last) const;

        std::deque<Transition> transitions_; // List of node adjacent nodes
        bool isHighlighted_ = false;         // Is node marked as terminal on diagram
        Graph &graph_;                       // Corresponding graph reference
        const nodeId_t id_;                  // Node id in graph
        std::string label_ = "";             // Node label on diagram
        std::string comment_ = "";           // Node comment on diagram

        friend class Graph;
    };

    // Graph where diagram is being built
    class Graph
    {
    public:
        // Add node to graph
        // Returns id of new node
        nodeId_t addNode();

        // Print graph to ostream in graphOutputFormat
        void printSelf(std::ostream &os, graphOutputFormat);

        // Get list of graph nodes
        // Returns const reference on list
        const std::deque<Node> &nodes() const;

        // Get graph node by id
        // Returns reference on node
        Node &node(nodeId_t it);

        // Merge other to node, now what became dest
        // Works if graph is not oriented
        // untouchable nodes will not be affected
        void mergeNodes(nodeId_t dest, nodeId_t what, const std::unordered_set<nodeId_t> &untouchable = {});

        // Removes edges a -> b, b -> a
        void removeOrientedEdge(nodeId_t, nodeId_t);

    private:
        std::deque<Node> nodes_;
        std::unordered_set<nodeId_t> removedNodes_;
    };
} // namespace van_kampen
