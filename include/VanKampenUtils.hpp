#pragma once

#include <ostream>
#include <string>
#include <cmath>

namespace van_kampen
{
    class ProcessLogger
    {
    public:
        ProcessLogger(std::size_t total,
                      std::ostream &os,
                      const std::string &processName = "Process",
                      bool quiet = false);
        std::size_t iterate();
        std::size_t getIteration() const noexcept;
        ~ProcessLogger();

    private:
        template <typename... Args>
        void log(Args... args)
        {
            if (quiet_)
            {
                return;
            }
            os_ << '\r';
            os_ << processName_ << ": ";
            (os_ << ' ' << ... << args);
        }

        std::size_t total_;
        std::size_t currentIt_ = 0;
        std::ostream &os_;
        int lastResult = -1;
        std::string processName_;
        bool quiet_;
    };
} // namespace van_kampen
