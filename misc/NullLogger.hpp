#pragma once

#include "Logger.hpp"

class NullLogger: public Logger {
protected:
    inline void write(const string&) override {}
};