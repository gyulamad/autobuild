#include "../misc/TEST.hpp"

#include "BuilderApp.hpp"

int main(int argc, char* argv[]) {
    createLogger<ConsoleLogger>();
    tester.run({});
    
    BuilderApp app;
    return app.run(argc, argv);
}
