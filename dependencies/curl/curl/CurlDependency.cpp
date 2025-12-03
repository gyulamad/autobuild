
#include "../../../Dependency.hpp"
#include "../../../../cpptools/misc/EXTERN.hpp"

#include <iostream>
#include <vector>

using namespace std;

class CurlDependency: public Dependency {
public:
    CurlDependency(): Dependency() {}
    virtual ~CurlDependency() {}

    void install(const string version) override {
        cout << "INSTALL lib curl (if not already), version: " << version << endl;
    }

    vector<string> flags() override {
        return { "-Wl,--no-as-needed -lcurl" };
    }

};

EXTERN(CurlDependency, (), ());