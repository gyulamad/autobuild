#pragma once

#ifndef TEST
#ifdef TEST_SHOW_DETAILS
#define TEST
#endif
#endif

#ifdef TEST
#undef TEST

#include <iostream>
#include <map>

#include "ERROR.hpp"
#include "ConsoleLogger.hpp"
#include "F.hpp"
#include "str_contains.hpp"
#include "capture_cout_cerr.hpp"
#include "implode.hpp"
#include "explode.hpp"

using namespace std;

#define assert(expr) if (!(expr)) throw ERROR("Assert failed: " #expr)

typedef void (*Test)();

class Tester {
public:

    virtual ~Tester() {}
    
    void add(string info, Test test) {
        tests[info] = test;
    }

    void run(vector<string> filters = {}) {
        println("Test(s) are running...");
        #ifndef TEST_SHOW_DETAILS
        println("Details are hidden, to show details compile with -DTEST_SHOW_DETAILS");
        #endif

        int counter = 0;
        int errors = 0;
        int warnings = 0;
        int skipped = 0;
        string error_infos;
        string warning_infos;

        auto add_warning = [&warnings, &warning_infos](const string& msg) {
            warnings++;
            warning_infos += "\n" + F(F_WARNING, "Warning: ") + msg;
        };

        auto add_error = [&errors, &error_infos](const string& err) {
            errors++;  
            error_infos += "\n" + F(F_ERROR, "Failure: ") + err;
        };

        for (const pair<string, Test> test: tests) {            
            string info = test.first;
            Test func = test.second;
            print_nfo(info);
            try {
                counter++;
                bool skipp = filters.empty() ? false : true;
                for (const string& filter: filters) {
                    if (filter.empty() || str_contains(info, filter)) {
                        skipp = false;
                        break;
                    }
                }
                if (skipp) {
                    skipped++;
                    print_skp();
                    continue;
                } else {
                    string output = capture_cout_cerr([&func](){
                        func();
                    }, true);
                    if (!output.empty()) {
                        add_warning("Test outputs: " + info + "\nOutput captured:\n" + output);
                        print_wrn();
                        continue;
                    }
                }
            } catch (exception& e) {
                add_error(info + "\n" + e.what());
                // error_infos += info + "\n" + e.what() + "\n";
                // errors++;                
                print_err();
                continue;
            }
            print_ok();            
        }
        #ifndef TEST_SHOW_DETAILS
        println();
        #endif
        if (!filters.empty())
            add_warning("Test(s) were filtered by keyword(s): '" + F(F_BOLD, implode(",", filters)));

        if (!counter) {
            add_error("No test running!");
        }

        if (warnings) cout << F(F_WARNING, to_string(warnings) + " warning(s) during testing of " + to_string(counter) + " test(s):") << warning_infos << endl;
        if (skipped) cout << F(F_WARNING, to_string(skipped) + " of " + to_string(counter) + " test(s) skipped.") << endl;
        if (errors) cout << F(F_ERROR, "Failed " + to_string(errors) + " of " + to_string(counter) + " test(s).") << endl << error_infos << endl;
        int ret = warnings + skipped + errors;
        if (!ret) cout << F(F_SUCCESS, "All (" + to_string(counter) + ") test(s) passed.") << endl;
        exit(ret);
    }
private:
    // bool capture;
    map<string, Test> tests;

    void print(const string& output) {
        cout << output << flush;
    }

    void println(const string& output = "") {
        cout << output << endl;
    }

    void print_nfo(
        const string&
        #ifdef TEST_SHOW_DETAILS
        info
        #endif
    ) {
        #ifdef TEST_SHOW_DETAILS
        print("[ ] " + info);
        #else
        //print("?\b");
        #endif
    }

    void print_skp() {
        #ifdef TEST_SHOW_DETAILS
        println("\r[" + F(F_WARNING, "!"));
        #else
        // print(F(F_WARNING, "!"));
        #endif
    }

    void print_wrn() {
        #ifdef TEST_SHOW_DETAILS
        println("\r[" + F(F_WARNING, "!"));
        #else
        print(F(F_WARNING, "!"));
        #endif
    }

    void print_err() {
        #ifdef TEST_SHOW_DETAILS
        println("\r[" + F(F_ERROR, "x"));
        #else
        print(F(F_ERROR, "x"));
        #endif
    }

    void print_ok() {
        #ifdef TEST_SHOW_DETAILS
        println("\r[" + F(F_SUCCESS, "✓"));
        #else
        print(F(F_SUCCESS, "."));
        #endif
    }

} tester;

// tester.add(string(#name) + " at " + __FILE__ + ":" + to_string(__LINE__), name);
#define TEST(name) \
void name(); \
struct struct_of_##name { \
    struct_of_##name() { \
        createLogger<ConsoleLogger>(); \
        tester.add(F_CALL(string(#name), __FILE__, __LINE__), name); \
    } \
} instance_of_##name; \
void name()

#else // TEST

#include <string>
#include <vector>
#include "ERROR.hpp"

using namespace std;

struct { 
    void run(vector<string> filters = {}) {
        if (filters.empty()) return;
        throw ERROR("Filter(s) left for tests runner");
    }
} tester;

#endif // TEST
