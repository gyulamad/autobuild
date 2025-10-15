#pragma once

#include <string>
#include <functional>
#include <iostream>
#include <sstream>

#include "ERROR.hpp"
#include "TeeStreamBuf.hpp"

using namespace std;

string capture_cerr(function<void()> func, bool show = false) {
    streambuf* original_cerr_buffer = cerr.rdbuf();
    stringstream buffer;

    // Create TeeStreamBuf for cerr if show is true
    TeeStreamBuf tee_cerr(original_cerr_buffer, buffer.rdbuf());

    // Set up streams based on show parameter
    if (show) cerr.rdbuf(&tee_cerr);
    else cerr.rdbuf(buffer.rdbuf());

    string ewhat;
    try {
        func();
    } catch (exception& e) {
        ewhat = e.what();
    }
    cerr.rdbuf(original_cerr_buffer);
    cerr.clear();
    string output = buffer.str();
    if (!ewhat.empty()) 
        throw ERROR("Exception in stderr capture: " + ewhat + 
            (output.empty() ? "" : "\nOutput captured:\n" + output));
    return output;
}

#ifdef TEST

TEST(test_capture_cerr_basic) {
    string actual = capture_cerr([]() { cerr << "Error occurred!"; });
    assert(actual == "Error occurred!" && "capture_cerr: Basic cerr capture failed");
}

TEST(test_capture_cerr_empty) {
    string actual = capture_cerr([]() {});
    assert(actual.empty() && "capture_cerr: Empty output should return an empty string");
}

#endif
