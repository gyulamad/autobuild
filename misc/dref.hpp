#pragma once

#include "NULLCHK.hpp"

// Dereference to a specific type of pointer with nullptr safety check
template<typename T>
T& dref(void* ptr) {
    NULLCHK(ptr, "Null pointer dereference");
    T& ref = (*(T*)ptr);
    return ref;
}
