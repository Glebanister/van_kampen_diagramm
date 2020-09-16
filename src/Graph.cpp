#include "Graph.hpp"

namespace van_kampmen
{
bool Node::reverse_iterator::isValid() const noexcept
{
    if (!node_)
    {
        return false;
    }
    return 0 <= lastTransitionId_ &&
           static_cast<std::size_t>(lastTransitionId_) < node_->notOwnedTransitions_.size() + node_->ownedTransitions_.size();
}

void Node::reverse_iterator::validate() const
{
    if (!isValid())
    {
        throw std::length_error("bad node iterator access");
    }
}

Node::reverse_iterator &Node::reverse_iterator::operator++() noexcept
{
    ++lastTransitionId_;
    return *this;
}

Node::reverse_iterator &Node::reverse_iterator::operator--() noexcept
{
    ++lastTransitionId_;
    return *this;
}

Node::reverse_iterator &Node::reverse_iterator::operator+=(std::size_t i) noexcept
{

    lastTransitionId_ += i;
    return *this;
}

Node::reverse_iterator &Node::reverse_iterator::operator-=(std::size_t i) noexcept
{
    lastTransitionId_ -= i;
    return *this;
}

bool Node::reverse_iterator::operator>(reverse_iterator &other)
{
    return lastTransitionId_ < other.lastTransitionId_;
}

bool Node::reverse_iterator::operator<(Node::reverse_iterator &other)
{
    return lastTransitionId_ > other.lastTransitionId_;
}

bool Node::reverse_iterator::operator==(Node::reverse_iterator &other)
{
    return lastTransitionId_ == other.lastTransitionId_;
}

bool Node::reverse_iterator::operator!=(Node::reverse_iterator &other)
{
    return lastTransitionId_ != other.lastTransitionId_;
}

std::pair<std::shared_ptr<Node>, GroupElement> Node::reverse_iterator::operator*() noexcept
{
    validate();
    auto iThTransitionData = node_->iThAddedTransitionData_[node_->iThAddedTransitionData_.size() - lastTransitionId_ - 1];
    if (iThTransitionData.first)
    {
        return node_->ownedTransitions_[iThTransitionData.second];
    }
    else
    {
        auto [node, label] = node_->notOwnedTransitions_[iThTransitionData.second];
        return {node.lock(), label};
    }
}

Node::reverse_iterator::reverse_iterator(Node &node, std::size_t id)
    : node_(&node), lastTransitionId_(id) {}

Node::Node(Graph &graph)
    : graph_(graph)
{
    static std::size_t cnt{0};
    id_ = ++cnt;
}

void Node::addTransition(std::weak_ptr<Node> to, const GroupElement &label)
{
    iThAddedTransitionData_.emplace_back(false, notOwnedTransitions_.size());
    notOwnedTransitions_.emplace_back(std::move(to), label);
    graph_.addNode(notOwnedTransitions_.back().first.lock());
}

void Node::swapLastAdditions()
{
    if (iThAddedTransitionData_.size() < 2)
    {
        throw std::length_error("unable to swap last two");
    }
    auto &last = iThAddedTransitionData_.back();
    auto &prevLast = iThAddedTransitionData_[iThAddedTransitionData_.size() - 2];
    std::swap(last, prevLast);
}

std::weak_ptr<Node> Node::addTransitionToNewNode(const GroupElement &label)
{
    iThAddedTransitionData_.emplace_back(true, ownedTransitions_.size());
    auto newNode = std::make_shared<Node>(graph_);
    graph_.addNode(newNode);
    return ownedTransitions_.emplace_back(std::move(newNode), label).first;
}

void Node::highlightNode(bool value) noexcept
{
    isHighlighted_ = value;
}

Node::reverse_iterator Node::begin() noexcept
{
    return reverse_iterator(*this, 0);
}

Node::reverse_iterator Node::end() noexcept
{
    return Node::reverse_iterator(*this, iThAddedTransitionData_.size());
}

std::size_t Node::getId() const noexcept
{
    return id_;
}

void Node::setLabel(const std::string &label)
{
    label_ = label;
}

void Node::setComment(const std::string &comment)
{
    comment_ = comment;
}

// Print this node and all outgoing transitions
void Node::printSelfAndTransitions(std::ostream &os, std::unordered_map<std::size_t, bool> &printed)
{
    std::string shape = isHighlighted_ ? "circle" : "point";
    std::string label = !label_.empty() ? ",label=" + label_ : "";
    std::string comment = !comment_.empty() ? ",xlabel=\"" + comment_ + "\"" : "";
    print(os, id_, "[shape=", shape, label, comment, "];\n");
    printed[id_] = true;
    auto printTransition = [&](std::size_t &id, const GroupElement &elem) {
        if (!elem.reversed)
        {
            print(os, id_, "->", id, " [label=\"", elem.name, "\"];\n");
        }
    };
    for (const auto &[tr, element] : notOwnedTransitions_)
    {
        printTransition(tr.lock()->id_, element);
    }
    for (const auto &[tr, element] : ownedTransitions_)
    {
        printTransition(tr->id_, element);
    }
}

void Graph::addNode(std::weak_ptr<Node> node)
{
    nodes_.emplace_back(std::move(node));
}

void Graph::printSelf(std::ostream &os)
{
    os << "digraph G {\n"
          "rankdir=LR;\n";

    std::unordered_map<std::size_t, bool> printed;
    for (const auto &node : nodes_)
    {
        if (node.expired())
        {
            throw std::bad_weak_ptr();
        }
        if (!printed[node.lock()->getId()])
        {
            node.lock()->printSelfAndTransitions(os, printed);
        }
    }

    os << "}\n";
}
} // namespace van_kampmen
