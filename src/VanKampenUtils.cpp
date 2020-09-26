#include <stdexcept>

#include "VanKampenUtils.hpp"

namespace van_kampen
{
ProcessLogger::ProcessLogger(std::size_t total,
                             std::ostream &os,
                             const std::string &processName,
                             bool quiet)
    : total_(total),
      os_(os),
      processName_(processName),
      quiet_(quiet)
{
    log("started");
}

std::size_t ProcessLogger::iterate()
{
    if (currentIt_ >= total_)
    {
        return total_;
    }
    ++currentIt_;
    int percent = (static_cast<double>(currentIt_) / static_cast<double>(total_)) * 100.0;
    if (percent != lastResult)
    {
        lastResult = percent;
        log((percent < 10 ? " " : ""), percent, "%    ");
    }
    if (currentIt_ == total_)
    {
        log("finished  ");
    }
    return currentIt_;
}

std::size_t ProcessLogger::getIteration() const noexcept
{
    return currentIt_;
}

ProcessLogger::~ProcessLogger()
{
    log('\n');
}
} // namespace van_kampen
