#pragma once

namespace van_kampen
{
    namespace utility
    {
        // Print all arguments to ostream
        template <typename... ToPrint>
        void print(std::ostream &os, ToPrint &&... args)
        {
            (os << ... << args);
            os.flush();
        }

    } // namespace utility
} // namespace van_kampen
