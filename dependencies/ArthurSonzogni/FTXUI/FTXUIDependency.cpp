#include "../../../../cpptools/misc/GithubDependency.hpp"

using namespace std;

class FTXUIDependency: public GithubDependency {
public:
    using GithubDependency::GithubDependency;
    virtual ~FTXUIDependency() {}

    void setVersion(string VERSION) override {
        GithubDependency::setVersion(VERSION);
        GithubDependency::setRepo("ArthurSonzogni/FTXUI");
    }

    vector<string> flags() override {
        return {};
    }

      vector<string> libs() override {
        const string path = getPath();
        return {
            path + "/build/libftxui-component.a",
            path + "/build/libftxui-dom.a",
            path + "/build/libftxui-screen.a"
        };
    }

    vector<string> incs() override {
        const string path = getPath();
        return {
            path + "/include"
        };
    }
};

EXTERN_GITHUB_DEPENDENCY(FTXUIDependency)
