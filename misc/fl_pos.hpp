#pragma once

#include <FL/Fl_Widget.H>

#include <vector>

using namespace std;

void fl_pos(Fl_Widget* widget, Fl_Widget* parent, int x = 0, int y = 0) {
    widget->position(
        widget->x() + parent->x() + x, 
        widget->y() + parent->y() + y
    );
}

void fl_pos(vector<Fl_Widget*> widgets, Fl_Widget* parent, int x = 0, int y = 0) {
    for (Fl_Widget* widget: widgets) fl_pos(widget, parent, x, y);
}
