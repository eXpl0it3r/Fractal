#pragma once

#include <sstream>
#include <string>

namespace utility
{
    template<typename T>
    std::string toString(const T& value, bool fixed = true)
    {
        std::ostringstream out;
        out.precision(20);

        if(fixed)
            out << std::fixed << value;

        return out.str();
    }
} // namespace utility
