#pragma once

#include "get_filename_only.hpp"
#include "get_extension_only.hpp"

string get_filename_ext(const string filepath) {    
    const string fname = get_filename_only(filepath);
    const string ext = get_extension_only(filepath);
    if (ext.empty()) return fname;
    if (ext == ".") return fname + ext;
    return fname + "." + ext;
}
