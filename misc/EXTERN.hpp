#pragma once

/************ [ EXTERN macro] ************

// Example Usages:

// Class with default constructor
EXTERN(DefaultClass);

// Class with 1-arg constructor
EXTERN(OneArgClass, (int x), (x));

// Class with 2-arg constructor
EXTERN(TwoArgClass, (const char* s, double d), (s, d));

******************************************/

#include "safe.hpp"

#define EXTERN_IMPL(lib, argsdefs, argspass) \
extern "C" lib* create argsdefs { \
    return new lib argspass; \
} \
extern "C" void destroy(lib* obj) { \
    delete SAFE(obj); \
}

#define EXTERN_SELECT(_1, _2, _3, NAME, ...) NAME
#define EXTERN_1(ptr) EXTERN_IMPL(ptr, (), ())  // Default constructor
#define EXTERN_3(ptr, argsdefs, argspass) EXTERN_IMPL(ptr, argsdefs, argspass) // Custom constructor
#define EXTERN(...) EXTERN_SELECT(__VA_ARGS__, EXTERN_3, EXTERN_1)(__VA_ARGS__)