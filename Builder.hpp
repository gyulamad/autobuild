#pragma once

#include <string>
#include <vector>
#include "../cpptools/misc/datetime_defs.hpp"
#include "../cpptools/misc/get_cwd.hpp"
#include "../cpptools/misc/get_absolute_path.hpp"
#include "../cpptools/misc/implode.hpp"
#include "../cpptools/misc/Executor.hpp"
#include "../cpptools/misc/highlight_compiler_outputs.hpp"
#include "../cpptools/misc/file_exists.hpp"
#include "../cpptools/misc/file_get_contents.hpp"
#include "../cpptools/misc/explode.hpp"
#include "../cpptools/misc/filemtime_ms.hpp"
#include "../cpptools/misc/unlink.hpp"
#include "../cpptools/misc/in_array.hpp"
#include "../cpptools/misc/regx_match.hpp"
#include "../cpptools/misc/trim.hpp"
#include "../cpptools/misc/array_merge.hpp"
#include "../cpptools/misc/get_path.hpp"
#include "../cpptools/misc/get_extension_only.hpp"
#include "../cpptools/misc/str_contains.hpp"
#include "../cpptools/misc/is_dir.hpp"
#include "../cpptools/misc/mkdir.hpp"
#include "../cpptools/misc/file_put_contents.hpp"
#include "../cpptools/misc/replace_extension.hpp"
#include "../cpptools/misc/get_filename_ext.hpp"
#include "../cpptools/misc/EWHAT.hpp"
#include "../cpptools/misc/Logger.hpp"
#include <future>

using namespace std;

class Builder {
public:
    Builder(vector<string> modes, bool verbose): modes(modes), verbose(verbose) {}
    virtual ~Builder() {}

    void setVerbose(bool verbose) { this->verbose = verbose; }
    
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
        if (verbose) LOG("Building source file: " + F(F_FILE, sourceFile));
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
            // buildCmd("iwyu" + arguments, verbose); // TODO - make iwyu optional

            // all header files should starts with '#pragma once'
            string outputs, errors;
            Executor::execute(
                "find . -path \"./libs/*\" -prune -o "
                "\\( -name \"*.h\" -o -name \"*.hpp\" \\) "
                "! -name \"*.wrapper.h\" ! -name \"*.wrapper.hpp\" -print | "
                "while read f; do "
                    "head -n1 \"$f\" | grep -q \"#pragma once\" || echo \"$f\"; "
                "done",
                &outputs, &errors);
            if (!outputs.empty() || !errors.empty())
                throw ERROR(
                    "Some header files does not starts with '#pragma once':\n" 
                    + outputs + (errors.empty() ? "" : "\nErrors:\n" + errors));
        }
        buildCmd(GXX + arguments, verbose);
        
        // if (buildType == BT_PRECOMPILED_HEADER)
        //     file_copy(outputFile, get_path(sourceFile), true);
    }

    void buildCmd(const string& command, bool show = false) const {
        string outputs, errors;
        if (show) cout << command << endl;
        int err = Executor::execute(command, &outputs, &errors, false);
        // if (show || err) cout << command << endl;
        if (show && !outputs.empty()) cout << outputs << endl;
        if ((show || err) && !errors.empty()) cerr << highlight_compiler_outputs(errors) << endl;
        if (err)
            throw ERROR("Compile failed: " + to_string(err));
    }

    vector<future<void>> pchBuilderFutures; // TODO: make protected
    mutable std::mutex lastPchFMTimeMutex;  // mutable if used in const methods
    vector<vector<string>> getIncludesAndImplementationsAndDependencies(
        time_ms& lastfmtime,
        const string& basePath, 
        const string& sourceFile, 
        const string& buildPath,
        const vector<string>& flags,
        const vector<string>& includeDirs,
        vector<string>& foundImplementations,
        vector<string>& foundDependencies,
        vector<string>& visitedSourceFiles,
        const bool pch,
        const bool verbose, // TODO: fetch maxPchThreads from and optional command line arguments
        const unsigned int maxPchThreads = thread::hardware_concurrency() // controls PCH build parallelism, 0=auto, 1=sequential, N=limit to N
    ) {
        // waitFutures(pchBuilderFutures);
        // pchBuilderFutures.reserve(maxPchThreads);
        const string cacheFile = replaceToBuildPath(sourceFile, buildPath) + EXT_DEP;
        if (file_exists(cacheFile)) {
            if (verbose) LOG("Load dependencies from cache for " + F(F_FILE, sourceFile));
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
        if (verbose) LOG("Collecting dependencies for " + F(F_FILE, sourceFile));
        

        // NOTE: it may only a warning but I keep it as an error 
        // as I do not want recursive includes in my project 
        // and I want to detect it early when it happening
        if (!pch && in_array(sourceFile, visitedSourceFiles)) // TODO: It may not needed if pch because it we build them one by one...???
            throw ERROR("Source file already visited (possible include recursion?): " + F(F_FILE, sourceFile) + " - Note: In case you changed the #include dependencies cleanup your build folder.");
        // else // TODO throw ERROR ^^^^ Only if --no-pch
            
        visitedSourceFiles.push_back(sourceFile);

        if (verbose) LOG("Searching includes in " + F(F_FILE, sourceFile));

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
                    if (verbose) LOG("Dependency found: " + matches[0] + " in " + F_FILE_LINE(sourceFile, line));
                    const vector<string> splits = explode(",", matches[1]);
                    for (const string& split: splits) {
                        const string dependency = trim(split);
                        if (!in_array(dependency, dependencies)) {
                            dependencies.push_back(dependency);
                            foundDependencies = array_merge(foundDependencies, dependencies);
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

                    includes.push_back(include);
                    lastfmtime = max(lastfmtime, filemtime_ms(include));

                    // === PCH PRECOMPILATION LOGIC (using wrapper to avoid #pragma once warning) ===
                    // Only precompile actual headers (.h / .hpp), skip other files
                    // string ext = "." + get_extension_only(include);                    
                    if (pch && in_array("." + get_extension_only(include), EXTS_H_HPP)) {
                        string pchFile = getPchPath(include, buildPath);
                        string wrapperFile = getPchWrapperPath(include, buildPath);
                        string pchDir = get_path(pchFile);

                        // Rebuild if PCH missing, header newer, or wrapper newer than PCH
                        bool needsRebuild = !file_exists(pchFile) ||
                                            filemtime_ms(include) > filemtime_ms(pchFile) ||
                                            filemtime_ms(wrapperFile) > filemtime_ms(pchFile);

                        if (needsRebuild) {
                            if (verbose) LOG("Precompiling header (via wrapper): " + F(F_FILE, include) + " -> " + pchFile);
                            
                            if (!is_dir(pchDir)) {
                                if (!mkdir(pchDir, 0777, true))
                                    throw ERROR("Failed to create PCH directory: " + pchDir);
                            }

                            // Create wrapper if missing or header changed
                            if (!file_exists(wrapperFile) || filemtime_ms(include) > filemtime_ms(wrapperFile)) {
                                string wrapperContent = "#include \"" + include + "\"\n";
                                file_put_contents(wrapperFile, wrapperContent, false, true);
                            }

                            // Build command for PCH using the wrapper (no #pragma once → no warning)
                            string pchArgs =
                                " " + implode(" ", flags) + " " +
                                FLAG_INCLDIR + implode(" " + FLAG_INCLDIR, includeDirs) + " " +
                                "-x c++-header " + // TODO: once it's added to gcc use this instead wrapper files: -Wno-pragma-once-outside-header " +
                                wrapperFile + " " +
                                FLAG_OUTPUT + " " + pchFile;

                            if (pchBuilderFutures.size() >= maxPchThreads) waitFutures(pchBuilderFutures);
                            pchBuilderFutures.push_back(async(launch::async, [this, wrapperFile, pchArgs, verbose, &lastfmtime, pchFile]() {
                                buildCmd(GXX + pchArgs, verbose);
                                {
                                    lock_guard<mutex> lock(lastPchFMTimeMutex);
                                    lastfmtime = max(lastfmtime, filemtime_ms(pchFile));
                                }
                            }));
                            if (verbose) LOG("Building on thread(s): " + to_string(pchBuilderFutures.size()) + "...");
                            
                        } else if (verbose) {
                            LOG("Using cached PCH: " + pchFile);
                            lastfmtime = max(lastfmtime, filemtime_ms(pchFile));
                        }

                        // === END PCH LOGIC ===
                    }

                    vector<vector<string>> cache = 
                        getIncludesAndImplementationsAndDependencies(
                            lastfmtime, 
                            basePath, 
                            include, 
                            buildPath,
                            flags, 
                            includeDirs,
                            foundImplementations,
                            foundDependencies,
                            visitedSourceFiles,
                            pch,
                            verbose,
                            maxPchThreads
                        );
                    includes = array_merge(includes, cache[0]);

                    // look up for implementations...
                    implementations = lookupFileInIncludeDirs(
                        get_path(sourceFile), matches[1], 
                        includeDirs, true, false, EXTS_C_CPP
                    );
                    if (verbose) {
                        for (const string& implementation: implementations)
                            LOG("Implementation found: " + F(F_FILE, implementation));
                    }
                    foundImplementations = array_merge(
                        foundImplementations,
                        implementations
                    );
                }
            }
        } catch (exception &e) {
            waitFutures(pchBuilderFutures);
            throw ERROR("Include search failed at " 
                + F_FILE_LINE(sourceFile, line) + EWHAT);
        }
        const string cachePath = get_path(cacheFile);
        if (!is_dir(cachePath)) {
            if (file_exists(cachePath)) {
                waitFutures(pchBuilderFutures);
                throw ERROR("Cache path is not a folder: " + cachePath);
            }
            else if (!mkdir(cachePath, 0777, true)) {
                waitFutures(pchBuilderFutures);
                throw ERROR("Unable to create folder: " + cachePath);
            }
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
        waitFutures(pchBuilderFutures);
        return { includes, implementations, dependencies };
    }

    void waitFutures(vector<future<void>>& futures) {
        for (future<void>& f: futures) f.get();
        futures.clear();
    }

    string getPchPath(
        const string& headerFile,
        const string& buildPath
    ) const {
        // Mirrors header path into .build-<modes>/pch/ structure
        string relative = str_replace(DIR_BASE_PATH + "/", "", get_absolute_path(headerFile));
        return fix_path(buildPath + "/" + DIR_PCH_FOLDER + "/" + relative + EXT_GCH);
    }

    string getPchWrapperPath(const string& headerFile, const string& buildPath) const {
        // e.g. .build-debug/pch/core/utils.hpp.wrapper.hpp
        string relative = str_replace(DIR_BASE_PATH + "/", "", get_absolute_path(headerFile));
        return fix_path(buildPath + "/" + DIR_PCH_FOLDER + "/" + relative + ".wrapper.hpp");
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
                    get_absolute_path(fix_path(trim(includeDir) + "/" + fix_path(basePath + "/" + include)));
                if (file_exists(includePath)) {
                    results.push_back(includePath);
                    if (stopAtFirstFound) break;
                    continue;
                }
            }
            if (throwsIfNotFound && results.empty())
                throw ERROR("Include not found: " + include);
        }
        if (verbose && !results.empty()) {
            LOG("Implementation(s) found.");
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

    const string GXX = "ccache g++";

    // build folder
    const string DIR_BASE_PATH = get_absolute_path(get_cwd());
    const string DIR_BUILD_FOLDER = ".build";
    const string DIR_BUILD_PATH = fix_path(DIR_BASE_PATH + "/" + DIR_BUILD_FOLDER);
    const string DIR_DEPENDENCIES = "autobuild/dependencies";

    const string DIR_PCH_FOLDER = "";  // Subfolder for precompiled headers
    const string EXT_GCH = ".gch";        // Precompiled header extension

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

    vector<string> modes;
    bool verbose;
};