#include "../../../../misc/EXTERN.hpp"
#include "../../../Dependency.hpp"

#include <iostream>

using namespace std;

class FltkDependency: public Dependency {
public:
    FltkDependency(const string& version): Dependency(version) {}
    virtual ~FltkDependency() {}

    void install() override {
        cout << "INSTALL and BUILD FLTK (if not already), version: " << version << endl;
    }

};

EXTERN(FltkDependency, (const string& version), (version));