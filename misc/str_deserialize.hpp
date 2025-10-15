#pragma once

#include <string>
#include <vector>
#include "cast.hpp"
#include "ERROR.hpp"

using namespace std;

template<typename S>
string str_deserialize(const vector<S>& serialized, size_t& nxt) {
    if (nxt >= serialized.size())
        throw ERROR("String serialized size mismatch");
    string str;
    // Deserialize size from 4 bytes
    if (nxt + 4 > serialized.size())
        throw ERROR("String serialized size mismatch");
    size_t size = (static_cast<size_t>(static_cast<unsigned char>(serialized[nxt])) << 24) |
                  (static_cast<size_t>(static_cast<unsigned char>(serialized[nxt + 1])) << 16) |
                  (static_cast<size_t>(static_cast<unsigned char>(serialized[nxt + 2])) << 8) |
                  static_cast<size_t>(static_cast<unsigned char>(serialized[nxt + 3]));
    nxt += 4;
    if (nxt + size > serialized.size())
        throw ERROR("String serialized size mismatch");
    for (size_t n = 0; n < size; n++)
        str += static_cast<char>(serialized[nxt++]);
    return str;
}
