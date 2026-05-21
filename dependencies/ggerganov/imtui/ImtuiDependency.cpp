#include "../../../../cpptools/misc/GithubDependency.hpp"

using namespace std;

class ImtuiDependency: public GithubDependency {
public:
    using GithubDependency::GithubDependency;
    virtual ~ImtuiDependency() {}

    void setVersion(string VERSION) override {
        GithubDependency::setVersion(VERSION);
        GithubDependency::setRepo("ggerganov/imtui");
    }

    vector<string> flags() override {
        return {};
    }

    vector<string> libs() override {
        const string path = getPath();
        return {
            path + "/build/src/libimtui-ncurses.a",
            path + "/build/src/libimtui.a",
            path + "/build/third-party/libimgui-for-imtui.a",
            "-lncurses",
            "-ldl"
        };
    }

    vector<string> incs() override {
        const string path = getPath();
        return {
            path + "/include",
            path + "/third-party/imgui"
        };
    }
};

EXTERN_GITHUB_DEPENDENCY(ImtuiDependency)
