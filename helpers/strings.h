#pragma once

#include <string>

namespace helpers {

    inline bool startsWith(std::string const & value, std::string const & prefix) {
        return value.find(prefix) == 0;
    }
    
    inline bool endsWith(std::string const & value, std::string const & ending) {
        if (ending.size() > value.size())
            return false;
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }

    
    
}
