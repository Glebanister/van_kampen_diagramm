#include "Graph.hpp"
#include "Utility.hpp"

namespace van_kampen
{
Node::Node(Graph &g)
    : graph_(g), id_(g.nodes().size()) {}

void Node::addTransition(nodeId_t to, const GroupElement &label, bool inSquare, bool isHub)
{
    transitions_.push_back(Transition{to, label, inSquare, 0.0, isHub});
}

void Node::addTransition(Transition &&tr)
{
    transitions_.push_back(std::move(tr));
}

nodeId_t Node::addTransitionToNewNode(const GroupElement &label, bool inSquare, bool isHub)
{
    nodeId_t node = graph_.addNode();
    addTransition(node, label, inSquare, isHub);
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
std::deque<Transition> &Node::transitions() { return transitions_; }
nodeId_t Node::makeNonexistantNode() noexcept { return -1; }
bool Node::isNonexistantNode(nodeId_t id) noexcept { return id == -1; }

void Node::printSelf(std::ostream &os, graphOutputFormat fmt) const
{
    switch (fmt)
    {
    case graphOutputFormat::DOT:
    {
        std::string shape = std::string{"shape="} + (isHighlighted_ ? "circle" : "point");
        std::string label = !label_.empty() ? ",label=" + label_ : "";
        std::string comment = !comment_.empty() ? ",xlabel=\"" + comment_ + "\"" : "";
        utility::print(os, id_, "[", shape, label, comment, "];\n");
        break;
    }

    case graphOutputFormat::WOLFRAM_NOTEBOOK:
    case graphOutputFormat::TXT_EDGES:
        break;

    default:
        break;
    }
}

void Node::printTransitions(std::ostream &os, graphOutputFormat fmt, bool last) const
{
    std::size_t nonReservedCount = std::count_if(transitions_.begin(), transitions_.end(), [](const Transition &tr) { return !tr.label.reversed; });
    for (const auto &[nodeToId, transitionLabel, _, weight, inHub] : transitions_)
    {
        if (transitionLabel.reversed)
        {
            continue;
        }
        --nonReservedCount;
        const Node &nodeTo = graph_.node(nodeToId);
        switch (fmt)
        {
        case graphOutputFormat::DOT:
            utility::print(os, id_, "->", nodeTo.getId(),
                           " [fontsize=12, arrowhead=vee, label=\"", transitionLabel.name, "\", penwidth=", inHub ? 5 : 1, "];\n");
            break;

        case graphOutputFormat::WOLFRAM_NOTEBOOK:
            utility::print(os, '{', id_, ", ", nodeTo.getId(), '}', ((last && !nonReservedCount) ? "" : ", "));
            break;

        case graphOutputFormat::TXT_EDGES:
            utility::print(os, id_, " ", nodeTo.getId(), '\n');
            break;

        default:
            break;
        }
    }
}

nodeId_t Graph::addNode()
{
    nodes_.push_back(Node{*this});
    return nodes_.back().getId();
}

void Graph::increaseDirEdgePriority(nodeId_t from, nodeId_t to, double value)
{
    for (Transition &tr : node(from).transitions())
    {
        if (tr.to == to)
        {
            tr.priority += value;
            return;
        }
    }
    throw std::invalid_argument("can not increase priority of edge " +
                                std::to_string(from) + " -> " +
                                std::to_string(to) +
                                ": it does not exist");
}

void Graph::increaseNondirEdgePriority(nodeId_t a, nodeId_t b, double value)
{
    increaseDirEdgePriority(a, b, value);
    increaseDirEdgePriority(b, a, value);
}

void Graph::printSelf(std::ostream &os, graphOutputFormat fmt) const
{
    switch (fmt)
    {
    case graphOutputFormat::DOT:
        os << "digraph G {\n"
              "rankdir=LR;\n"
              "layout=neato;\n";
        break;
    case graphOutputFormat::WOLFRAM_NOTEBOOK:
        os << "Graph[Rule @@@ {";
        break;

    case graphOutputFormat::TXT_EDGES:
        break;

    default:
        break;
    }

    for (std::size_t i = 0; i < nodes_.size(); ++i)
    {
        if (removedNodes_.find(i) != removedNodes_.end())
        {
            continue;
        }
        nodes_[i].printSelf(os, fmt);
    }
    for (std::size_t i = 0; i < nodes_.size(); ++i)
    {
        if (removedNodes_.find(i) != removedNodes_.end())
        {
            continue;
        }
        nodes_[i].printTransitions(os, fmt, i == nodes_.size() - 1);
    }

    switch (fmt)
    {
    case graphOutputFormat::DOT:
        os << "}\n";
        break;

    case graphOutputFormat::WOLFRAM_NOTEBOOK:
        os << "}, GraphLayout -> \"PlanarEmbedding\"]\n";
        break;

    case graphOutputFormat::TXT_EDGES:
        break;

    default:
        break;
    }
    os.flush();
}

Node &Graph::node(nodeId_t it)
{
    return nodes_.at(it);
}

const Node &Graph::node(nodeId_t id) const
{
    return nodes_.at(id);
}

const std::deque<Node> &Graph::nodes() const
{
    return nodes_;
}

void Graph::mergeNodes(nodeId_t alive, nodeId_t dead, const std::unordered_set<nodeId_t> &untouchable)
{
    for (Transition &edgeFromDead : node(dead).transitions())
    {
        if (untouchable.count(edgeFromDead.to))
        {
            continue;
        }
        node(alive).addTransition(edgeFromDead.to, edgeFromDead.label, false, false); // TODO
        for (Transition &edge : node(edgeFromDead.to).transitions())
        {
            if (edge.to == dead)
            {
                edge.to = alive;
            }
        }
    }
    removedNodes_.insert(dead);
}

void Graph::removeOrientedEdge(nodeId_t a, nodeId_t b)
{
    auto maybeRemove = [this](nodeId_t x, nodeId_t y) {
        std::size_t id = 0;
        for (; id < node(x).transitions().size() && node(x).transitions()[id].to != y; ++id)
            ;
        if (id < node(x).transitions().size())
            node(x).transitions_.erase(node(x).transitions_.begin() + id);
    };
    maybeRemove(a, b);
    maybeRemove(b, a);
}
} // namespace van_kampen
