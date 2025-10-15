#pragma once

#include <vector>

using namespace std;

// Deserializes a vector of arbitrary types
template<typename T, typename S>
vector<T> vector_deserialize(const vector<S>& serialized, size_t& nxt) {
    vector<T> vec;
    
    // Check if we have enough data for the size
    if (nxt + 4 > serialized.size()) {
        throw ERROR("Vector serialized size mismatch");
    }
    
    // Deserialize size from 4 bytes (big-endian)
    size_t size = (static_cast<size_t>(static_cast<unsigned char>(serialized[nxt])) << 24) |
                  (static_cast<size_t>(static_cast<unsigned char>(serialized[nxt + 1])) << 16) |
                  (static_cast<size_t>(static_cast<unsigned char>(serialized[nxt + 2])) << 8) |
                  static_cast<size_t>(static_cast<unsigned char>(serialized[nxt + 3]));
    nxt += 4;
    
    // Check if we have enough data for all elements
    if (nxt + (size * sizeof(T)) > serialized.size()) {
        throw ERROR("Vector serialized data mismatch");
    }
    
    for (size_t i = 0; i < size; i++) {
        vec.push_back(static_cast<T>(serialized[nxt++]));
    }
    
    return vec;
}
