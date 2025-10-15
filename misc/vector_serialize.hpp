#pragma once

#include <vector>

using namespace std;

// Serializes a vector of arbitrary types
template<typename S, typename T>
vector<S> vector_serialize(const vector<T>& vec) {
    vector<S> serialized;
    size_t size = vec.size();
    // Serialize size as 4 bytes (big-endian)
    serialized.push_back(static_cast<S>((size >> 24) & 0xFF));
    serialized.push_back(static_cast<S>((size >> 16) & 0xFF));
    serialized.push_back(static_cast<S>((size >> 8) & 0xFF));
    serialized.push_back(static_cast<S>(size & 0xFF));
    
    for (const T& item : vec) {
        serialized.push_back(static_cast<S>(item));
    }
    return serialized;
}
