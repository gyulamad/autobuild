#include "../../../Dependency.hpp"
#include "../../../../cpptools/misc/EXTERN.hpp"
#include "../../../../cpptools/misc/file_exists.hpp"
#include "../../../../cpptools/misc/Executor.hpp"
#include "../../../../cpptools/misc/ConsoleLogger.hpp"

using namespace std;

class NloptDependency: public Dependency {
public:
    NloptDependency(): Dependency() {}
    virtual ~NloptDependency() {}

    void install(const string VERSION) override {
        createLogger<ConsoleLogger>();

        // Install and build nlopt with the selected version (if not already)
        const string TARGET = fix_path(LIBS_DIR + "/stevengj/nlopt/");
        const string BASE = fix_path(LIBS_DIR + "/..");
        const string nloptf = TARGET + "/" + VERSION + "/build/nlopt.hpp";
        LOG("Checking nlopt installed: " + nloptf);
        if (!file_exists(nloptf)) { // already instelled?
            LOG("=> not found, install nlopt...");
            const string cmd = __DIR__ + "/install.sh " + LIBS_DIR + "/.. " + TARGET + " " + VERSION;
            LOG("Execute: " + cmd);
            Executor::execute(cmd);
        }
    }

    vector<string> flags() override {
        return { };
    }

    vector<string> libs() override {
        return { };
    }

    vector<string> incs() override {
        return {
            "libs/stevengj/nlopt/v2.10.1/build",
            "libs/stevengj/nlopt/v2.10.1/src/api"
        };
    }
};

EXTERN(NloptDependency, (), ());