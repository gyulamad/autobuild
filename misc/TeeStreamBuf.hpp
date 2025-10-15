#pragma once

#include <streambuf>

using namespace std;

class TeeStreamBuf: public streambuf {
private:
    streambuf* console; // Original console buffer
    streambuf* capture; // Capture buffer (stringstream)

protected:
    int overflow(int c) override {
        if (c != EOF) {
            // Write to both console and capture buffer
            if (console->sputc(c) == EOF || capture->sputc(c) == EOF) {
                return EOF;
            }
        }
        return c;
    }

    int sync() override {
        // Synchronize both buffers
        if (console->pubsync() == -1 || capture->pubsync() == -1) {
            return -1;
        }
        return 0;
    }

public:
    TeeStreamBuf(streambuf* console, streambuf* capture)
        : console(console), capture(capture) {}
};
