#include <cassert>
#include <random>
#include <set>
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
            "f,format", "Set file format", cxxopts::value(outputFormatString)->default_value("dot"), "(dot, nb, edges)")(
            "o,output", "Specify output filename without exestention", cxxopts::value(outputFileName)->default_value("vankamp-vis-out"), "")(
            "c,cycle-output", "Set boundary cycle output file", cxxopts::value(wordOutputFileName)->default_value("vankamp-vis-cycle.txt"), "")(
            "n,no-shuffle", "Do not shuffle representation before generation", cxxopts::value(notShuffleGroup)->default_value("false"), "")(
            "q,quiet", "Do not log status to console", cxxopts::value(quiet)->default_value("false"), "")(
            "l,limit", "Set cells limit", cxxopts::value(cellsLimit), "integer")(
            "iterative", "Build diagramm with iterative algorithm", cxxopts::value(iterativeAlgo))(
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
            {"edges", van_kampen::graphOutputFormat::TXT_EDGES},
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
    std::string outputFormatString;
    van_kampen::graphOutputFormat outputFormat;
};

struct DiagrammGeneratingAlgorithm
{
    virtual void generate(const std::vector<std::vector<van_kampen::GroupElement>> &words) = 0;
    virtual van_kampen::Diagramm &diagramm() = 0;
    virtual ~DiagrammGeneratingAlgorithm() = default;
    van_kampen::Graph &graph() { return *graph_; }

protected:
    std::shared_ptr<van_kampen::Graph> graph_ = std::make_shared<van_kampen::Graph>();
};

struct IterativeAlgorithm : DiagrammGeneratingAlgorithm
{
    IterativeAlgorithm()
        : diagramm_(graph_) {}

    void generate(const std::vector<std::vector<van_kampen::GroupElement>> &words_) override
    {
        auto words = words_;
        std::reverse(words.begin(), words.end());
        std::size_t totalIterations = words.size();
        if (cellsLimit)
        {
            totalIterations = std::min(totalIterations, cellsLimit);
        }
        std::vector<bool> isAdded(words.size());
        bool force = false;
        van_kampen::ProcessLogger logger(totalIterations, std::clog, "Relations used", quiet);
        diagramm_.bindWord(words.front());
        isAdded[0] = true;
        std::reverse(words.begin(), words.end());
        std::reverse(isAdded.begin(), isAdded.end());
        auto iterateOverWordsOnce = [&]() {
            for (std::size_t i = 0; i < words.size(); ++i)
            {
                if (isAdded[i])
                    continue;
                if (diagramm_.bindWord(words[i], force))
                {
                    isAdded[i] = true;
                    if (logger.iterate() >= totalIterations)
                    {
                        break;
                    }
                }
            }
        };
        iterateOverWordsOnce();
        force = true;
        iterateOverWordsOnce();
    }

    van_kampen::Diagramm &diagramm() override
    {
        return diagramm_;
    }

    std::size_t cellsLimit = 0;
    bool quiet = false;

private:
    van_kampen::Diagramm diagramm_;
};

struct LargeFirstAlgorithm : DiagrammGeneratingAlgorithm
{
    LargeFirstAlgorithm()
        : diagramm_(graph_) {}

    void generate(const std::vector<std::vector<van_kampen::GroupElement>> &words) override
    {
        std::size_t totalIterations = words.size();
        if (cellsLimit)
        {
            totalIterations = std::min(totalIterations, cellsLimit);
        }
        van_kampen::ProcessLogger logger(totalIterations, std::clog, "Relations used", quiet);

        std::vector<bool> isAdded(words.size());
        using iterator = std::vector<std::vector<van_kampen::GroupElement>>::const_iterator;
        auto added = [&](iterator it) {
            return isAdded[it - begin(words)];
        };
        bool oneAdded = false;
        bool force = false;
        auto add = [&](iterator it) {
            if (added(it))
            {
                throw std::logic_error("already added word");
            }
            if (diagramm_.bindWord(*it, force))
            {
                isAdded[it - begin(words)] = true;
                oneAdded = true;
                return true;
            }
            return false;
        };
        auto smallIt = words.begin();
        auto bigIt = prev(words.end());
        auto nextNotAdded = [&](iterator it) {
            while (isAdded[++it - begin(words)])
                ;
            return it;
        };
        auto prevNotAdded = [&](iterator it) {
            while (isAdded[--it - begin(words)])
                ;
            return it;
        };
        while (true)
        {
            int rest = maximalSmallForOneBig;
            bool infinite = rest == 0;
            bool success = true;
            while (!add(bigIt))
            {
                if (rest-- == 0 && !infinite)
                {
                    success = false;
                    break;
                }

                if (add(smallIt) && logger.iterate() >= totalIterations)
                    return;

                smallIt = nextNotAdded(smallIt);
            }

            if (success && logger.iterate() >= totalIterations)
                return;

            bigIt = prevNotAdded(bigIt);
            if (smallIt >= bigIt)
            {
                smallIt = nextNotAdded(begin(words));
                bigIt = prevNotAdded(words.end());
                if (!oneAdded)
                {
                    force = true;
                }
                oneAdded = false;
            }
        }
    }

    van_kampen::Diagramm &diagramm() override
    {
        return diagramm_;
    }

    std::size_t cellsLimit = 0;
    bool quiet = false;
    int maximalSmallForOneBig = 10;

private:
    van_kampen::Diagramm diagramm_;
};

struct MergingAlgorithm : DiagrammGeneratingAlgorithm
{
    MergingAlgorithm()
        : result_(graph_) {}

    void generate(const std::vector<std::vector<van_kampen::GroupElement>> &words) override
    {
        std::vector<van_kampen::Diagramm> diagrams;
        diagrams.reserve(words.size());
        int left = limit ? limit : words.size();
        for (auto &word : words)
        {
            if (!left--)
            {
                // TODO: Implement limitation
            }
            diagrams.push_back(van_kampen::Diagramm{graph_});
            diagrams.back().bindWord(word, false);
        }
        van_kampen::ProcessLogger log{diagrams.size(), std::cout, "Merging", quiet};
        while (diagrams.size() > 1)
        {
            std::vector<std::pair<van_kampen::Diagramm, std::vector<van_kampen::Transition>>> circuits;
            for (std::size_t i = 0; i < diagrams.size(); ++i)
            {
                circuits.push_back({diagrams[i], diagrams[i].getCircuit()});
            }
            auto compareCircuitPrefix = [circuits](std::size_t first,
                                                   std::size_t second) {
                std::size_t i = 0;
                for (; i < circuits[first].second.size() &&
                       i < circuits[second].second.size() &&
                       circuits[first].second[i].label == circuits[second].second[i].label;
                     ++i)
                    ;
                if (i == circuits[first].second.size())
                {
                    return true;
                }
                if (i == circuits[second].second.size())
                {
                    return false;
                }
                return circuits[first].second[i].label.name.front() < circuits[second].second[i].label.name.front();
            };
            std::vector<std::size_t> order(circuits.size());
            std::generate(order.begin(), order.end(), [&, x = 0]() mutable { return x++; });
            diagrams.clear();
            std::stable_sort(order.begin(), order.end(), compareCircuitPrefix);
            bool atleastOne = false;
            for (std::size_t i = 0; i + 1 < circuits.size(); i += 2)
            {
                auto &&cur = circuits[order[i]].first;
                auto &&next = circuits[order[i + 1]].first;
                if (!cur.merge(std::move(next)))
                {
                    diagrams.push_back(std::move(cur));
                    diagrams.push_back(std::move(next));
                }
                else
                {
                    log.iterate();
                    atleastOne = true;
                    diagrams.push_back(std::move(cur));
                }
            }
            if (!atleastOne)
            {
                throw std::logic_error("cannot build diagram");
            }
        }
        result_ = diagrams[0];
    }

    van_kampen::Diagramm &diagramm() override
    {
        return result_;
    }

    std::size_t limit = 0;
    bool quiet = false;
    van_kampen::Diagramm result_;
};

int main(int argc, const char **argv)
{
    try
    {
        // int argc_ = 7;
        // const char *argv_[] = {"./vankamp-vis", "-i", "../../LangToGroup/out.txt", "-l", "2000", "-o", "one-2000"};
        ConsoleFlags flags(argc, argv);
        std::ifstream inputFile(flags.inputFileName);
        if (!inputFile.good())
        {
            throw std::invalid_argument("cannot open '" + flags.inputFileName + "'");
        }
        std::string text((std::istreambuf_iterator<char>(inputFile)),
                         std::istreambuf_iterator<char>());

        std::vector<std::vector<van_kampen::GroupElement>> words = van_kampen::GroupRepresentationParser::parse(text);
        std::clog << "Total relations count: " << words.size() << std::endl;
        if (!flags.notShuffleGroup)
        {
            // std::random_shuffle(words.begin(), words.end());
            std::stable_sort(words.begin(),
                             words.end(),
                             [](const std::vector<van_kampen::GroupElement> &a, const std::vector<van_kampen::GroupElement> &b) {
                                 return a.size() < b.size();
                             });
        }

        std::unique_ptr<DiagrammGeneratingAlgorithm> algo;
        if (flags.iterativeAlgo && flags.mergingAlgo)
        {
            throw std::invalid_argument("specify one algorithm");
        }
        if (flags.iterativeAlgo || !(flags.iterativeAlgo || flags.mergingAlgo))
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

        algo->graph().node(algo->diagramm().getTerminal()).setDiagramLabel("S");
        algo->graph().node(algo->diagramm().getTerminal()).highlightNode(true);
        std::ofstream outFile(flags.outputFileName);
        if (!outFile.good())
        {
            throw std::invalid_argument("cannot write to file '" + flags.outputFileName + "'");
        }
        algo->graph().printSelf(outFile, flags.outputFormat);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 0;
    }
}
