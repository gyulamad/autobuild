#pragma once

#include <string>

using namespace std;

class Dependency {
public:
    Dependency(const string& version): version(version) {}
    virtual ~Dependency() {}
    virtual void install() = 0;
protected:
    string version;
};