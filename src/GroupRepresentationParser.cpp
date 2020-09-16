#include "GroupRepresentationParser.hpp"

#include <unordered_map>

namespace van_kampmen
{
std::vector<std::string> split_by_delim(const std::string &word, const std::string &delim)
{
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = word.find(delim, prev);
        if (pos == std::string::npos)
        {
            pos = word.length();
        }
        std::string token = word.substr(prev, pos - prev);
        if (!token.empty())
        {
            tokens.push_back(std::move(token));
        }
        prev = pos + delim.length();
    } while (pos < word.length() && prev < word.length());
    return tokens;
}

std::vector<std::vector<GroupElement>> GroupRepresentationParser::parse(const std::string &repr)
{
    auto alphabetBegin = std::find(repr.begin(), repr.end(), '<');
    auto splitter = std::find(alphabetBegin, repr.end(), '|');
    auto alphabetEnd = std::find(splitter + 1, repr.end(), '>');

    if (alphabetEnd == repr.end())
    {
        throw std::invalid_argument("invalid representation format");
    }

    std::unordered_map<char, bool> isInAlphabet;

    for (auto c : split_by_delim({alphabetBegin + 1, splitter - 1}, ", "))
    {
        if (c.length() != 1 || !('a' <= c[0] && c[0] <= 'z'))
        {
            throw std::invalid_argument("variable name must contain one lowercase latin character");
        }
        isInAlphabet[c[0]] = true;
    }

    std::vector<std::vector<GroupElement>> result;

    for (auto word : split_by_delim({splitter + 2, alphabetEnd}, ", "))
    {
        std::vector<GroupElement> currentWord;
        for (std::size_t i = 0; i < word.length(); ++i)
        {
            if (!isInAlphabet[word[i]])
            {
                throw std::invalid_argument("word '" + word + "' contains character '" + word[i] + "' which is not in alphabet");
            }
            if (i + 1 < word.length() && word[i + 1] == '*')
            {
                currentWord.push_back({word[i], true});
                ++i;
            }
            else
            {
                currentWord.push_back({word[i], false});
            }
        }
        result.push_back(std::move(currentWord));
    }
    return result;
}
} // namespace van_kampmen
