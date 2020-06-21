#pragma once

#include <string>

namespace KiteConnect
{
    class logging
    {
        public:
        static void Write(std::string log);
        logging();
        void SetLogging(bool enable);

        private:
        bool _enableLogging;
    };
}