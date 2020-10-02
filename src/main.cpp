#include <cassert>
#include <random>
#include <sstream>

#include "cxxopts.hpp"

#include "Graph.hpp"
#include "Group.hpp"
#include "GroupRepresentationParser.hpp"
#include "VanKampenUtils.hpp"

struct ConsoleFlags
{
    ConsoleFlags() = default;

    ConsoleFlags(int argc, const char **argv)
    {
        cxxopts::Options options("vankamp-vis", "Van Kampen diagram visualisation tool");
        options.add_options()(
            "i,input", "Specify input file", cxxopts::value(inputFileName), "(required)")(
            "f,format", "Set file format", cxxopts::value(outputFormatString)->default_value("dot"), "(dot, nb)")(
            "o,output", "Specify output filename without exestention", cxxopts::value(outputFileName)->default_value("vankamp-vis-out"), "")(
            "c,cycle-output", "Set boundary cycle output file", cxxopts::value(wordOutputFileName)->default_value("vankamp-vis-cycle.txt"), "")(
            "n,no-shuffle", "Do not shuffle representation before generation", cxxopts::value(notShuffleGroup)->default_value("false"), "")(
            "q,quiet", "Do not log status to console", cxxopts::value(quiet)->default_value("false"), "")(
            "l,limit", "Set cells limit", cxxopts::value(cellsLimit), "integer")(
            "iterative", "Build diagramm with iterative algorithm", cxxopts::value(iterativeAlgo)->default_value("true"))(
            "merging", "Build diagramm with merging algorithm", cxxopts::value(mergingAlgo))(
            "h,help", "Print usage");

        auto result = options.parse(argc, argv);

        if (result.count("help"))
        {
            std::cout << options.help() << std::endl;
            exit(0);
        }
        if (!result.count("input"))
        {
            throw cxxopts::option_required_exception("input");
        }
        hasCellsLimit = result.count("limit");

        std::unordered_map<std::string, van_kampen::graphOutputFormat> formatByString = {
            {"dot", van_kampen::graphOutputFormat::DOT},
            {"nb", van_kampen::graphOutputFormat::WOLFRAM_NOTEBOOK},
        };

        outputFormat = formatByString[outputFormatString];
        if (!static_cast<int>(outputFormat))
        {
            throw std::invalid_argument("Undefined format: " + outputFormatString);
        }

        outputFileName += std::string(".") + outputFormatString;
    }

    std::string inputFileName, outputFileName, wordOutputFileName;
    std::size_t cellsLimit = 0;
    bool notShuffleGroup = false;
    bool quiet = false;
    bool hasCellsLimit = false;
    bool iterativeAlgo = true;
    bool mergingAlgo = false;
    std::string outputFormatString = "dot";
    van_kampen::graphOutputFormat outputFormat;
};

struct DiagrammGeneratingAlgorithm
{
    DiagrammGeneratingAlgorithm(van_kampen::Graph &graph)
        : diagramm_(graph) {}

    virtual void generate(const std::vector<std::vector<van_kampen::GroupElement>> &words) = 0;
    virtual van_kampen::Diagramm &diagramm()
    {
        return diagramm_;
    }

    virtual ~DiagrammGeneratingAlgorithm() = default;

protected:
    van_kampen::Diagramm diagramm_;
};

struct IterativeAlgorithm : DiagrammGeneratingAlgorithm
{
    IterativeAlgorithm(van_kampen::Graph &graph)
        : DiagrammGeneratingAlgorithm(graph) {}

    void generate(const std::vector<std::vector<van_kampen::GroupElement>> &words) override
    {
        std::size_t totalIterations = words.size();
        if (cellsLimit)
        {
            totalIterations = std::min(totalIterations, cellsLimit);
        }
        van_kampen::ProcessLogger logger(totalIterations, std::clog, "Relations used", quiet);
        std::size_t addedWordsCount = 0;
        auto iterateOverWordsOnce = [&]() {
            for (const auto &word : words)
            {
                if (diagramm_.bindWord(word))
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
    }

    std::size_t cellsLimit = 0;
    bool quiet = false;
};

struct MergingAlgorithm : DiagrammGeneratingAlgorithm
{
    MergingAlgorithm(van_kampen::Graph &graph)
        : DiagrammGeneratingAlgorithm(graph) {}

    void generate(const std::vector<std::vector<van_kampen::GroupElement>> &) override
    {
        throw std::logic_error("not implemented");
    }
};

int main(int argc, const char **argv)
{
    try
    {
        ConsoleFlags flags(argc, argv);
        std::ifstream inputFile(flags.inputFileName);
        if (!inputFile.good())
        {
            throw std::invalid_argument("cannot open '" + flags.inputFileName + "'");
        }
        std::string text((std::istreambuf_iterator<char>(inputFile)),
                         std::istreambuf_iterator<char>());

        std::vector<std::vector<van_kampen::GroupElement>> words = van_kampen::GroupRepresentationParser::parse(text);
        if (!flags.notShuffleGroup)
        {
            std::random_shuffle(words.begin(), words.end());
        }

        van_kampen::Graph graph;
        std::unique_ptr<DiagrammGeneratingAlgorithm> algo;

        if (flags.iterativeAlgo && flags.mergingAlgo)
        {
            throw std::invalid_argument("specify one algorithm");
        }
        if (flags.iterativeAlgo)
        {
            auto iterative = std::make_unique<IterativeAlgorithm>(graph);
            iterative->cellsLimit = flags.cellsLimit;
            iterative->quiet = flags.quiet;
            algo.reset(iterative.release());
        }
        else if (flags.mergingAlgo)
        {
            auto merging = std::make_unique<MergingAlgorithm>(graph);
            algo.reset(merging.release());
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

        graph.node(algo->diagramm().getTerminal()).setDiagramLabel("S");
        std::ofstream outFile(flags.outputFileName);
        if (!outFile.good())
        {
            throw std::invalid_argument("cannot write to file '" + flags.outputFileName + "'");
        }
        graph.printSelf(outFile, flags.outputFormat);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 0;
    }
}
