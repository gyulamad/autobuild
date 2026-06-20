#include "../../../../cpptools/misc/Dependency.hpp"
#include <string>
#include <vector>

using namespace std;

class FltkDependency: public Dependency {
public:
    FltkDependency(): Dependency() {}
    virtual ~FltkDependency() {}

    // LCOV_EXCL_START
    void install() override {
        
    }
    // LCOV_EXCL_STOP

    bool installed() override {
        return true; // TODO... fake it until we make it!
    }

    vector<string> flags() override {
        return { };
    }

    vector<string> libs() override {
        return { "-lncursesw" };
    }

    vector<string> incs() override {
        return { };
    }
};

EXTERN_DEPENDENCY(FltkDependency);