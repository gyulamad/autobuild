#include "../../../../cpptools/misc/Dependency.hpp"
#include "../../../../cpptools/misc/EXTERN.hpp"
#include <vector>

using namespace std;

class CurlDependency: public Dependency {
public:
    CurlDependency(): Dependency() {}
    virtual ~CurlDependency() {}

    void install() override {
        // TODO cout << "INSTALL lib curl (if not already), version: " << version << endl;
    }

    bool installed() override {
        return true; // TODO
    }

    vector<string> flags() override {
        return { "-Wl,--no-as-needed" };
    }

    vector<string> libs() override {
        return { "-lcurl" };
    }

    vector<string> incs() override {
        return { };
    }

};

EXTERN(CurlDependency, (), ());