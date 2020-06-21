#include <string>
#include <iostream>

#include "logging.h"

namespace KiteConnect
{
    logging::logging()
    {
        _enableLogging = false;
    }
    void logging::Write(std::string log)
    {
        std::cout << log << std::endl;
    }

    void logging::SetLogging(bool enable)
    {
        _enableLogging = enable;
    }

}