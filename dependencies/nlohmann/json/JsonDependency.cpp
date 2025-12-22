#include "../../../../cpptools/misc/EXTERN.hpp"
#include "../../../../cpptools/misc/file_exists.hpp"
#include "../../../../cpptools/misc/Executor.hpp"
#include "../../../../cpptools/misc/ConsoleLogger.hpp"
#include "../../../../cpptools/misc/GithubDependency.hpp"

using namespace std;

class JsonDependency: public GithubDependency {
public:
    using GithubDependency::GithubDependency;
    virtual ~JsonDependency() {}

    void setVersion(string VERSION) override {
        GithubDependency::setVersion(VERSION);
        this->REPO = "nlohmann/json";
        this->TARGET = fix_path(LIBS_DIR + "/" + REPO + "/");
    }

    void install() override {
        createLogger<ConsoleLogger>();

        // Install and build json with the selected version (if not already)
        if(!installed()) { // already instelled?
            LOG("Install json...");
            const string BASE = fix_path(LIBS_DIR + "/..");
            DBG(LIBS_DIR);
            const string cmd = __DIR__ + "/install.sh " + REPO + " " + BASE + " " + TARGET + " " + VERSION;
            DBG(REPO);
            DBG(BASE);
            DBG(TARGET);
            DBG(VERSION);
            LOG("Execute: " + cmd);
            Executor::execute(cmd);
        }
    }

    bool installed() override {
        const string jsonf = TARGET + "/" + VERSION;
        // LOG("Checking json installed: " + jsonf);
        return file_exists(jsonf);
    }

    vector<string> flags() override {
        return { };
    }

    vector<string> libs() override {
        return { };
    }

    vector<string> incs() override {
        return {
            // TARGET + "/" + VERSION + "/build",
            // TARGET + "/" + VERSION + "/src/api"
        };
    }

protected:

    void setRepo(string REPO) {
        this->REPO = REPO;
        this->TARGET = fix_path(LIBS_DIR + "/" + REPO + "/");
    }

    string REPO;
    string TARGET;
    string LIBS_DIR = "libs/";
};

EXTERN(JsonDependency, (), ());