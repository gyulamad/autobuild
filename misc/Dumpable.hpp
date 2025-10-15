#pragma once

#include <string>

using namespace std;

class Dumpable {
public:
    Dumpable() {}
    virtual ~Dumpable() {}
    virtual string dump(bool show = false) const = 0;
};