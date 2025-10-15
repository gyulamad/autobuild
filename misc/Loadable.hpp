#pragma once

#include <string>
#include "Filemode.hpp"

using namespace std;

class Loadable {
public:
    Loadable() {}
    virtual ~Loadable() {}
    virtual void load(const string& filename, Filemode mode = Filemode::UNDEF) = 0;
};
