#pragma once

#include <string>
#include <vector>

using namespace std;

class Dependency {
public:
    Dependency() {}
    virtual ~Dependency() {}
    virtual void install(const string) = 0;
    virtual vector<string> flags() = 0;
    virtual vector<string> libs() = 0;
};