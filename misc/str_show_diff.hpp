#pragma once

#include <iostream>

#include "F.hpp"

#include "str_diff_t.hpp"

using namespace std;

// Function to display a single diff block
string str_show_diff(const str_diff_t& diff, bool show = true) {
    string ret = to_string(diff.bound[0]) + ".." + to_string(diff.bound[1]) + ":\n";
    if (show) cout << "changed line(s) " << ret;
    for (const string& line: diff.added) {
        if (line.empty()) continue;
        string outp = F(F_GREEN, "+ " + line) + "\n";
        if (show) cout << outp;
        ret += outp;
    }
    for (const string& line: diff.removed) {
        if (line.empty()) continue;
        string outp = F(F_RED, "- " + line) + "\n";
        if (show) cout << outp;
        ret += outp;
    }
    return ret;
}
