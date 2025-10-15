#pragma once

#include <dlfcn.h>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "ERROR.hpp"
#include "get_absolute_path.hpp"
#include "Logger.hpp"
#include "F.hpp"
#include "replace_extension.hpp"
#include "remove_extension.hpp"
// #include "execute.hpp"
#include "file_exists.hpp"
#include "highlight_compiler_outputs.hpp"
#include "Executor.hpp"
#include "__DIR__.hpp"
#include "get_filename_only.hpp"
#include "get_extension_only.hpp"
#include "sort.hpp"

#include "../Builder.hpp"

using namespace std;

class DynLoader: public Builder {
public:
    // Constructor
    DynLoader(): Builder() {};

    // Destructor: Clean up all loaded objects and libraries
    virtual ~DynLoader() {
        // Destroy all loaded objects
        for (auto& objPair : objects)
            if (objPair.first && objPair.second)
                objPair.second(objPair.first);

        // Close all libraries
        for (auto& libPair : libraries)
            if (libPair.second.handle)
                dlclose(libPair.second.handle);
    }

    // Load a shared library and create an instance of type T with no arguments
    template <typename T, typename... Args>
    T* load(const string& path, Args&&... args) {
        void* handle = loadLibrary(path);
        
        // Select the appropriate CreateFunc type based on whether arguments are provided
        using CreateFunc = typename std::conditional<
            sizeof...(Args) == 0,
            T* (*)(),
            T* (*)(Args...)
        >::type;
        using DestroyFunc = void (*)(T*);
        
        CreateFunc create = nullptr;
        DestroyFunc destroy = nullptr;

        // Load symbols
        loadSymbols(path, handle, create, destroy);

        // Create the object based on whether arguments are provided
        T* obj;
        if constexpr (sizeof...(Args) == 0) {
            obj = create();
        } else {
            obj = create(std::forward<Args>(args)...);
        }
        
        if (!obj)
            throw ERROR("Failed to create object from library: " + path);

        // Store object and its destroy function
        objects.emplace_back(obj, reinterpret_cast<void (*)(void*)>(destroy));
        return obj;
    }

private:
    // vector<string> flags = { "--strict", "--fast" }; // TODO: pass the flags from the build with command line arguments -DXXXX constant maybe?

    // Structure to hold library handle and destroy function for each library
    struct LibInfo {
        void* handle;
        void (*destroy)(void*); // Generic destroy function, cast to specific type when needed
        LibInfo(void* h, void (*d)(void*)) : handle(h), destroy(d) {}
    };

    // Map of library path to its handle and destroy function
    unordered_map<string, LibInfo> libraries;
    // Vector to track loaded objects and their destroy functions
    vector<pair<void*, void (*)(void*)>> objects;


    // Private helper: Load a shared library or get existing handle
    void* loadLibrary(const string& path) {

        vector<string> modeFlags = { 
            "strict", "fast" 
            #ifdef DEBUG
                , "debug"
            #endif
        }; // TODO: to parameter
        sort(modeFlags);
            
        const string buildPath = getBuildFolder(
            DIR_BUILD_PATH, // TODO: to parameter
            modeFlags,
            SEP_MODES // TODO: to parameter
        ); // TODO: to parameter

        const bool verbose = true;

        string libPath = fix_path(replace_extension(path, ".so"));
        libPath = replaceToBuildPath(libPath, buildPath); // fixPath(path);
        
        auto it = libraries.find(libPath);
        if (it != libraries.end()) {
            return it->second.handle;
        }

        // rebuild only once before it really have to be loaded 
        // so that it does not take time to rebuild everything when debug builds
        
        // string cppPath = replace_extension(
        //     #ifdef DEBUG
        //         remove_extension(libPath)
        //     #else
        //         libPath
        //     #endif
        //         , ".cpp");

        if (!file_exists(libPath)) {

            string cppPath = replace_extension(path, ".cpp");
            if (!file_exists(cppPath)) 
                throw ERROR("Cound not rebuild shared library, file not found: " 
                    + F(F_FILE, cppPath));

            if (verbose) LOG("Build shared library: " + F(F_FILE, cppPath));

            // string cmd = "./build " + cppPath + " " + libPath + " --shared" +
            //     (flags.empty() ? "" : " " + implode(" ", flags)) 
            //     #ifdef DEBUG
            //         + " --debug"
            //     #endif
            //     + " 2>&1"
            // ;

            string cmd = "./autobuild " + cppPath // + " " + libPath 
                + (!modeFlags.empty() ? " --mode=" + implode(",", modeFlags) : "") // TODO: flags from parameter + rename and add to "modes"
                //+ (!buildPath.empty() ? " --build-folder=" + buildPath : "")
                + (verbose ? " --verbose" : "") // TODO: to parameter
                + " --shared"// 2>&1"
            ;

            string errs;
            if (verbose) LOG("Execute: " + cmd);
            int err = Executor::execute(cmd, nullptr, &errs, false);
            if (err)
                throw ERROR("Library build failed (" + to_string(err) + "):\n" 
                    + highlight_compiler_outputs("\nError:\n" + errs));
        }
         

        // Load the shared library
        if (verbose) LOG("Loading shared library: " + F(F_FILE, libPath));
        void* handle = dlopen(libPath.c_str(), RTLD_LAZY);
        if (!handle)
            throw ERROR("Failed to load library: " + string(dlerror()));
        
        return handle;
    }

    // Private helper: Load create and destroy symbols, store library info if needed
    template <typename T, typename... Args>
    void loadSymbols(
        const string& libPath, 
        void* handle, 
        T* (*&create)(Args...), 
        void (*&destroy)(T*)
    ) {
        // Reset dlerror
        dlerror();

        // Load the create function
        create = reinterpret_cast<T* (*)(Args...)>(dlsym(handle, "create"));
        const char* dlsym_error = dlerror();
        if (dlsym_error) {
            dlclose(handle);
            throw ERROR("Failed to load create function: " + string(dlsym_error));
        }

        // Load the destroy function
        destroy = reinterpret_cast<void (*)(T*)>(dlsym(handle, "destroy"));
        dlsym_error = dlerror();
        if (dlsym_error) {
            dlclose(handle);
            throw ERROR("Failed to load destroy function: " + string(dlsym_error));
        }

        // Store library info if not already present
        if (libraries.find(libPath) == libraries.end()) {
            libraries.emplace(libPath, LibInfo(handle, reinterpret_cast<void (*)(void*)>(destroy)));
        }
    }

//     string fixPath(const string& path) {
//         string folder = ".build";
// #ifdef DEBUG        
//         // return get_absolute_path(path) + ".gdb";
//         folder += "-debug";
// #else
//         return get_absolute_path(folder + "/" + path);
// #endif
//     }
};