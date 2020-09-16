#include <random>
#include <sstream>

#include "Graph.hpp"
#include "Group.hpp"
#include "GroupRepresentationParser.hpp"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        throw std::invalid_argument("specify group representation");
    }
    auto words = van_kampmen::GroupRepresentationParser::parse(argv[1]);
    van_kampmen::Graph graph;
    van_kampmen::Diagramm diagramm(graph);

    std::mt19937 random;
    auto randint = [&](int begin, int end) {
        return random() % (end - begin + 1) + begin;
    };

    for (auto word : words)
    {
        diagramm.bindWord(word);
        for (int it = randint(0, word.size()); it; --it)
        {
            std::rotate(word.begin(), word.begin() + 1, word.end());
            diagramm.bindWord(word);
        }
    }

    std::stringstream word;
    for (auto letter : diagramm.getWord())
    {
        word << letter.second.name << (letter.second.reversed ? "*" : "");
    }
    diagramm.getTerminal()->setLabel("S");
    diagramm.getTerminal()->setComment(word.str());
    graph.printSelf(std::cout);
}
