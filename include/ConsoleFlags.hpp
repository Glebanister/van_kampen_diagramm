#pragma once

#include <string>
#include <iostream>

#include "cxxopts.hpp"

#include "Graph.hpp"

namespace van_kampen
{
    struct ConsoleFlags
    {
        ConsoleFlags(int argc, const char **argv);

        std::string inputFileName, outputFileName, wordOutputFileName;
        std::size_t cellsLimit = 0;
        std::size_t perLarge = 0;
        bool shuffleGroup = false;
        bool quiet = false;
        bool hasCellsLimit = false;
        bool iterativeAlgo = true;
        bool mergingAlgo = false;
        bool largeFirstAlgo = false;
        bool notSort = false;
        std::string outputFormatString;
        van_kampen::graphOutputFormat outputFormat = van_kampen::graphOutputFormat::DOT;
    };
} // namespace van_kampen
