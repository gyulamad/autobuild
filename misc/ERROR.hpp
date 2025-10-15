#pragma once

#include <string>
#include <stdexcept>
#include "F.hpp"
#include "fix_path.hpp"
#include "str_cut_end.hpp"

using namespace std;

#define FILELN (std::string("\n\tat ") + F_FILE_LINE(fix_path(__FILE__), __LINE__))
#define EMPTY_OR(s, ...) (F(F_DEBUG, ((s).empty() ? (std::string(__VA_ARGS__).empty() ? "<empty>" : std::string(__VA_ARGS__)) : (s).size() > 100 ? str_cut_end((s), 100) : (s))))
#define EWHAT (F(F_REASON, ", reason: ") + e.what())
#define ERROR(msg) runtime_error(F(F_ERROR, "\n\tError: ") + msg + FILELN) //"\nat " + F_FILE_LINE(fix_path(__FILE__), __LINE__))