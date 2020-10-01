#include "Graph.hpp"

namespace van_kampen
{
Node::Node(Graph &g)
    : graph_(g), id_(g.nodes().size()) {}

void Node::addTransition(nodeId_t to, const GroupElement &label)
{
    transitions_.emplace_back(to, label);
}

nodeId_t Node::addTransitionToNewNode(const GroupElement &label)
{
    nodeId_t node = graph_.addNode();
    addTransition(node, label);
    return node;
}

void Node::swapLastAdditions()
{
    if (transitions_.size() < 2)
    {
        throw std::length_error("unable to swap last two");
    }
    std::swap(transitions_[transitions_.size() - 2], transitions_[transitions_.size() - 1]);
}

void Node::highlightNode(bool value) noexcept { isHighlighted_ = value; }
nodeId_t Node::getId() const noexcept { return id_; }
void Node::setDiagramLabel(const std::string &label) { label_ = label; }
void Node::setDiagramComment(const std::string &comment) { comment_ = comment; }
void Node::setDiagramLabel(std::string &&label) { label_ = std::move(label); }
void Node::setDiagramComment(std::string &&comment) { comment_ = std::move(comment); }
const std::deque<Transition> &Node::transitions() const { return transitions_; }
nodeId_t Node::makeNonexistantNode() noexcept { return -1; }
bool Node::isNonexistantNode(nodeId_t id) noexcept { return id == -1; }

// Print this node and all outgoing transitions
void Node::printSelfAndTransitions(std::ostream &os) const
{
    std::string shape = isHighlighted_ ? "circle" : "point";
    std::string label = !label_.empty() ? ",label=" + label_ : "";
    std::string comment = !comment_.empty() ? ",xlabel=\"" + comment_ + "\"" : "";
    print(os, id_, "[shape=", shape, label, comment, "];\n");
    for (const auto &[nodeToId, transitionLabel] : transitions_)
    {
        if (transitionLabel.reversed)
        {
            continue;
        }
        const Node &nodeTo = graph_.nodes().at(nodeToId);
        print(os, id_, "->", nodeTo.getId(), " [label=\"", transitionLabel.name, "\"];\n");
    }
}

nodeId_t Graph::addNode()
{
    nodes_.push_back(Node{*this});
    return nodes_.back().getId();
}

void Graph::printSelf(std::ostream &os)
{
    os << "digraph G {\n"
          "rankdir=LR;\n";

    for (const auto &node : nodes_)
    {
        node.printSelfAndTransitions(os);
    }

    os << "}\n";
    os.flush();
}

Node &Graph::node(nodeId_t it)
{
    return nodes_.at(it);
}

const std::deque<Node> &Graph::nodes() const
{
    return nodes_;
}
} // namespace van_kampen
