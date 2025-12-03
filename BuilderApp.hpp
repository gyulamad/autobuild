#pragma once

#include "Builder.hpp"
#include "Dependency.hpp"
#include "../cpptools/misc/App.hpp"
#include "../cpptools/misc/ConsoleLogger.hpp"
#include "../cpptools/misc/Arguments.hpp"
#include "../cpptools/misc/DynLoader.hpp"
#include "../cpptools/misc/Stopper.hpp"
#include "../cpptools/misc/readdir.hpp"
#include "../cpptools/misc/array_key_exists.hpp"
#include "../cpptools/misc/sort.hpp"
#include "../cpptools/misc/vector_remove.hpp"
#include "../cpptools/misc/array_unique.hpp"
#include "../cpptools/misc/array_shift.hpp"
#include "../cpptools/misc/ucfirst.hpp"
#include "../cpptools/misc/in_array.hpp"
#include "../cpptools/misc/get_filename.hpp"
#include <thread>
#include <mutex>
#include <queue>

// TODO: add parallel builds flag
class BuilderApp: public Builder, public App<ConsoleLogger, Arguments> {
public:
    BuilderApp(): Builder(), App() {}
    virtual ~BuilderApp() {}

protected:

    // Command-line parameters
    const pair<string, string> PRM_INPUT = { "input", "i" };
    const pair<string, string> PRM_RECURSIVE = { "recursive", "r" };
    const pair<string, string> PRM_MODE = { "mode", "m" };
    const pair<string, string> PRM_LIBS = { "libs", "l" };
    const pair<string, string> PRM_BUILD_FOLDER = { "build-folder", "o" };
    const pair<string, string> PRM_INCLUDE_DIRS = { "include-dirs", "I" };
    const pair<string, string> PRM_ARGS = { "args", "a" };
    const pair<string, string> PRM_RUN = { "run", "x" };
    const pair<string, string> PRM_RUN_ARGS = { "run-args", "xargs" };
    const pair<string, string> PRM_SHARED = { "shared", "s" };
    const pair<string, string> PRM_VERBOSE = { "verbose", "v" };
    // TODO 
    const pair<string, string> PRM_PARALLEL { "parallel", "p"};
    const pair<string, string> PRM_CLEAN = { "clean", "c" };

    // "mode" argument selected compile flags
    const vector<string> FLAGS = { "--std=c++20" };
    const vector<string> FLAGS_TEST = { "-DTEST" };
    const vector<string> FLAGS_DEBUG = { "-g", "-DDEBUG", "-fno-omit-frame-pointer" };
    const vector<string> FLAGS_STRICT = { "-pedantic-errors", "-Werror", "-Wall", "-Wextra", "-Wunused", "-fno-elide-constructors" };
    const vector<string> FLAGS_FAST = array_merge(FLAGS_STRICT, { "-Ofast", "-fno-fast-math" });
    const vector<string> FLAGS_SAFE = array_merge(FLAGS_STRICT, { "-fsanitize-address-use-after-scope", "-fsanitize=undefined", "-fstack-protector" });
    const vector<string> FLAGS_SAFE_MEMORY = array_merge(FLAGS_SAFE, { "-fsanitize=address", "-fsanitize=leak" });
    const vector<string> FLAGS_SAFE_THREAD = array_merge(FLAGS_SAFE, { "-fsanitize=thread" });    
    const vector<string> FLAGS_COVERAGE = { "-fprofile-arcs", "-ftest-coverage" };

    // TODO: compile to .o files with -fPIC and link them into a .so file with -shared in separated step 
    // (for now it's ok as we don't have much shared lib files that uses non-header-only includes anyway)
    const vector<string> FLAGS_SHARED = { "-fPIC", "-shared" }; // used when --shared parameter added

    // "mode" argument possible values
    const string MODE_NONE = "";
    const string MODE_DEBUG = "debug";
    const string MODE_FAST = "fast";
    const string MODE_TEST = "test";
    const string MODE_STRICT = "strict";
    const string MODE_SAFE_MEMORY = "safe_memory";
    const string MODE_SAFE_THREAD = "safe_thread";
    const string MODE_COVERAGE = "coverage";

    const unordered_map<string, vector<string>> modeFlags = {
        // { "", array_merge(FLAGS, array_merge(FLAGS_FAST, FLAGS_SAFE_MEMORY)) },
        { MODE_NONE, array_merge(FLAGS, FLAGS_STRICT) },
        { MODE_DEBUG, array_merge(FLAGS, FLAGS_DEBUG) },
        { MODE_FAST, array_merge(FLAGS, FLAGS_FAST) },
        { MODE_TEST, array_merge(FLAGS, FLAGS_TEST) },
        { MODE_STRICT, array_merge(FLAGS, FLAGS_STRICT) },
        { MODE_SAFE_MEMORY, array_merge(FLAGS, FLAGS_SAFE_MEMORY) },
        { MODE_SAFE_THREAD, array_merge(FLAGS, FLAGS_SAFE_THREAD) },
        { MODE_COVERAGE, array_merge(FLAGS, FLAGS_COVERAGE) },
    };

    // "coverage" related settings
    const string COVERAGE_INFO_FILE = "coverage.info";
    const string COVERAGE_FOLDER = ".coverage";
    const bool COVERAGE_DARK_MODE = true;
    const string COVERAGE_BROWSER = "google-chrome"; // "brave-browser"

    // TODO: move this into dependencies
    // "libs" arguments are added with '-l...' flag but we can override it to simplify things
    const unordered_map<string, string> libArgs = {
        { "fltk" , "`fltk-config --cxxflags --ldflags` -lfltk -lfltk_images" },
    };


    // enum BuildType { BT_EXECUTABLE, BT_PRECOMPILED_HEADER };

    void process(Arguments& args) override {

        Arguments::Helps helps;

        args.setHelps(&helps);
        args.addHelp(0, PRM_INPUT.first, 
            "Input file or folder");
        args.addHelp(PRM_INPUT, 
            "Input file or folder");
        args.addHelp(PRM_RECURSIVE, 
            "If the input is a folder, then reads it recursively.");
        args.addHelp(PRM_MODE, 
            "Build mode (" 
                + MODE_DEBUG + ", " 
                + MODE_FAST + ", " 
                + MODE_TEST + ", " 
                + MODE_STRICT + ", " 
                + MODE_SAFE_MEMORY + ", " 
                + MODE_SAFE_THREAD + ", " 
                + MODE_COVERAGE 
                + ")");
        args.addHelp(PRM_LIBS, 
            "Additional libraries to link (separated by '" + SEP_PRMS + "')");
        args.addHelp(PRM_BUILD_FOLDER, 
            "Output build folder.");
        args.addHelp(PRM_INCLUDE_DIRS, 
            "Additional include directories (separated by '" + SEP_PRMS + "')");
        args.addHelp(PRM_ARGS,
            "Additional build parameters");
        args.addHelp(PRM_RUN, 
            "Run the executable after building.");
        args.addHelp(PRM_RUN_ARGS, 
            "Arguments to pass to the executable when running.");
        args.addHelp(PRM_SHARED, 
            "Build a shared library.");
        args.addHelp(PRM_VERBOSE,
            "Enable verbose output.");
        args.addHelp(PRM_CLEAN,
            "Clean the project from all generated files and folders.");

            
        Stopper stopper;

        // set "verbose" parameter (on/off) to see the full progress
        const bool verbose = args.has(PRM_VERBOSE);

        // "input" argument or the first parameter is to build
        // can be a .cpp file or an entire folder. 
        // If it's a folder it will look up all the *.cpp file
        // and build them one-by-one applying the same argument(s).
        const vector<string> inputs = explode(SEP_PRMS, args.has(PRM_INPUT) 
            ? args.get<string>(PRM_INPUT) 
            : args.get<string>(1));
        vector<string> cppFiles;
        for (const string& input: inputs)
            if (file_exists(input)) {
                if (!is_dir(input)) cppFiles.push_back(input);
                else cppFiles = array_merge(cppFiles, readdir(
                    input, PTRN_EXTS_C_CPP, args.has(PRM_RECURSIVE)
                ));
            }

        // "libs" parameter add-s lib with "-l" 
        // or lookup the "libArgs" and replace them.
        vector<string> libs;
        if (args.has(PRM_LIBS) && !args.get<string>(PRM_LIBS).empty()) {
            for (const string& lib: explode(SEP_PRMS, args.get<string>(PRM_LIBS)))
                if (array_key_exists(lib, libArgs)) libs.push_back(libArgs.at(lib));
                else libs.push_back(FLAG_LIBRARY + lib);
        }
        
        // When "shared" argument (yes/no) is set then compile a shared library
        const bool shared = args.has(PRM_SHARED);

        // "include-dirs" will add include directories using -I... flag
        const vector<string> includeDirs = args.has(PRM_INCLUDE_DIRS) ? 
            explode(SEP_PRMS, args.get<string>(PRM_INCLUDE_DIRS)) : 
            vector<string>({});
        
        // "mode" argument (multiple) selects the used compiler flags from "modeFlags" map
        vector<string> flags = shared ? array_merge(FLAGS, FLAGS_SHARED) : FLAGS;
        vector<string> modes;
        if (args.has(PRM_MODE) && !args.get<string>(PRM_MODE).empty()) {
            modes = explode(SEP_PRMS, args.get<string>(PRM_MODE));
            for (const string& mode: modes)
                if (array_key_exists(mode, modeFlags)) flags = array_merge(flags, modeFlags.at(mode));
                else 
                    throw ERROR("Mode flags are undefined: " + EMPTY_OR(mode));
        }
        sort(modes);

        // Additional parameters to build process
        if (args.has(PRM_ARGS))
            flags = array_merge(flags, explode(" ", args.get<string>(PRM_ARGS)));

        // "coverage" argument (on/off) creates coverage report
        const bool coverage = in_array(MODE_COVERAGE, modes);

        
        // "strict" argument (on/off) set the compilation to 
        // the most padentic and error sensitive way, plus using iwyu etc..
        const bool strict = modes.empty() || in_array(MODE_STRICT, modes);

        // "run" and "run-args" parameter runs the executable 
        // after successful compilation with the "run-args" specified 
        // command-line parameters (optional)
        const bool run = args.has(PRM_RUN) || args.has(PRM_RUN_ARGS);
        const string runArgs = args.has(PRM_RUN_ARGS) 
            ? args.get<string>(PRM_RUN_ARGS) : "";

        // TODO: maybe we can use {src} and {pwd} etc. template variables 
        // (using str_replace() helper) to set the path root 
        // instead using build folder for each .cpp separatelly 
        // (similarly for include-dirs?)
        const string buildPath = getBuildFolder(
            (args.has(PRM_BUILD_FOLDER) 
                ? get_absolute_path(
                    get_path(DIR_BUILD_PATH) + "/" + args.get<string>(PRM_BUILD_FOLDER)
                ) : DIR_BUILD_PATH),
            modes, SEP_MODES
        );
        // const string buildPath = fix_path(
        //     (args.has(PRM_BUILD_FOLDER) ? 
        //         args.get<string>(PRM_BUILD_FOLDER) : DIR_BUILD) 
        //     + (!modes.empty() ? SEP_MODES + implode(SEP_MODES, modes) : "")
        // );


        // ====== clean first if needed ======

        if (args.has(PRM_CLEAN)) {
            if (!inputs.empty() && !inputs[0].empty()) {
                vector<string> uniquePaths;
                for (const string& input : inputs) {
                    string path = get_path(input);
                    if (!in_array(path, uniquePaths))
                        uniquePaths.push_back(path);
                }
                for (const string& path : uniquePaths)
                    cleanProject(path);
            } else // If no input is given, clean the current working directory.
                cleanProject(get_cwd());
        }

        // ====== compilation starts at this point ======

        // TODO: we may want to compile the main .cpp file 
        // separatly from the linking so that it re-compiles 
        // only when it changed 
        // - but for now as I don't use much object files 
        // it's ok as it recompiles anyway...

        const string outputExtension = (shared ? EXT_SO : "");
        vector<string> allOutputFiles;
        // TODO: add parallel flag to command-line arguments
        const bool parallel = false; // default to sequential for now
        const vector<string> builtOutputFiles = buildCppFiles(allOutputFiles,
            buildPath, cppFiles, modes, flags, includeDirs, libs,
            outputExtension, parallel, strict, verbose
        );
        
        for (const string& builtOutputFile: builtOutputFiles) 
            if (verbose) LOG("(Re)built output file: " + builtOutputFile);

        if (verbose) LOG("Build proceed in " + stopper.toString());

        if (run) for (const string& outputFile: allOutputFiles) {
            string command = outputFile + (!runArgs.empty() ? " " + runArgs : "");
            if (verbose) LOG("Execute: " + command);
            Executor::execute(command);
            if (coverage) {
                if (verbose) LOG("Generating coverage report...");
                const string coverageInfoFilePath = 
                    fix_path(buildPath + "/" + COVERAGE_INFO_FILE);
                const string coverageOutputPath = 
                    fix_path(buildPath + "/" + COVERAGE_FOLDER);
                

                const string browseCoverageCommand = 
                    COVERAGE_BROWSER + " " + 
                    coverageOutputPath + "/index.html";
                const string createCoverageCommand = 
                    "lcov --no-external --directory . --capture --output-file " 
                        + coverageInfoFilePath + " && \\"
                    "php ../autobuild/lcov-fixer.php " 
                        + coverageInfoFilePath + " && \\"
                    "genhtml -s --demangle-cpp -o " 
                        + coverageOutputPath 
                        + (COVERAGE_DARK_MODE ? " --dark-mode " : " ") 
                        + coverageInfoFilePath + " && \\"
                    "lcov --summary " 
                        + coverageInfoFilePath + " && \\"
                    // "echo -e \"\\033[1;32mTests passed\\033[0m\" && \\"
                    "echo \"" 
                        + F(F_SUCCESS, "Coverage info generated") 
                        + ", run:\" && \\"
                    "echo \"" 
                        + F(F_HIGHLIGHT, browseCoverageCommand) + "\"";
                Executor::execute(createCoverageCommand);
                Executor::execute(browseCoverageCommand);
            }
        } else if (coverage) {
            LOG_INFO("Use --" + PRM_RUN.first + " or --" + PRM_RUN_ARGS.first 
                + " parameter to generate coverage report.");
        }
    }

    [[nodiscard]]
    vector<string> buildCppFiles(
        vector<string>& allOutputFiles,
        const string& buildPath,
        const vector<string>& cppFiles,
        const vector<string>& modes,
        const vector<string>& flags,
        const vector<string>& includeDirs,
        const vector<string>& libs,
        const string& outputExtension,
        bool parallel,
        bool strict,
        bool verbose
    ) {
        // Always use thread pool, set size to 1 when parallel=false
        const unsigned int numThreads = parallel ? thread::hardware_concurrency() : 1;
        
        vector<string> builtOutputFiles; // This will be the return value, built by the workers

        // Create a queue of files to process
        queue<string> fileQueue;
        for (const string& file : cppFiles) fileQueue.push(file);

        // Mutexes for synchronization
        mutex queueMutex;
        mutex outputMutex;
        mutex loaderMutex;

        // Vector to hold worker threads
        vector<thread> workers;
        // Vector to collect exceptions
        vector<exception_ptr> exceptions;

        // Worker function
        auto worker_func = [&]() {
            try {
                while (true) {
                    string cppFile;
                    {
                        lock_guard<mutex> lock(queueMutex);
                        if (fileQueue.empty()) return;
                        cppFile = fileQueue.front();
                        fileQueue.pop();
                    }

                    // Process one file
                    const string cppFilePath = get_absolute_path(get_path(cppFile)) + "/";
                    const string outputFile = remove_extension(
                        replaceToBuildPath(cppFile, buildPath)
                    ) + outputExtension;

                    time_ms lastfmtime = filemtime_ms(cppFile);
                    vector<string> foundImplementations;
                    vector<string> visitedSourceFiles;
                    vector<vector<string>> cache =
                        getIncludesAndImplementationsAndDependencies(
                            lastfmtime, cppFilePath, get_absolute_path(cppFile), buildPath,
                            flags, includeDirs, foundImplementations, visitedSourceFiles,
                            verbose
                        );
                    vector<string> includes = cache[0];
                    vector_remove(foundImplementations, cppFile);
                    foundImplementations = array_unique(foundImplementations);

                    vector<string> dependencies = cache[2];
                    vector<string> dependencyFlags;
                    vector<string> dependencyLibs;
                    for (const string& dependency: dependencies) {
                        string
                            creator = DEFAULT_DEPENDENCY_CREATOR,
                            library = DEFAULT_DEPENDENCY_LIBRARY,
                            version = DEFAULT_DEPENDENCY_VERSION;
                        const vector<string> splits =
                            explode(SEP_DEPENDENCY_VERSION, dependency);
                        vector<string> splits0 =
                            explode(SEP_DEPENDENCY_LIBRARY, splits[0]);
                        if (splits0.size() == 2) creator = array_shift(splits0);
                        library = splits0[0];
                        if (splits.size() == 2) version = splits[1];
                        if (creator == DEFAULT_DEPENDENCY_CREATOR) creator = library;
                        if (library == DEFAULT_DEPENDENCY_LIBRARY)
                            throw ERROR("Unnamed library in dependency: " + dependency);
                        const string libClassName = ucfirst(library) + "Dependency";
                        const string libPathName = /*__DIR__ + "/" +*/ DIR_DEPENDENCIES + "/"
                            + creator + "/" + library + "/" + libClassName;
                        if (verbose) {
                            lock_guard<mutex> outputLock(outputMutex);
                            LOG("Loading dependency: " + F(F_HIGHLIGHT, libClassName));
                        }
                        {
                            lock_guard<mutex> loaderLock(loaderMutex);
                            Dependency* dependency = loader.load<Dependency>(libPathName);
                            dependency->install(version);
                            dependencyFlags = array_merge(dependencyFlags, dependency->flags());
                            dependencyLibs = array_merge(dependencyLibs, dependency->libs());
                        }
                    }

                    vector<string> linkObjectFiles;
                    // Recursive call with parallel=false to avoid nested parallelism
                    vector<string> allflags = array_merge({ FLAG_COMPILE }, flags);
                    vector<string> builtObjectFiles = buildCppFiles(
                        linkObjectFiles, // allOutputFiles parameter
                        buildPath, foundImplementations, modes,
                        allflags, includeDirs, libs,
                        EXT_O, false, strict, verbose
                    );

                    bool built = false;
                    if (!builtObjectFiles.empty() ||
                        !file_exists(outputFile) || lastfmtime > filemtime_ms(outputFile)
                    ) {
                        this->buildSourceFile(
                            cppFile, outputFile, 
                            array_merge(flags, dependencyFlags), 
                            includeDirs, 
                            linkObjectFiles,
                            array_merge(libs, dependencyLibs),
                            strict, verbose
                        );
                        built = true;
                    }

                    // Update the output vectors
                    {
                        lock_guard<mutex> lock(outputMutex);
                        allOutputFiles.push_back(outputFile);
                        if (built) {
                            builtOutputFiles.push_back(outputFile);
                        }
                    }

                    // Log if verbose
                    // if (verbose && built) {
                    //     lock_guard<mutex> lock(outputMutex);
                    //     LOG("(Re)built output file: " + outputFile);
                    // }
                }
            } catch (...) {
                lock_guard<mutex> lock(outputMutex);
                exceptions.push_back(current_exception());
            }
        };

        // Create and start worker threads
        for (unsigned int i = 0; i < numThreads; i++) {
            workers.emplace_back(worker_func);
        }

        // Join worker threads
        for (auto& worker : workers) {
            if (worker.joinable()) worker.join();
        }

        // Propagate exceptions, if any
        if (!exceptions.empty()) {
            rethrow_exception(exceptions[0]);
        }

        return builtOutputFiles;
    }

    void cleanProject(const string& projectRoot) {
        LOG("Starting project cleanup in: " + F(F_FILE, projectRoot));

        // 1. Delete build directories first for efficiency.
        LOG("Removing build directories...");
        const vector<string> topLevelEntries = readdir(projectRoot, "", true, false);
        for (const string& entry : topLevelEntries) {
            if (!is_dir(entry)) continue;
            if (entry == ".git") continue;
            if (!str_starts_with(get_filename(entry), DIR_BUILD_FOLDER)) continue;
            LOG("Deleting directory: " + entry);
            Executor::execute("rm -rf \"" + entry + "\"");
        }

        // 2. Define all possible single-part artifact extensions (without the dot).
        // Use constants where available.
        vector<string> artifactExtensionParts = {
            // Remove leading dot for comparison
            string(EXT_O).erase(0, 1),
            string(EXT_SO).erase(0, 1),
            string(EXT_DEP).erase(0, 1),
            // Others are not defined as constants
            "test", "gdb", "cov", "gcda", "gcno"
        };

        // 3. Scan all remaining files recursively.
        LOG("Scanning for generated artifacts...");
        const vector<string> allFiles = readdir(projectRoot, "*.*", true);
        LOG("Found " + to_string(allFiles.size()) + " file(s) in project root: " 
            + F(F_FILE, projectRoot));
        for (const string& file : allFiles) { 
            if (is_dir(file)) continue; // Skip directories

            const string filename = get_filename(file);
            const string filenameWithoutExt = get_filename_only(file);
            const string extensionStr = get_extension_only(file);
            
            // Case 1: No extension - check for a source file.
            if (extensionStr.empty()) {
                bool sourceExists = false;
                for (const string& srcExt : EXTS_C_CPP) {
                    if (file_exists(get_path(file) + "/" + filenameWithoutExt + srcExt)) {
                        sourceExists = true;
                        break;
                    }
                }
                if (sourceExists) {
                    LOG("Deleting executable: " + file);
                    unlink(file);
                }
                continue;
            }

            // Case 2: Has an extension - check if ALL parts are artifact extensions.
            const vector<string> pathPieces = explode("/", filename);
            const vector<string> extensionPieces = explode(".", pathPieces[pathPieces.size() - 1]);
            int nth = 0;
            bool inArtifactExtensions = false;
            bool allPartsAreArtifacts = false;
            for (const string& extensionPiece: extensionPieces) {
                if (nth > 0) {
                    if (!inArtifactExtensions) {
                        if (in_array(extensionPieces[nth], artifactExtensionParts)) {
                            inArtifactExtensions = true;
                            allPartsAreArtifacts = true;
                        }
                    } else {
                        if (!in_array(extensionPieces[nth], artifactExtensionParts)) {
                            allPartsAreArtifacts = false;
                            break;
                        }
                    }
                }
                nth++;
            }
            // const vector<string> extensionPieces = explode(".", filename);
            // if (extensionPieces.size() > 1) {
            //     bool allPartsAreArtifacts = extensionPieces.size() > 2;
            //     for (size_t i = 2; i < extensionPieces.size(); ++i) {
            //         if (!in_array(extensionPieces[i], artifactExtensionParts)) {
            //             allPartsAreArtifacts = false;
            //             break;
            //         }
            //     }

            if (allPartsAreArtifacts) {
                LOG("Deleting artifact: " + file);
                unlink(file);
            }
            // }
        }

        LOG("Project cleanup finished.");
    }

    DynLoader loader;
};
