#include "ConsoleFlags.hpp"

namespace van_kampen
{
ConsoleFlags::ConsoleFlags(int argc, const char **argv)
{
    cxxopts::Options options("vankamp-vis", "Van Kampen diagram visualisation tool");
    options.add_options()(
        "i,input", "Specify input file", cxxopts::value(inputFileName), "(required)")(
        "o,output", "Specify output filename, '<input-filename>-diagram.dot' by default", cxxopts::value(outputFileName), "")(
        "c,circuit-output", "Set boundary circuit output file, '<input-filename>-circuit.txt' by default", cxxopts::value(wordOutputFileName), "")(
        "shuffle", "Shuffle representation before generation", cxxopts::value(shuffleGroup)->default_value("false"), "")(
        "not-sort", "Do not sort representation by relation legth before generation", cxxopts::value(notSort)->default_value("false"), "")(
        "q,quiet", "Do not log status to console", cxxopts::value(quiet)->default_value("false"), "")(
        "l,limit", "Set limit for used cells (valid for iterative and large-first)", cxxopts::value(cellsLimit), "")(
        "per-large", "Set the number of small words used to build one big one (valid for large-first)", cxxopts::value(perLarge)->default_value("10"), "")(
        "large-first", "Build diagramm with large-first algorithm", cxxopts::value(largeFirstAlgo))(
        "iterative", "Build diagramm with iterative algorithm", cxxopts::value(iterativeAlgo)->default_value("true"))(
        "merging", "Build diagramm with merging algorithm (not recommended)", cxxopts::value(mergingAlgo))(
        "s,split", "Split diagram in smaller components", cxxopts::value(split)->default_value("true"))(
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

    if (outputFileName.empty())
    {
        outputFileName = inputFileName + "-diagram.dot";
    }
    if (wordOutputFileName.empty())
    {
        wordOutputFileName = inputFileName + "-circuit.txt";
    }
}
} // namespace van_kampen
