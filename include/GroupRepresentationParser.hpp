#pragma once

#include "Group.hpp"

namespace van_kampmen
{
    std::vector<std::string> split_by_delim(const std::string &word, const std::string &delim);

    class GroupRepresentationParser
    {
    public:
        // Builds vector of words from group representation
        // Format: <a, b, c, d | abc, a*b*c, bba>
        // x* is for inverse of x
        // All variables are lowercase latin characters
        static std::vector<std::vector<GroupElement>> parse(const std::string &);
    };
} // namespace van_kampmen
