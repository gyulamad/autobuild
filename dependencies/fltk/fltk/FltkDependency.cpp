
#include "../../../Dependency.hpp"
#include "../../../../cpptools/misc/EXTERN.hpp"

#include <iostream>
#include <vector>

using namespace std;

class FltkDependency: public Dependency {
public:
    FltkDependency(): Dependency() {}
    virtual ~FltkDependency() {}

    void install(const string version) override {
        // TODO cout << "[TODO] INSTALL and BUILD FLTK (if not already), version: " << version << endl;
    }

    vector<string> flags() override {
        return { };
    }

    vector<string> libs() override {
        return { "`fltk-config --cxxflags --ldflags`" };
    }
};

EXTERN(FltkDependency, (), ());