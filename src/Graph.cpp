#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/fileformats/GraphIO.h>

#include "Graph.hpp"
#include "Utility.hpp"

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
std::deque<Transition> &Node::transitions() { return transitions_; }
nodeId_t Node::makeNonexistantNode() noexcept { return -1; }
bool Node::isNonexistantNode(nodeId_t id) noexcept { return id == -1; }

void Node::printSelfAndTransitions(std::ostream &os, graphOutputFormat fmt, bool last) const
{
    switch (fmt)
    {
    case graphOutputFormat::DOT:
    {
        std::string shape = isHighlighted_ ? "circle" : "point";
        std::string label = !label_.empty() ? ",label=" + label_ : "";
        std::string comment = !comment_.empty() ? ",xlabel=\"" + comment_ + "\"" : "";
        utility::print(os, id_, "[shape=", shape, label, comment, "];\n");
        break;
    }

    case graphOutputFormat::WOLFRAM_NOTEBOOK:
    case graphOutputFormat::TXT_EDGES:
        break;

    default:
        break;
    }

    std::size_t nonReservedCount = std::count_if(transitions_.begin(), transitions_.end(), [](const Transition &tr) { return !tr.label.reversed; });

    for (const auto &[nodeToId, transitionLabel, coordinate] : transitions_)
    {
        if (transitionLabel.reversed)
        {
            continue;
        }
        --nonReservedCount;
        const Node &nodeTo = graph_.nodes().at(nodeToId);
        switch (fmt)
        {
        case graphOutputFormat::DOT:
            utility::print(os, id_, "->", nodeTo.getId(), " [label=\"", transitionLabel.name, "\"];\n");
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

void Graph::printSelf(std::ostream &os, graphOutputFormat fmt)
{
    switch (fmt)
    {
    case graphOutputFormat::DOT:
        os << "digraph G {\n"
              "rankdir=LR;\n";
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
        nodes_[i].printSelfAndTransitions(os, fmt, i == nodes_.size() - 1);
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

void Graph::printSelfToSvg(const std::string &filename)
{
    ogdf::Graph ogdfGraph;
    ogdf::GraphAttributes ogdfGraphArrtibutes(ogdfGraph,
                                              ogdf::GraphAttributes::nodeGraphics |
                                                  ogdf::GraphAttributes::edgeGraphics |
                                                  ogdf::GraphAttributes::edgeStyle |
                                                  ogdf::GraphAttributes::edgeLabel |
                                                  ogdf::GraphAttributes::edgeArrow |
                                                  ogdf::GraphAttributes::nodeLabel |
                                                  ogdf::GraphAttributes::nodeStyle);
    std::vector<ogdf::node> ogdfNodes;
    ogdfGraphArrtibutes.directed() = true;
    ogdf::GraphIO::svgSettings.fontSize(1);

    const double scale = 10.0;
    const double nodeSize = 0.08;
    const double strokeWidth = 0.01;

    for (const Node &node : nodes_)
    {
        ogdfNodes.push_back(ogdfGraph.newNode(node.id_));
        ogdfGraphArrtibutes.x(ogdfNodes.back()) = node.position.x * scale;
        ogdfGraphArrtibutes.y(ogdfNodes.back()) = node.position.y * scale;
        ogdfGraphArrtibutes.shape(ogdfNodes.back()) = ogdf::Shape::Ellipse;
        ogdfGraphArrtibutes.width(ogdfNodes.back()) = nodeSize * scale;
        ogdfGraphArrtibutes.height(ogdfNodes.back()) = nodeSize * scale;
        if (node.isHighlighted_)
        {
            ogdfGraphArrtibutes.shape(ogdfNodes.back()) = ogdf::Shape::Hexagon;
        }
        ogdfGraphArrtibutes.label(ogdfNodes.back()) = std::to_string(node.id_);
        ogdfGraphArrtibutes.strokeWidth(ogdfNodes.back()) = strokeWidth;
    }

    for (std::size_t i = 0; i < ogdfNodes.size(); ++i)
    {
        ogdf::node ogdfNode = ogdfNodes[i];
        nodeId_t nodeId = nodes_[i].getId();
        for (const Transition &transition : node(nodeId).transitions())
        {
            if (transition.label.reversed)
            {
                continue;
            }
            ogdf::node ogdfNodeTo = ogdfNodes[transition.to];
            ogdf::edge edge = ogdfGraph.newEdge(ogdfNode, ogdfNodeTo);
            ogdf::DPolyline &line = ogdfGraphArrtibutes.bends(edge);
            line.pushBack(ogdf::DPoint(transition.edgeMedian.x * scale,
                                       transition.edgeMedian.y * scale));
            ogdfGraphArrtibutes.strokeWidth(edge) = strokeWidth * scale;
            ogdfGraphArrtibutes.arrowType(edge) = ogdf::EdgeArrow::First;
            ogdfGraphArrtibutes.label(edge) = transition.label.name;
        }
    }

    if (!ogdf::GraphIO::write(ogdfGraphArrtibutes, filename, ogdf::GraphIO::drawSVG))
    {
        throw std::invalid_argument("Can not write graph to '" + filename + "'");
    }
}

Node &Graph::node(nodeId_t it)
{
    return nodes_.at(it);
}

const std::deque<Node> &Graph::nodes() const
{
    return nodes_;
}

void Graph::mergeNodes(nodeId_t alive, nodeId_t dead)
{
    for (Transition &edgeFromDead : node(dead).transitions())
    {
        node(alive).addTransition(edgeFromDead.to, edgeFromDead.label);
        for (Transition &edge : node(edgeFromDead.to).transitions())
        {
            if (edge.to == dead)
            {
                edge.to = alive;
            }
        }
    }
}

void Graph::removeOrientedEdge(nodeId_t a, nodeId_t b)
{
    std::size_t idA = 0, idB = 0;
    bool edgeFound = false;
    for (; idA < node(a).transitions().size(); ++idA)
    {
        Transition &edge = node(a).transitions()[idA];
        if (edge.to != b)
        {
            continue;
        }
        for (; idB < node(b).transitions().size(); ++idB)
        {
            if (edge.to != a)
            {
                continue;
            }
            edgeFound = true;
        }
        if (!edgeFound)
        {
            throw std::invalid_argument("can not remove non-oriented edge");
        }
        break;
    }
    node(a).transitions_.erase(node(a).transitions_.begin() + idA);
    node(b).transitions_.erase(node(b).transitions_.begin() + idB);
}
} // namespace van_kampen
