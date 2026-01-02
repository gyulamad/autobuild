
#include "../../../../cpptools/misc/GithubDependency.hpp"

class IXWebSocketDependency: public GithubDependency {
public:
    using GithubDependency::GithubDependency;
    virtual ~IXWebSocketDependency() {}

    void setVersion(string VERSION) override {
        GithubDependency::setVersion(VERSION);
        GithubDependency::setRepo("machinezone/IXWebSocket");
    }


    vector<string> flags() override {
        return { };
    }

    vector<string> libs() override {
        const string path = getPath();
        return {
            path + "/build/libixwebsocket.a",
            "-lpthread", "-lssl", "-lcrypto", "-lz"
        };
    }

    vector<string> incs() override {
        const string path = getPath();
        return {
            path,
        };
    }
};

EXTERN_GITHUB_DEPENDENCY(IXWebSocketDependency)