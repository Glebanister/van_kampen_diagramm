#include "GroupRepresentationParser.hpp"

#include <unordered_map>

namespace van_kampen
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

std::vector<std::vector<GroupElement>> GroupRepresentationParser::parse(const std::string &text)
{
    auto withoutBorders = [](const std::string &s) {
        if (s.length() < 2)
        {
            throw std::invalid_argument("can not parse pattern '" + s + "'");
        }
        return s.substr(1, s.length() - 2);
    };

    auto alphabetBegin = text.begin() + text.find("FreeGroup( ") + std::string("FreeGroup( ").length();
    auto alphabetEnd = text.begin() + text.find(" );", std::distance(text.begin(), alphabetBegin));

    auto wordsBegin = text.begin() + text.find("[ ", std::distance(text.begin(), alphabetEnd)) + std::string("[ ").length();
    auto wordsEnd = text.begin() + text.find(" ]", std::distance(text.begin(), wordsBegin));

    if (!(text.begin() <= wordsEnd && wordsEnd <= text.end()))
    {
        throw std::invalid_argument("invalid group representation format");
    }

    std::vector<std::vector<van_kampen::GroupElement>> words;

    for (auto word : van_kampen::split_by_delim({wordsBegin, wordsEnd}, ", "))
    {
        std::vector<van_kampen::GroupElement> curWord;
        for (auto c : van_kampen::split_by_delim(word, "*"))
        {
            auto powChar = c.find('^') + c.begin();
            auto element = c.substr(0, powChar - c.begin());
            if (element[0] == '(')
            {
                element = withoutBorders(element);
            }
            bool reversed = element.size() != c.size();
            curWord.emplace_back(GroupElement{std::move(element), reversed});
        }
        words.emplace_back(std::move(curWord));
    }

    return words;
}
} // namespace van_kampen
