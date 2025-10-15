#pragma once

#include <vector>

using namespace std;

template<typename T>
class Serializable {
public:
    Serializable() {}
    virtual ~Serializable() {}
    virtual vector<T> serialize() = 0;
    virtual void deserialize(const vector<T>& serialized, size_t& nxt) = 0;
};
