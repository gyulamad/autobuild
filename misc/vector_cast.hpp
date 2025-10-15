#pragma once

#include <vector>

using namespace std;

template<typename Tto, typename Tfrom>
vector<Tto> vector_cast(const vector<Tfrom>& froms) {
    size_t size = froms.size();
    if (!size) return {};
    vector<Tto> tos(size);
    for (size_t i = 0; i < size; i++)
        tos[i] = froms[i];
    return tos;
}
