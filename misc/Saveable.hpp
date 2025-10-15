#pragma once

#include <string>
#include "Filemode.hpp"

using namespace std;

class Saveable {
public:
    Saveable() {}
    virtual ~Saveable() {}
    virtual void save(const string& filename, Filemode mode = Filemode::UNDEF) = 0;
};
