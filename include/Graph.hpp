#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include "Group.hpp"

namespace van_kampmen
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

    // Van Kanpmen graph node
    class Node
    {
    public:
        class reverse_iterator
        {
        public:
            bool isValid() const noexcept;
            void validate() const;
            reverse_iterator &operator++() noexcept;
            reverse_iterator &operator--() noexcept;
            reverse_iterator &operator+=(std::size_t i) noexcept;
            reverse_iterator &operator-=(std::size_t i) noexcept;
            bool operator>(reverse_iterator &other);
            bool operator<(reverse_iterator &other);
            bool operator==(reverse_iterator &other);
            bool operator!=(reverse_iterator &other);
            std::pair<std::shared_ptr<Node>, GroupElement> operator*() noexcept;

        private:
            friend class Node;
            explicit reverse_iterator(Node &node, std::size_t id = 0);
            Node *node_;
            int lastTransitionId_;
        };

        Node(Graph &graph);

        // Add transition to existing node with label
        void addTransition(std::weak_ptr<Node> to, const GroupElement &label);

        void swapLastAdditions();

        // Add transition to new node
        std::weak_ptr<Node> addTransitionToNewNode(const GroupElement &label);

        // Highlight node on diagramm
        void highlightNode(bool) noexcept;

        reverse_iterator begin() noexcept;
        reverse_iterator end() noexcept;

        std::size_t getId() const noexcept;

        void setLabel(const std::string &);
        void setComment(const std::string &);

    private:
        friend class Graph;
        // Print this node and all outgoing transitions
        void printSelfAndTransitions(std::ostream &os, std::unordered_map<std::size_t, bool> &printed);

        std::vector<std::pair<std::weak_ptr<Node>, GroupElement>> notOwnedTransitions_;
        std::vector<std::pair<std::shared_ptr<Node>, GroupElement>> ownedTransitions_;
        std::vector<std::pair<bool, std::size_t>> iThAddedTransitionData_; // [is_owned, id in its array]
        bool isHighlighted_ = false;
        std::size_t id_;
        Graph &graph_;
        std::string label_ = "";
        std::string comment_ = "";
    };

    class Graph
    {
    public:
        void addNode(std::weak_ptr<Node> node);

        void printSelf(std::ostream &os);

    private:
        std::vector<std::weak_ptr<Node>> nodes_;
    };
} // namespace van_kampmen
