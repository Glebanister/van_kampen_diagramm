#include <filesystem>

#include "cxxopts.hpp"

#include "ConsoleFlags.hpp"
#include "GraphSplitter.hpp"
#include "GroupRepresentationParser.hpp"
#include "IterativeAlgorithm.hpp"
#include "LargeFirstAlgorithm.hpp"
#include "MergingAlgorithm.hpp"

int main(int argc, const char **argv)
{
    using namespace van_kampen;

    try
    {
        van_kampen::ConsoleFlags flags(argc, argv);
        std::ifstream inputFile(flags.inputFileName);
        if (!inputFile.good())
        {
            throw std::invalid_argument("cannot open '" + flags.inputFileName + "'");
        }
        std::string text((std::istreambuf_iterator<char>(inputFile)),
                         std::istreambuf_iterator<char>());

        std::vector<std::vector<van_kampen::GroupElement>> words = van_kampen::GroupRepresentationParser::parse(text);
        auto hub = words.back();
        if (!flags.quiet)
        {
            std::clog << "Total relations count: " << words.size() << std::endl;
            std::clog << "Hub size: " << hub.size() << std::endl;
        }
        words.pop_back();
        if (flags.shuffleGroup)
        {
            std::random_shuffle(words.begin(), words.end());
        }
        if (!flags.notSort)
        {
            std::stable_sort(words.begin(),
                             words.end(),
                             [](const std::vector<van_kampen::GroupElement> &a, const std::vector<van_kampen::GroupElement> &b) {
                                 return a.size() < b.size();
                             });
        }
        words.push_back(hub);
        std::unique_ptr<DiagrammGeneratingAlgorithm> algo;
        if (flags.iterativeAlgo)
        {
            auto iterative = std::make_unique<IterativeAlgorithm>();
            iterative->cellsLimit = flags.cellsLimit;
            iterative->quiet = flags.quiet;
            algo.reset(iterative.release());
        }
        else if (flags.mergingAlgo)
        {
            auto merging = std::make_unique<MergingAlgorithm>();
            merging->limit = flags.cellsLimit;
            merging->quiet = flags.quiet;
            algo.reset(merging.release());
        }
        else if (flags.largeFirstAlgo)
        {
            auto largeFirst = std::make_unique<LargeFirstAlgorithm>();
            largeFirst->cellsLimit = flags.cellsLimit;
            largeFirst->quiet = flags.quiet;
            largeFirst->maximalSmallForOneBig = flags.perLarge;
            algo.reset(largeFirst.release());
        }

        algo->generate(words);

        {
            std::ofstream wordOutputFile(flags.wordOutputFileName);
            if (!wordOutputFile.good())
            {
                std::cerr << "cannot write to file '" << flags.wordOutputFileName << "'" << std::endl;
            }
            else
            {
                std::vector<van_kampen::Transition> word = algo->diagramm().getCircuit();
                for (std::size_t i = 0; i < word.size(); ++i)
                {
                    auto &letter = word[i];
                    wordOutputFile << letter.label.name << (letter.label.reversed ? "^(-1)" : "");
                    if (i < word.size() - 1)
                    {
                        wordOutputFile << "*";
                    }
                }
            }
        }


        if (!flags.split)
        {
            std::ofstream outFile(flags.outputFileName);
            if (!outFile.good())
            {
                throw std::invalid_argument("cannot write to file '" + flags.outputFileName + "'");
            }
            algo->graph().printSelf(outFile, flags.outputFormat);
        }
        else
        {
            std::deque<van_kampen::Graph> comps = splitToStrongComponents(algo->graph(), [](const Transition &tr) {
                return tr.priority >= 0.01;
            });
            std::filesystem::create_directory(flags.outputFileNameWoEx);
            std::size_t compId = 1;
            for (const van_kampen::Graph &comp : comps)
            {
                if (comp.nodes().size() < 2)
                    continue;
                std::ofstream outFile(std::filesystem::path(flags.outputFileNameWoEx) / (std::to_string(compId) + "." + flags.outputFormatString));
                comp.printSelf(outFile, flags.outputFormat);
                ++compId;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 0;
    }
}
