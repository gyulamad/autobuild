#include "../../../../cpptools/misc/GithubDependency.hpp"

using namespace std;

class CppTuiDependency: public GithubDependency {
public:
    using GithubDependency::GithubDependency;
    virtual ~CppTuiDependency() {}

    void setVersion(string VERSION) override {
        GithubDependency::setVersion(VERSION);
        GithubDependency::setRepo("gyulamad/cpp-tui");
    }

    vector<string> flags() override {
        return {};
    }

    vector<string> libs() override {
        // Header-only library, no libraries to link
        return {};
    }

    vector<string> incs() override {
        const string path = getPath();
        return {
            path
        };
    }
};

EXTERN_GITHUB_DEPENDENCY(CppTuiDependency)
