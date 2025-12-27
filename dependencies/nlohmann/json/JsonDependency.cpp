#include "../../../../cpptools/misc/EXTERN.hpp"
#include "../../../../cpptools/misc/file_exists.hpp"
#include "../../../../cpptools/misc/GithubDependency.hpp"

using namespace std;

class JsonDependency: public GithubDependency {
public:
    using GithubDependency::GithubDependency;
    virtual ~JsonDependency() {}

    void setVersion(string VERSION) override {
        GithubDependency::setVersion(VERSION);
        GithubDependency::setRepo("nlohmann/json");
    }

    vector<string> flags() override {
        return { };
    }

    vector<string> libs() override {
        return { };
    }

    vector<string> incs() override {
        return {
            TARGET + "/" + VERSION + "/build",
            TARGET + "/" + VERSION + "/src/api"
        };
    }
};

EXTERN(JsonDependency, (), ());