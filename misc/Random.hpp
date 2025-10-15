#pragma once

#include <random>

using namespace std;

template<typename T = mt19937>
class Random {
    T* engine = nullptr;
public:
    Random() { this->init(); }
    explicit Random(unsigned int seed) { this->seed(seed); }
    virtual ~Random() {}

    T* getEngine() const { return engine; }
    
    void close() {
        delete engine;
        engine = nullptr;
    }

    void init() {
        if (engine) close();
        random_device rd; 
        engine = new T(rd());
    }

    void seed(unsigned int seed) {
        if (engine) close();
        engine = new T(seed);
    }

    template<typename R>
    R get(R min, R max) {
        if (!engine) init();
        
        if constexpr (is_floating_point<R>::value) {
            uniform_real_distribution<R> dis(min, max);
            return dis(*engine);
        }

        if constexpr (is_integral<R>::value) {
            uniform_int_distribution<R> dis(min, max);
            return dis(*engine);
        }

        throw ERROR("Invalid type");
    }

    string gets(size_t length, const string& chars) {
        string ret = "";
        size_t max = chars.size() - 1;
        for (size_t i = 0; i < length; i++)
            ret += chars[(size_t)get(0, (int)max)];
        return ret;
    }
};