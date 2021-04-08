#pragma once

#include <string>
#include <iostream>

#include "BoostCommon.hpp"

namespace logger
{
    void fail(const beast::error_code& ec, std::string message)
    {
        std::cerr << "Error on " << message << ": " << ec.message() << '\n';
    }
}