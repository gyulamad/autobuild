(AI generated README file)

# Main features
*   Build and run any C++ project simply easily and fast (incremental/parallel build, caching objects etc.) 
*   Different build targets for release, debug, test, memory/thread safe mode, etc.

# 5 minutes tutorial

*   Download or clone the main repo into to your project root
*   Download or clone the cpptools-misc repo relative to this folder into `cpptools/misc` folder
*   Get the autobuild tool, run:
```
autobuild/get-builder.sh
```
*   Build and run your project:
```
./builder . --run
```
*   Clean project:
```
./builder --clean
```

# Autobuild Tool

The `autobuild` folder contains the source code for the builder tool. This tool is used to automatically build and test the project.

The folder contains the following files:

*   `autobuild.cpp`: The main source code file for the autobuild tool.
*   `Builder.hpp`: Header file defining the Builder class, responsible for managing the build process.
*   `BuilderApp.hpp`: Header file defining the BuilderApp class, the main application class for the autobuild tool.
*   `get-builder.sh`: A shell script that compiles the `autobuild.cpp` file.
*   `dependencies/`: This folder contains external dependencies required by the autobuild tool.

### Autobuild Usages

Autobuild is a tool that assists in the build process, runs tests, and can be integrated into automated workflows. It simplifies the development workflow and helps ensure consistency across builds.

#### Automating the Build Process

Autobuild can be integrated into automated systems to compile your project whenever changes are detected in the source code. However, it can also be run manually.

Example:

To automatically build the project, run:

```bash
./builder .
```

#### Running Tests

Autobuild can also be used to automatically run tests after each build. This helps to identify and fix bugs early in the development cycle.

Example:

To run tests after building the project, use the `--mode=test --run` flag:

```bash
./builder . --mode=test --run
```

#### Common Use Cases

*   Continuous Integration: Integrate autobuild into your CI/CD pipeline to automate builds, tests, and documentation generation.
*   Development Environment: Use autobuild to automatically build and test your project as you develop.
*   Release Management: Use autobuild to create consistent and reproducible builds for releases.

## Building the Autobuild Tool

The `get-builder.sh` script is a shell script that compiles the `autobuild.cpp` file using `g++` and creates an executable named `builder`. It uses the C++20 standard.

To use the script, run:

```bash
autobuild/get-builder.sh
```

**Note:** This script should be executed from the project root directory.

This will generate the `builder` executable in the same directory.

The `get-builder.sh` script simplifies the compilation process of the `autobuild.cpp` file. It uses `g++` with the `-std=c++20` flag to ensure that the code is compiled using the C++20 standard. This script is essential for building the `builder` executable, which is the main tool for automating builds, running tests, and other development tasks.

After running the script, the `builder` executable will be created in the same directory. You can then use this executable to perform various tasks, such as building your project, running tests, and generating reports.

## Usage [TODO: simplify the readme, tell to use the --help argument and then update the help messages with these:]

The `builder` executable accepts the following command-line parameters:

*   `--input` or `-i`: Input file or folder. Specifies the C++ file or folder to be compiled. (Note: if `--input` parameter is not provided, the first command line argument applies)
*   `--recursive` or `-r`: If the input is a folder, this flag enables recursive reading of the folder to find all C++ files.
*   `--mode` or `-m`: Build mode. Selects the compilation flags to use. Available modes are: debug, fast, test, strict, safe_memory, safe_thread, and coverage.
    *   `debug`: Enables debugging information (`-g`, `-DDEBUG`, `-fno-omit-frame-pointer`).
    *   `fast`: Optimizes for speed (`-pedantic-errors`, `-Werror`, `-Wall`, `-Wextra`, `-Wunused`, `-fno-elide-constructors`, `-Ofast`, `-fno-fast-math`).
    *   `test`: Enables testing mode (`-DTEST`).
    *   `strict`: Enables strict compilation flags (`-pedantic-errors`, `-Werror`, `-Wall`, `-Wextra`, `-Wunused`, `-fno-elide-constructors`).
    *   `safe_memory`: Enables memory safety checks (`-pedantic-errors`, `-Werror`, `-Wall`, `-Wextra`, `-Wunused`, `-fno-elide-constructors`, `-fsanitize-address-use-after-scope`, `-fsanitize=undefined`, `-fstack-protector`, `-fsanitize=address`, `-fsanitize=leak`).
    *   `safe_thread`: Enables thread safety checks (`-pedantic-errors`, `-Werror`, `-Wall`, `-Wextra`, `-Wunused`, `-fno-elide-constructors`, `-fsanitize-address-use-after-scope`, `-fsanitize=undefined`, `-fstack-protector`, `-fsanitize=thread`).
    *   `coverage`: Enables code coverage analysis (`-fprofile-arcs`, `-ftest-coverage`).
*   `--libs` or `-l`: Additional libraries to link. Specifies additional libraries to link with the compiled executable. Libraries should be separated by a comma (,).
*   `--build-folder` or `-o`: Output build folder. Specifies the directory where the compiled output will be placed.
*   `--include-dirs` or `-I`: Additional include directories. Specifies additional directories to search for include files. Directories should be separated by a comma (,).
*   `--run` or `-x`: Run the executable after building. If this flag is present, the compiled executable will be run after a successful build.
*   `--run-args` or `-xargs`: Arguments to pass to the executable when running. Specifies arguments to be passed to the executable when it is run.
*   `--shared` or `-s`: Build a shared library. If this flag is present, the tool will build a shared library instead of an executable.
*   `--verbose` or `-v`: Enable verbose output. If this flag is present, the tool will print more detailed information about the compilation process.

#### Running Tests

To run tests, use the `--mode=test --run` flag:

```bash
./builder . --mode=test --run
```

This will compile the project with the `test` build mode and run the resulting executable. The `test` build mode is typically configured to include additional testing code and assertions.

#### Coverage Report

To generate a coverage report, use the `--mode=coverage` flag:

```bash
./builder . --mode=test,coverage --run
```

This will compile the project with the `coverage` build mode, which enables code coverage analysis. After running the executable, a coverage report will be generated, which shows which parts of the code were executed during the test run.

## Dependency Management

A dependency manager helps organize and track external libraries required by your project. It simplifies the process of including and managing these libraries, ensuring that your project builds correctly and consistently.

The `dependencies` directory serves as a central location for storing and managing external libraries required by the autobuild tool. This directory helps to keep your project organized and ensures that all dependencies are easily accessible.

### Installing Dependencies

Once you have installed the `fltk` library, you can specify it as a dependency when building your project:

```bash
./builder main.cpp --libs=fltk
```

This will tell the autobuild tool to link with the `fltk` library. The specific flags used to link with `fltk` are defined in the `libArgs` map in `BuilderApp.hpp`:
[TODO] - it will be defined in the dependency classes!

```cpp
const unordered_map<string, string> libArgs = {
    { "fltk" , "`fltk-config --cxxflags --ldflags` -lfltk -lfltk_images" },
};
```

### Creating Custom Dependency Classes
[TODO] - this part is still in development, it need to be updated!

To create custom dependency classes, you need to define a class that inherits from the `Dependency` class. This class should implement the `install` method, which is responsible for installing the dependency.

The `BuilderApp` uses `DynLoader` to load dependency classes from the `dependencies` directory. The class name should follow the format `[LibraryName]Dependency`, and the file should be located in the `dependencies/[Creator]/[Library]/` directory.

For example, to create a custom dependency class for the `foo` library, you would create a file named `FooDependency.hpp` in the `dependencies/foo/foo/` directory:

```cpp
#pragma once

#include "Dependency.hpp"

class FooDependency : public Dependency {
public:
    void install(const std::string& version) override {
        // Install the foo library
    }
};
```

You can then specify dependencies in your source code using the `// DEPENDENCY: foo` comment for each.
Note: the `// DEPENDENCY:` comment should be always at the very top of the `.cpp` main files!

### Specifying Dependencies in Source Code

To specify dependencies in your source code, use comments to indicate which libraries are required. This helps to document the dependencies and makes it easier to understand the project's requirements.

Example:

```cpp
// DEPENDENCY: fltk
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
```

In this example, the comment `// DEPENDENCY: fltk` indicates that the code depends on the `fltk` library. This comment recommended to be placed before any `#include` directives or code that uses the library.

This will tell the autobuild tool to search for the dependency class in the `dependencies/` directory.

## Advanced Code Examples

Here are some more advanced examples of how to use the autobuild tool:

*   Build a shared library:

    ```bash
    ./builder mylib.cpp --shared
    ```

*   Run the executable after building:

    ```bash
    ./builder main.cpp --run
    ```

*   Compile the executable for debugging:

    ```bash
    ./builder main.cpp --mode=debug
    ```

*   Run the executable with arguments:

    ```bash
    ./builder main.cpp --run-args "arg1 arg2"
    ```

*   Generate a coverage report:

    ```bash
    ./builder main.cpp --mode=test,coverage --run
    ```

## Troubleshooting

If you encounter any issues while using the autobuild tool, here are some troubleshooting tips:

*   **Make sure that all dependencies are installed correctly.** Check the error messages to identify missing dependencies and install them using the appropriate package manager or by downloading the source code and building it manually.
*   **Check the command-line arguments to ensure that they are correct.** Verify that the input file or folder is specified correctly, the build mode is valid, and any additional libraries or include directories are specified correctly.
*   **Enable verbose output using the `--verbose` or `-v` flag to get more detailed information about the compilation process.** This can help you identify the source of the problem.
*   **Ensure that the `get-builder.sh` script is executed from the project root directory.** The script relies on relative paths and may not work correctly if executed from a different directory.
*   **If you are using the `coverage` mode, make sure that you have installed `lcov` and `genhtml`.** These tools are required to generate the coverage report.
*   **If you encounter errors related to missing include files, make sure that the include directories are specified correctly using the `--include-dirs` or `-I` flag.**

## Extending the Autobuild Tool

To extend the autobuild tool, you can add new features, build modes, or dependencies.

*   **To add a new build mode,** you need to define a new entry in the `modeFlags` map in `BuilderApp.hpp`. This entry should specify the compiler flags to use for the new build mode.
*   **To add a new dependency,** you need to create a custom dependency class as described above and place it in the `dependencies` directory. You also need to update the `libArgs` map in `BuilderApp.hpp` to specify the flags used to link with the new dependency. [TODO]: libArgs is deprecated and the dependecy classes will do this but as I am writing this it is still in development so this part should be updated!
*   **To add a new feature,** you can modify the `autobuild.cpp` file or create a new class that extends the `Builder` or `BuilderApp` class.

## Contributing

If you would like to contribute to the autobuild tool, please follow these guidelines:

*   Fork the repository on GitHub.
*   Create a new branch for your changes.
*   Make your changes and commit them with clear and concise commit messages.
*   Submit a pull request to the main repository.

[TODO: add `/misc` folder to the readme]