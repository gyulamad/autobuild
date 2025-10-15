#pragma once

#include <string.h>

// This function is SAFE and does not violate strict aliasing.
template<typename Tt, typename Tf>
inline Tt cast(Tf data1) { 
    static_assert(sizeof(Tf) <= sizeof(Tt), "Type length mismatch");
    Tt data2 = Tt(0);
    memcpy(&data2, &data1, sizeof(Tf));
    return data2;
}
