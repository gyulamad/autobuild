#pragma once

#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

#include "ERROR.hpp"

using namespace std;

class Executor {
public:
    Executor() {}
    virtual ~Executor() {}

    static int execute(const string& command, string* outputs = nullptr, string* errors = nullptr, bool throws = true) {
        // Create pipes for stdout and stderr
        int stdout_pipe[2];
        int stderr_pipe[2];
        string errstr;

        if (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) {
            errstr = "Failed to create pipe";
            if (errors) *errors = errstr;
            else cerr << errstr << endl;
            if (throws) throw ERROR("execute failed: " + command + "\n" + errstr);
            return -1;
        }

        pid_t pid = fork();
        if (pid == -1) {
            errstr = "Failed to fork process";
            if (errors) *errors = errstr;
            else cerr << errstr << endl;
            if (throws) throw ERROR("execute failed: " + command + "\n" + errstr);
            return -1;
        }

        if (pid == 0) { // Child process
            // Redirect stdout and stderr to pipes
            dup2(stdout_pipe[1], STDOUT_FILENO);
            dup2(stderr_pipe[1], STDERR_FILENO);
            
            // Close unused pipe ends
            close(stdout_pipe[0]);
            close(stdout_pipe[1]);
            close(stderr_pipe[0]);
            close(stderr_pipe[1]);

            // Execute the command
            execl("/bin/sh", "sh", "-c", command.c_str(), (char*)nullptr);
            
            // If execl fails
            if (throws)
                throw ERROR("execl failed");
            _exit(EXIT_FAILURE);
        } else { // Parent process
            // Close unused pipe ends
            close(stdout_pipe[1]);
            close(stderr_pipe[1]);

            // Read output and error from pipes
            array<char, 256> buffer;
            ssize_t count;
            
            if (outputs) outputs->clear();
            if (errors) errors->clear();
            
            // Read stdout
            while ((count = read(stdout_pipe[0], buffer.data(), buffer.size())) > 0) {
                if (outputs) {
                    outputs->append(buffer.data(), count);
                } else {
                    cout.write(buffer.data(), count);
                }
            }
            
            // Read stderr
            while ((count = read(stderr_pipe[0], buffer.data(), buffer.size())) > 0) {
                if (errors) {
                    errors->append(buffer.data(), count);
                } else {
                    cerr.write(buffer.data(), count);
                }
            }

            // Close pipe ends
            close(stdout_pipe[0]);
            close(stderr_pipe[0]);

            // Wait for child process to finish
            int status;
            waitpid(pid, &status, 0);
            
            int ret = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            if (throws && ret != 0)
                throw ERROR("Execute failed (" + to_string(ret) + "): " + command + "\n" + (errors ? *errors : "<cerr>"));
            return ret;
        }
    } 
};

#ifdef TEST

#include <fcntl.h>
#include "capture_cout.hpp"

// Test struct for complex test cases
struct test_Executor_TestData {
    string command;
    int expected_status;
    string expected_output_contains;
    string expected_error_contains;
};

TEST(test_Executor_successful_command_with_output_capture) {
    string output, error;
    int status = Executor::execute("echo 'Hello World'", &output, &error);
    
    assert(status == 0 && "Successful command should return 0");
    assert(output == "Hello World\n" && "Should capture stdout");
    assert(error.empty() && "Successful command should have empty stderr");
}

TEST(test_Executor_failing_command_with_error_capture) {
    string output, error;
    int status = Executor::execute("ls /nonexistent_directory_xyz", &output, &error, false);
    
    assert(status != 0 && "Failing command should return non-zero");
    assert(output.empty() && "Failing command should have empty stdout");
    assert(!error.empty() && "Should capture stderr");
    assert(str_contains(error, "No such file or directory") && "Error should contain expected message");
}

TEST(test_Executor_display_output_when_nullptr) {
    // This test visually verifies output is displayed when pointers are nullptr
    string output = capture_cout([]() {
        int status = Executor::execute("echo 'This should appear on console'", nullptr, nullptr);
        assert(status == 0 && "Should return success");
    });
    assert(str_contains(output, "This should appear on console") && "Output should appear");
    
    bool t = false;
    try {
        string out, err;
        Executor::execute("ls /nonexistent_directory_xyz", &out, &err, true);
    } catch (exception &e) {
        t = true;
        string errs = e.what();
        assert(str_contains(errs, "No such file or directory") && "Error massage should contains error infos");
    }
    assert(t && "Should throw");
}

TEST(test_Executor_mixed_output_and_error_capture) {
    string output, error;
    int status = Executor::execute("echo 'Output'; echo 'Error' >&2", &output, &error);
    
    assert(status == 0 && "Command should succeed");
    assert(output == "Output\n" && "Should capture stdout");
    assert(error == "Error\n" && "Should capture stderr");
}

TEST(test_Executor_empty_command) {
    test_Executor_TestData data = {
        "",  // Empty command
        0,   // Expected status (shell typically succeeds for empty command)
        "",  // Expected output
        ""   // Expected error
    };
    
    string output, error;
    int status = Executor::execute(data.command, &output, &error);
    
    assert(status == data.expected_status && "Empty command should return expected status");
    assert(output == data.expected_output_contains && "Empty command should produce no output");
    assert(error == data.expected_error_contains && "Empty command should produce no error");
}

TEST(test_Executor_invalid_command) {
    test_Executor_TestData data = {
        "this_command_does_not_exist_xyz123",
        127,  // Command not found typically returns 127
        "",    // No stdout expected
        "not found"  // Error should contain this
    };
    
    string output, error;
    int status = Executor::execute(data.command, &output, &error, false);
    
    assert(status == data.expected_status && "Invalid command should return expected status");
    assert(output == data.expected_output_contains && "Invalid command should produce no output");
    assert(str_contains(error, data.expected_error_contains) && 
        "Invalid command error should contain expected message");
}

TEST(test_Executor_command_with_spaces) {
    string output;
    int status = Executor::execute("echo \"Hello  World\"", &output, nullptr);
    
    assert(status == 0 && "Command with spaces should succeed");
    assert(output == "Hello  World\n" && "Should preserve spaces in output");
}

TEST(test_Executor_pipe_command) {
    string output;
    int status = Executor::execute("echo 'pipe test' | wc -c", &output, nullptr);
    
    assert(status == 0 && "Piped command should succeed");
    assert(str_contains(output, "10") && "Should get correct character count");
}


#endif
