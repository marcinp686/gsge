#pragma once

#include <string>

namespace component
{

struct name
{
    name(std::string pName) : value(pName){};
    std::string value;
};

} // namespace component
