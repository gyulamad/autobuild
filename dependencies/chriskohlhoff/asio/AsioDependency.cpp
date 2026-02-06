#include "../../../../cpptools/misc/EXTERN.hpp"
#include "../../../../cpptools/misc/file_exists.hpp"
#include "../../../../cpptools/misc/Executor.hpp"
#include "../../../../cpptools/misc/ConsoleLogger.hpp"
#include "../../../../cpptools/misc/GithubDependency.hpp"
#include "../../../../cpptools/misc/get_absolute_path.hpp"

using namespace std;

class AsioDependency: public GithubDependency {
public:
    using GithubDependency::GithubDependency;
    virtual ~AsioDependency() {}

    void setVersion(string VERSION) override {
        GithubDependency::setVersion(VERSION);
        this->REPO = "chriskohlhoff/asio";
        this->TARGET = fix_path(LIBS_DIR + "/" + REPO + "/");
    }

    void install() override {
        createLogger<ConsoleLogger>();

        // Install and build json with the selected version (if not already)
        if(!installed()) { // already instelled?
            LOG("Install asio...");
            const string BASE = fix_path(LIBS_DIR + "/..");
            const string cmd = __DIR__ + "/install.sh " + REPO + " " + BASE + " " + TARGET + " " + VERSION;
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
        return { "-DASIO_STANDALONE" };
    }

    vector<string> libs() override {
        return { };
    }

    vector<string> incs() override {
        return {
            // get_absolute_path(get_cwd() + "/libs/" + REPO + "/" + VERSION + "/"),
            // get_absolute_path(get_cwd() + "/libs/" + REPO + "/" + VERSION + "/asio"),
            get_absolute_path(get_cwd() + "/libs/" + REPO + "/" + VERSION + "/asio/include"),
            // get_absolute_path(get_cwd() + "/libs/" + REPO + "/" + VERSION + "/asio/include/asio"),
            // get_absolute_path(get_cwd() + "/libs/" + REPO + "/" + VERSION + "/asio/include/asio/impl"),
            // get_absolute_path(get_cwd() + "/libs/" + REPO + "/" + VERSION + "/asio/include/asio")
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

EXTERN_GITHUB_DEPENDENCY(AsioDependency);