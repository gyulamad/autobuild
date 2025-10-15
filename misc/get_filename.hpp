#pragma once

#include "get_filename_only.hpp"
#include "get_extension_only.hpp"

string get_filename(const string& path) {
    return get_filename_only(path) + "." + get_extension_only(path);
}