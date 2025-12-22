#include "../../../Dependency.hpp"
#include "../../../../cpptools/misc/EXTERN.hpp"
#include <string>
#include <vector>

using namespace std;

class FltkDependency: public Dependency {
public:
    FltkDependency(const string version): Dependency("fltk/fltk", version) {}
    virtual ~FltkDependency() {}

    void install() override {
        // TODO cout << "[TODO] INSTALL and BUILD FLTK (if not already), version: " << version << endl;
    }

    bool installed() override {
        return true; // TODO... fake it until we make it!
    }

    vector<string> flags() override {
        return { };
    }

    vector<string> libs() override {
        return { "`fltk-config --cxxflags --ldflags`" };
    }

    vector<string> incs() override {
        return { };
    }
};

EXTERN(FltkDependency, (const string& version), (version));