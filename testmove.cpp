// This is a tool for main projects and sub-projects:
// - Pragmatically founds *.hpp inline header files, 
// - Moves inline tests for the tests/ folder, 
// - Discovers test_*.hpp test files,
// - Creates/updates a tests/tests.cpp tests runner.

// Run it in the main projects folder.

// Build and run: 
//      ./builder autobuild/testmove.cpp --run

// Build and run the created tests/tests.cpp file(s):
//      ./builder tests/tests.cpp --mode=test --run

// Note: use --mode=debug or --mode=test,debug for gdb debug builds

// TODO: add this to the README

#include "../cpptools/misc/readdir.hpp"
#include "../cpptools/misc/Logger.hpp"
#include "../cpptools/misc/ConsoleLogger.hpp"
#include "../cpptools/misc/str_starts_with.hpp"
#include "../cpptools/misc/str_ends_with.hpp"
#include "../cpptools/misc/explode.hpp"
#include "../cpptools/misc/file_get_contents.hpp"
#include "../cpptools/misc/get_path.hpp"
#include "../cpptools/misc/get_filename.hpp"
#include "../cpptools/misc/file_put_contents.hpp"
#include "../cpptools/misc/tpl_replace.hpp"
#include "../cpptools/misc/is_dir.hpp"
#include "../cpptools/misc/file_exists.hpp"

int main() {
    createLogger<ConsoleLogger>();
    vector<string> hppFiles = readdir("./", "*.hpp", true, true);
    for (const string& hppFile: hppFiles) {
        if (str_starts_with(hppFile, "./autobuild")) continue;
        if (str_starts_with(hppFile, "./.build")) continue;
        if (str_ends_with(hppFile, ".wrapper.hpp")) continue;
        string hppFilename = get_filename(hppFile);
        if (str_starts_with(hppFilename, "test_")) continue;
        if (hppFilename == "TEST.hpp") continue;
        vector<string> codeSplit = explode("\n#ifdef TEST\n", file_get_contents(hppFile));
        if (codeSplit.size() != 2) continue;
        string testHppFile = get_path(hppFile) + "/tests/test_" + get_filename(hppFile);
        string testsCode = tpl_replace({
            { "{{hppFilename}}", hppFilename },
            { "{{testsCode}}", codeSplit[1] },

            // escape hack for builder
            { "{{#include}}", "#include" },
            { "{{#ifdef}}", "#ifdef" },

        }, string(R"(#pragma once

{{#include}} "../TEST.hpp"
{{#include}} "../{{hppFilename}}"

{{#ifdef}} TEST

{{testsCode}}
)")
        );
        LOG("Moving tests from " + F(F_FILE, hppFile) + " to " + F(F_FILE, testHppFile));
        if (file_exists(testHppFile))
            throw ERROR("Test file already exists: " + F(F_FILE, testHppFile));
        file_put_contents(testHppFile, testsCode, false, true); // create test file and move tests
        file_put_contents(hppFile, codeSplit[0], false, true);  // remove tests from source file
    }

    // test file discovery and rebuild test runners
    vector<string> allFiles = readdir("./", "", true, false);
    for (const string& testFolder: allFiles) {
        if (!is_dir(testFolder)) continue;
        if (!str_ends_with(testFolder, "/tests")) continue;
        LOG("Collecting test from " + F(F_FILE, testFolder));
        if (!file_exists(testFolder) || !is_dir(testFolder))
            throw ERROR("Test folder is not exists: " + F(F_FILE, testFolder));
        vector<string> testFiles = readdir(testFolder, "*.hpp");
        vector<string> testIncludes;
        for (const string& testFile: testFiles) {
            string testFilename = get_filename(testFile);
            if (!str_starts_with(testFilename, "test_")) continue;
            if (str_ends_with(testFilename, ".wrapper.hpp")) continue;
            testIncludes.push_back("#include \"" + testFilename + "\"");
        }
        string testRunnerCode = tpl_replace({
            { "{{testIncludes}}", implode("\n", testIncludes) },

            // escape hack for builder
            { "{{#include}}", "#include"}

        },
        string(R"(#include "../TEST.hpp"
{{#include}} "../ConsoleLogger.hpp"

{{testIncludes}}

int main() {
    createLogger<ConsoleLogger>();
    tester.run();
}
)")
        );
        string testRunnerFile = testFolder + "/tests.cpp";
        LOG("Creating tests runner: " + F(F_FILE, testRunnerFile));
        file_put_contents(testRunnerFile, testRunnerCode, false, true);
    }
}