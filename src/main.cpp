#include "Graph.hpp"
#include "Group.hpp"
#include "GroupRepresentationParser.hpp"

int main()
{
    // van_kampmen::Graph graph;
    // van_kampmen::Diagramm diagramm(graph);
    // diagramm.bindWord(
    //     {
    //         {'a', true},
    //         {'b', true},
    //         {'b', false},
    //         {'a', false},
    //     });
    // graph.printSelf(std::cout);

    auto words = van_kampmen::GroupRepresentationParser::parse("<a, b, c | ab*c, bca>");

    for (auto word : words)
    {
        for (auto ch : word)
        {
            std::cout << ch.name << ' ' << ch.reversed << std::endl;
        }
        std::cout << std::endl;
    }
}
