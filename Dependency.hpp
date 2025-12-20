#pragma once

#include "../cpptools/misc/__DIR__.hpp"
#include <vector>
#include <string>

using namespace std;

const string LIBS_DIR = fix_path(__DIR__ + "/../libs");

class Dependency {
public:
    Dependency() {}
    virtual ~Dependency() {}
    virtual void install(const string) = 0;
    virtual vector<string> flags() = 0;
    virtual vector<string> libs() = 0;
    virtual vector<string> incs() = 0;
};
