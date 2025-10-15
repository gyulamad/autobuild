#pragma once

#include "ERROR.hpp"

// Null pointer check macros
#define NULLCHK_IMPL(ptr, errmsg) { \
    if (nullptr == ptr) \
        throw ERROR(errmsg); \
}

#define NULLCHK_SELECT(_1, _2, NAME, ...) NAME
#define NULLCHK_1(ptr) NULLCHK_IMPL(ptr, "nullptr: "#ptr)
#define NULLCHK_2(ptr, errmsg) NULLCHK_IMPL(ptr, errmsg)
#define NULLCHK(...) NULLCHK_SELECT(__VA_ARGS__, NULLCHK_2, NULLCHK_1)(__VA_ARGS__)