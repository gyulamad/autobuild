#pragma once

#include <vector>

using namespace std;

template<typename T, typename S>
void Serializable_vector_deserialize(T& v, vector<T>& vec, const vector<S>& serialized, size_t& nxt) {
    size_t size = serialized[nxt++];
    for (size_t n = 0; n < size; n++) {
        v.deserialize(serialized, nxt);
        vec.push_back(v);
    }
}
