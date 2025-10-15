#pragma once

#include <string>

using namespace std;

class Stringable {
public:
    Stringable() {}
    virtual ~Stringable() {}
    virtual string toString() const = 0;
    virtual void fromString(const string&) = 0;
};
