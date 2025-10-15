#pragma once

#include "Logger.hpp"
#include "Arguments.hpp"
#include <functional>

template<typename L, typename A>
class App {
public:
    App() {}

    virtual ~App() {}

    int run(int argc, char* argv[]) {
        try {
            createLogger<L>();
            A args(argc, argv);
            process(args);
        } catch (exception &e) {
            LOG_ERROR("Exception" + EWHAT);
            return 1;
        }
        return 0;
    }

protected:
    virtual void process(A& args) = 0;
};
