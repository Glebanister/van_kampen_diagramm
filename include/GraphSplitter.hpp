#pragma once

#include <type_traits>
#include <cassert>

#include "Graph.hpp"

namespace van_kampen
{
    namespace details
    {
        template <typename Pred>
        void dfs(nodeId_t v,
                 const Graph &g,
                 std::unordered_map<nodeId_t, int> &color,
                 int curColor,
                 Graph &component,
                 std::deque<nodeId_t> &graphNodes,
                 Pred &pred,
                 std::unordered_map<nodeId_t, nodeId_t> &newNames)
        {
            color[v] = curColor;
            newNames[v] = component.addNode();
            graphNodes.push_back(v);
            for (const auto &tr : g.node(v).transitions())
            {
                if (!pred(tr) || color[tr.to])
                    continue;
                dfs(tr.to, g, color, curColor, component, graphNodes, pred, newNames);
            }
        }
    } // namespace details

    // Splits graph into strong components
    // Returns vector of Graphs which are components
    // All edges in one component are satisfying to predicate
    template <typename F>
    std::deque<Graph> splitToStrongComponents(const Graph &graph, F pred)
    {
        std::unordered_map<nodeId_t, int> color;
        std::deque<Graph> components;
        int curColor = 1;
        for (const auto &node : graph.nodes())
        {
            if (!color[node.getId()])
            {
                components.push_back(Graph{});
                std::unordered_map<nodeId_t, nodeId_t> newNames;
                std::deque<nodeId_t> graphNodes;
                details::dfs(node.getId(), graph, color, curColor++, components.back(), graphNodes, pred, newNames);
                for (nodeId_t from : graphNodes)
                {
                    for (const auto &to : graph.node(from).transitions())
                    {
                        if (!pred(to))
                            continue;
                        components.back().node(newNames[from]).addTransition(Transition{newNames[to.to], to.label, to.isInSquare, to.priority, to.isInHub});
                    }
                }
            }
        }
        return components;
    }
} // namespace van_kampen
