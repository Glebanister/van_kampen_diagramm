#include <random>
#include <sstream>

#include "cxxopts.hpp"

#include "Graph.hpp"
#include "Group.hpp"
#include "GroupRepresentationParser.hpp"
#include "VanKampenUtils.hpp"

class ProcessLogger
{
public:
private:
};

int main(int argc, const char **argv)
{
    try
    {
        std::string inputFileName, outputFileName, outputFileFormat = "dot", wordOutputFileName;
        std::size_t cellsLimit = 0;
        bool shuffleGroup = true;
        bool quiet = false;
        bool hasCellsLimit = false;

        {
            cxxopts::Options options("vankamp-vis", "Van Kampen diagram visualisation tool");

            options.add_options()(
                "i,input", "Specify input file", cxxopts::value(inputFileName), "required")(
                // "f,format", "Set file format", cxxopts::value(outputFileFormat)->default_value("dot"), "")(
                "o,output", "Specify custom output file", cxxopts::value(outputFileName)->default_value("vankamp-vis-out"), "")(
                "c,cycle-output", "Set boundary cycle output file", cxxopts::value(wordOutputFileName)->default_value("vankamp-vis-cycle.txt"), "")(
                "s,shuffle-group", "Shuffle group elements", cxxopts::value(shuffleGroup)->default_value("true"), "")(
                "q,quiet", "Do not log status to console", cxxopts::value(quiet)->default_value("false"), "")(
                "l,limit", "Set cells limit", cxxopts::value(cellsLimit), "integer")(
                "h,help", "Print usage");

            auto result = options.parse(argc, argv);

            if (result.count("help"))
            {
                std::cout << options.help() << std::endl;
                return 0;
            }
            if (!result.count("input"))
            {
                throw cxxopts::option_required_exception("input");
            }
            outputFileName += "." + outputFileFormat;
            hasCellsLimit = result.count("limit");
        }

        std::ifstream inputFile(inputFileName);

        if (!inputFile.good())
        {
            throw std::invalid_argument("cannot open '" + inputFileName + "'");
        }

        std::string text((std::istreambuf_iterator<char>(inputFile)),
                         std::istreambuf_iterator<char>());

        auto words = van_kampen::GroupRepresentationParser::parse(text);
        if (shuffleGroup)
        {
            std::random_shuffle(words.begin(), words.end());
        }

        van_kampen::Graph graph;
        van_kampen::Diagramm diagramm(graph);

        std::size_t totalIterations = words.size();
        if (hasCellsLimit)
        {
            totalIterations = std::min(totalIterations, cellsLimit);
        }
        van_kampen::ProcessLogger logger(totalIterations, std::clog, "Relations used");
        std::size_t addedWordsCount = 0;
        auto iterateOverWordsOnce = [&]() {
            for (const auto &word : words)
            {
                if (diagramm.bindWord(word))
                {
                    addedWordsCount += 1;
                    if (logger.iterate() >= totalIterations)
                    {
                        break;
                    }
                }
            }
        };

        iterateOverWordsOnce();
        iterateOverWordsOnce();

        {
            std::ofstream wordOutputFile(wordOutputFileName);
            if (!wordOutputFile.good())
            {
                std::cerr << "cannot write to file '" << wordOutputFileName << "'" << std::endl;
            }
            else
            {
                auto word = diagramm.getWord();
                for (std::size_t i = 0; i < word.size(); ++i)
                {
                    auto &letter = word[i];
                    wordOutputFile << letter.second.name << (letter.second.reversed ? "^(-1)" : "");
                    if (i < word.size() - 1)
                    {
                        wordOutputFile << "*";
                    }
                }
            }
        }

        diagramm.getTerminal()->setLabel("S");
        std::ofstream outFile(outputFileName);
        if (!outFile.good())
        {
            throw std::invalid_argument("cannot write to file '" + outputFileName + "'");
        }
        graph.printSelf(outFile);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 0;
    }
}
