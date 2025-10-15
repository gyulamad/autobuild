#pragma once

#include "misc/Logger.hpp"
#include "misc/Executor.hpp"
#include "misc/str_replace.hpp"
#include "misc/get_absolute_path.hpp"
#include "misc/implode.hpp"
#include "misc/explode.hpp"
#include "misc/filemtime_ms.hpp"
#include "misc/unlink.hpp"
#include "misc/regx_match.hpp"
#include "misc/array_merge.hpp"
#include "misc/is_dir.hpp"
#include "misc/mkdir.hpp"
#include "misc/highlight_compiler_outputs.hpp"
#include "misc/file_exists.hpp"
#include "misc/file_get_contents.hpp"
#include "misc/file_put_contents.hpp"
#include "misc/in_array.hpp"
#include "misc/trim.hpp"
#include "misc/get_path.hpp"
#include "misc/get_extension_only.hpp"
#include "misc/replace_extension.hpp"
#include "misc/get_cwd.hpp"
#include "misc/get_filename_ext.hpp"

#include <string>
#include <vector>

using namespace std;

class Builder {
public:
    Builder() {}
    virtual ~Builder() {}
    
protected:

    void buildSourceFile(
        const string& sourceFile,
        const string& outputFile, 
        const vector<string>& flags,
        const vector<string>& includeDirs,
        const vector<string>& linkObjectFiles,
        const vector<string>& libs,
        // BuildType buildType
        const bool strict,
        const bool verbose
    ) const {
        if (verbose) LOG("Build source file: " + sourceFile);
        // NOTE: known bug in gcc shows warning: #pragma once in main file
        // see: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=64117
        // const string wrapperFile = 
        //     get_path(outputFile) + "/" + get_filename(sourceFile) + ".wrp";
        // if (buildType == BT_PRECOMPILED_HEADER && !file_exists(wrapperFile))
        //     file_put_contents(
        //         wrapperFile,
        //         "#include \"" + sourceFile + "\"\n", 
        //         false, true
        //     );
        const string arguments = 
            (!flags.empty() ? " " + implode(" ", flags) : "") +
            (!includeDirs.empty() ? " " + FLAG_INCLDIR + implode(" " + FLAG_INCLDIR, includeDirs) : "") + 
            // + (buildType == BT_PRECOMPILED_HEADER ? " -x c++-header" : "")
            " " + FLAG_OUTPUT + " " + outputFile + " " +
            // + (buildType == BT_PRECOMPILED_HEADER ? wrapperFile : sourceFile);
            sourceFile + 
            (!linkObjectFiles.empty() ? " " + implode(" ", linkObjectFiles) : "") +
            (!libs.empty() ? " " + implode(" ", libs) : "");

        if (strict) {
            buildCmd("iwyu" + arguments);

            // all header files should starts with '#pragma once'
            string outputs, errors;
            Executor::execute("find . -path \"./libs/*\" -prune -o -name \"*.h\" -o -name \"*.hpp\" -print | while read file; do head -n 1 \"$file\" | grep -q \"#pragma once\" || echo \"$file\"; done", &outputs, &errors);
            if (!outputs.empty() || !errors.empty())
                throw ERROR(
                    "Some header files does not starts with '#pragma once':\n" 
                    + outputs + (errors.empty() ? "" : "\nErrors:\n" + errors));
        }
        buildCmd("g++" + arguments, verbose);
        
        // if (buildType == BT_PRECOMPILED_HEADER)
        //     file_copy(outputFile, get_path(sourceFile), true);
    }

    void buildCmd(const string& command, bool show = false) const {
        string outputs, errors;
        int err = Executor::execute(command, &outputs, &errors, false);
        if (show || err) cout << command << endl;
        if (show && !outputs.empty()) cout << outputs << endl;
        if ((show || err) && !errors.empty()) cerr << highlight_compiler_outputs(errors) << endl;
        if (err)
            throw ERROR("Compile failed: " + to_string(err));
    }

    vector<vector<string>> getIncludesAndImplementationsAndDependencies(
        time_ms& lastfmtime,
        const string& basePath, 
        const string& sourceFile, 
        const string& buildPath,
        const vector<string>& flags,
        const vector<string>& includeDirs,
        vector<string>& foundImplementations,
        vector<string>& visitedSourceFiles,
        const bool verbose
    ) const {
        // if (verbose) LOG("Collecting dependencies for " + F(F_FILE, sourceFile));
        const string cacheFile = replaceToBuildPath(sourceFile, buildPath) + EXT_DEP;
        if (file_exists(cacheFile)) {
            string cache = file_get_contents(cacheFile);
            if (cache.empty()) return {};

            vector<string> 
            splits =  explode(CACHE_SEP_IMP, cache);
            if (splits.size() != 2)
                throw ERROR("Invalid cache format: " + F(F_FILE, cacheFile) 
                    + " - missing marker: " + CACHE_SEP_IMP);
            vector<string> incs_and_imps_and_deps = splits;
            
            splits = explode(CACHE_SEP_DEP, incs_and_imps_and_deps[1]);
            if (splits.size() != 2)
                throw ERROR("Invalid cache format: " + F(F_FILE, cacheFile) 
                    + " - missing marker: " + CACHE_SEP_DEP);
            incs_and_imps_and_deps[1] = splits[0];
            incs_and_imps_and_deps.push_back(splits[1]);

            vector<string> includes = explode("\n", incs_and_imps_and_deps[0]);
            if (includes.size() == 1 && includes[0].empty()) includes = {};
            vector<string> implementations = explode("\n", incs_and_imps_and_deps[1]);
            if (implementations.size() == 1 && implementations[0].empty()) implementations = {};
            vector<string> dependencies = explode("\n", incs_and_imps_and_deps[2]);
            if (dependencies.size() == 1 && dependencies[0].empty()) dependencies = {};
            for (const string& include: includes)
                lastfmtime = max(lastfmtime, filemtime_ms(include));
            for (const string& implementation: implementations)
                lastfmtime = max(lastfmtime, filemtime_ms(implementation));
            if (filemtime_ms(cacheFile) < lastfmtime) {
                unlink(cacheFile);
                if (file_exists(cacheFile))
                    throw ERROR("Unable to delete: " + F(F_FILE, cacheFile));
            } else return { includes, implementations, dependencies };
        }

        // NODE: it may only a warning but I keep it as an error 
        // as I do not want recursive includes in my project 
        // and I want to detect it early when it happening
        if (in_array(sourceFile, visitedSourceFiles)) 
            throw ERROR("Source file already visited (possible include recursion?): " + sourceFile);
        visitedSourceFiles.push_back(sourceFile);

        if (verbose) LOG("Searching includes in " + sourceFile);

        string sourceCode = file_get_contents(sourceFile);
        vector<string> sourceLines = explode("\n", sourceCode);
        vector<string> matches;
        vector<string> includes;
        vector<string> implementations;
        vector<string> dependencies;
        int line = 0;
        try {
            for (const string& sourceLine: sourceLines) {
                line++; 
                if (regx_match(RGX_DEPENDENCY, sourceLine, &matches)) {
                    if (verbose) LOG("Dependency found: " + matches[0]);
                    const vector<string> splits = explode(",", matches[1]);
                    for (const string& split: splits) {
                        const string dependency = trim(split);
                        if (!in_array(dependency, dependencies)) {
                            dependencies.push_back(dependency);
                        }
                    }
                }
                if (regx_match(RGX_INCLUDE, sourceLine, &matches)) {
                    if (verbose) LOG("Include found: " + matches[0]);
                    const vector<string> foundIncludes = lookupFileInIncludeDirs(
                        get_path(sourceFile), matches[1], 
                        includeDirs, true, true
                    );
                    if (foundIncludes.empty())
                        throw ERROR("Include file not found: " + matches[0] 
                            + " at " + F_FILE_LINE(sourceFile, line));
                    if (foundIncludes.size() > 1)
                        throw ERROR("Multipe file found: " + matches[0]
                            + " at " + F_FILE_LINE(sourceFile, line));

                    const string include = foundIncludes[0];

                    if (in_array(include, includes)) 
                        continue;
                    lastfmtime = max(lastfmtime, filemtime_ms(include));
                    // if (verbose) LOG("Dependency found: " + F(F_FILE, include));
                    includes.push_back(include);
                    vector<vector<string>> cache = 
                        getIncludesAndImplementationsAndDependencies(
                            lastfmtime, 
                            basePath, 
                            include, 
                            buildPath,
                            flags, 
                            includeDirs,
                            foundImplementations,
                            visitedSourceFiles,
                            verbose
                        );
                    includes = array_merge(includes, cache[0]);

                    // look up for implementations...
                    implementations = lookupFileInIncludeDirs(
                        get_path(sourceFile), matches[1], 
                        includeDirs, true, false, EXTS_C_CPP
                    );
                    foundImplementations = array_merge(
                        foundImplementations,
                        implementations
                    );
                }
            }
        } catch (exception &e) {
            throw ERROR("Include search failed at " 
                + F_FILE_LINE(sourceFile, line) + EWHAT);
        }
        const string cachePath = get_path(cacheFile);
        if (!is_dir(cachePath)) {
            if (file_exists(cachePath))
                throw ERROR("Cache path is not a folder: " + cachePath);
            else if (!mkdir(cachePath, 0777, true))
                throw ERROR("Unable to create folder: " + cachePath);
        }
        file_put_contents(
            cacheFile, 
            (!includes.empty() ? implode("\n", includes) : "") + 
            CACHE_SEP_IMP + 
            (!implementations.empty() ? implode("\n", implementations) : "") +
            CACHE_SEP_DEP + 
            (!dependencies.empty() ? implode("\n", dependencies) : ""), 
            false, true
        );
        return { includes, implementations, dependencies };
    }

    vector<string> lookupFileInIncludeDirs(
        const string& basePath,
        const string& include, 
        const vector<string>& includeDirs,
        bool stopAtFirstFound,
        bool throwsIfNotFound,
        vector<string> extensions = {}
    ) const {
        vector<string> results;
        if (extensions.empty()) { 
            string extension = get_extension_only(include);
            extensions = extension.empty() ? EXTS_H_HPP : vector({ extension });
        }
        for (const string& extension: extensions) {
            string includePath = replace_extension(
                get_absolute_path(fix_path(basePath + "/" + include)), 
                extension
            );
            if (file_exists(includePath)) {
                results.push_back(includePath);
                if (stopAtFirstFound) break;
                continue;
            }
            for (const string& includeDir: includeDirs) {
                includePath = 
                    get_absolute_path(fix_path(trim(includeDir) + "/" + include));
                if (file_exists(includePath)) {
                    results.push_back(includePath);
                    if (stopAtFirstFound) break;
                    continue;
                }
            }
            if (throwsIfNotFound && results.empty())
                throw ERROR("Include not found: " + include);
        }
        return results;
    }

    string getBuildFolder(
        const string& buildPath,
        const vector<string>& modes,
        const string& sep
    ) const {
        return fix_path(
            buildPath + (!modes.empty() ? sep + implode(sep, modes) : "")
        );
    }

    string replaceToBuildPath(
        const string& sourceFile,
        string buildPath = ""
    ) const {
        const string sourceFileName = get_filename_ext(sourceFile);
        const string sourceFilePath = get_absolute_path(get_path(sourceFile));
        const string basePath = DIR_BASE_PATH;
        if (buildPath.empty()) buildPath = DIR_BUILD_PATH;
        return fix_path(str_replace(
            basePath, buildPath + "/", sourceFilePath
        ) + "/" + sourceFileName);
    }


    // build folder
    const string DIR_BASE_PATH = get_absolute_path(get_cwd());
    const string DIR_BUILD_FOLDER = ".build";
    const string DIR_BUILD_PATH = fix_path(DIR_BASE_PATH + "/" + DIR_BUILD_FOLDER);
    const string DIR_DEPENDENCIES = "utils/autobuild/dependencies";

    const string RGX_INCLUDE = "^\\s*#include\\s*\"([^\"]+)\"\\s*";
    const string RGX_DEPENDENCY = "^\\s*//\\s*DEPENDENCY\\s*:\\s*([^\"]+)\\s*";

    const string SEP_PRMS = ",";
    const string SEP_MODES = "-";
    const string SEP_DEPENDENCY_LIBRARY = "/";
    const string SEP_DEPENDENCY_VERSION = ":";

    const string DEFAULT_DEPENDENCY_CREATOR = "";
    const string DEFAULT_DEPENDENCY_LIBRARY = "";
    const string DEFAULT_DEPENDENCY_VERSION = "master";

    const string EXT_O = ".o";
    const string EXT_SO = ".so";
    const string EXT_DEP = ".dep";

    const vector<string> EXTS_H_HPP = { ".h", ".hpp" };
    const vector<string> EXTS_C_CPP = { ".c", ".cpp" };

    const vector<string> PTRN_EXTS_C_CPP = { "*.c", "*.cpp" };

    const string CACHE_SEP_IMP = "\n<=== [ INCS | IMPS ] ===>\n";
    const string CACHE_SEP_DEP = "\n<=== [ IMPS | DEPS ] ===>\n";


    const string FLAG_COMPILE = "-c";
    const string FLAG_LIBRARY = "-l";
    const string FLAG_INCLDIR = "-I";
    const string FLAG_OUTPUT = "-o";
};