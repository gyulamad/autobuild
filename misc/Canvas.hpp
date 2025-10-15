#pragma once

#include <string>
#include <functional>

using namespace std;

// Virtual Canvas interface
class Canvas {
public:
    Canvas() {}
    virtual ~Canvas() {}
    virtual void line(int left1, int top1, int left2, int top2, unsigned int color, int style = 0) = 0;
    virtual void circle(int left, int top, int radius, unsigned int color) = 0;
    virtual void circlef(int left, int top, int radius, unsigned int color) = 0;
    virtual void rect(int left, int top, int width, int height, unsigned int color) = 0;
    virtual void rectf(int left, int top, int width, int height, unsigned int color) = 0;
    virtual void text(int left, int top, const string& txt, unsigned int color, int font = 0, int size = 14) = 0;
    virtual void measure(const string& text, int& width, int& height, int& descent, int font = 0, int size = 14) = 0;
    virtual int width() = 0;
    virtual int height() = 0;
    virtual void clear() = 0;
    virtual void flush() {};

    function<void(int /*left*/, int /*top*/, int /*button*/)> push = nullptr;
    function<void(int /*left*/, int /*top*/, int /*button*/)> release = nullptr;
    function<void(int /*left*/, int /*top*/, int /*button*/)> drag = nullptr;
    function<void(int /*left*/, int /*top*/, int /*delta_x*/, int /*delta_y*/, int /*button*/)> scroll = nullptr;
    function<void(int /*left*/, int /*top*/)> move = nullptr;
    function<void(int /*left*/, int /*top*/)> leave = nullptr;
    function<void(int /*left*/, int /*top*/)> enter = nullptr;
};
