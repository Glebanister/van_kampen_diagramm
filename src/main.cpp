#include <random>
#include <regex>
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

    std::string filename(argv[1]);
    std::ifstream file(filename);

    if (!file.good())
    {
        throw std::invalid_argument("file '" + filename + "' not found");
    }

    std::string text((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());

    auto words = van_kampmen::GroupRepresentationParser::parse(text);
    van_kampmen::Graph graph;
    van_kampmen::Diagramm diagramm(graph);

    int prevRes = -1;
    size_t cnt = 0;
    for (auto word : words)
    {
        int res = static_cast<int>(static_cast<double>(cnt++) / static_cast<double>(words.size()) * 1000.0);
        if (res != prevRes)
        {
            std::cout << "\rprogress (%): " << static_cast<double>(res) / 10.0;
            prevRes = res;
        }
        diagramm.bindWord(word);
    }
    std::cout << '\r';

    std::stringstream word;
    for (auto letter : diagramm.getWord())
    {
        word << letter.second.name << (letter.second.reversed ? "*" : "");
    }
    diagramm.getTerminal()->setLabel("S");
    // diagramm.getTerminal()->setComment(word.str());

    std::string outputFilename = filename + ".dot";
    std::ofstream outFile(outputFilename);
    graph.printSelf(outFile);
}
