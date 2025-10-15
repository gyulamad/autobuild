#pragma once

#include "Canvas.hpp"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

// FLTK Canvas implementation
class Fl_CanvasBox: public Fl_Box, public Canvas {
public:
    Fl_CanvasBox(int X, int Y, int W, int H): Fl_Box(X, Y, W, H) {
        box(FL_DOWN_BOX);
        color(FL_BLACK);
    }

    virtual ~Fl_CanvasBox() {}
    
    // Handle mouse events
    int handle(int event) override {
        switch(event) {
            case FL_PUSH: {
                // Convert window coordinates to canvas coordinates
                int canvas_x = Fl::event_x() - x() - 2; // -2 for border
                int canvas_y = Fl::event_y() - y() - 2; // -2 for border
                int button = Fl::event_button();
                
                // Check if click is within canvas area
                if (canvas_x >= 0 && canvas_x < w() - 4 && canvas_y >= 0 && canvas_y < h() - 4) {
                    // cout << "Mouse down at canvas coordinates: (" << canvas_x << ", " << canvas_y 
                    //      << ") with button " << button << endl;
                    
                    // Call the virtual push method
                    if (push) push(canvas_x, canvas_y, button);
                }
                return 1;
            }
            case FL_RELEASE: {
                // Convert window coordinates to canvas coordinates
                int canvas_x = Fl::event_x() - x() - 2; // -2 for border
                int canvas_y = Fl::event_y() - y() - 2; // -2 for border
                int button = Fl::event_button();
                
                // Check if release is within canvas area
                if (canvas_x >= 0 && canvas_x < w() - 4 && canvas_y >= 0 && canvas_y < h() - 4) {
                    // cout << "Mouse up at canvas coordinates: (" << canvas_x << ", " << canvas_y 
                    //      << ") with button " << button << endl;
                    
                    // Call the virtual release method
                    if (release) release(canvas_x, canvas_y, button);
                }
                return 1;
            }
            case FL_DRAG: {
                // Convert window coordinates to canvas coordinates
                int canvas_x = Fl::event_x() - x() - 2; // -2 for border
                int canvas_y = Fl::event_y() - y() - 2; // -2 for border
                int button = Fl::event_state() & FL_BUTTONS; // Get which buttons are pressed
                
                // Check if drag is within canvas area
                if (canvas_x >= 0 && canvas_x < w() - 4 && canvas_y >= 0 && canvas_y < h() - 4) {
                    // cout << "Mouse drag at canvas coordinates: (" << canvas_x << ", " << canvas_y 
                    //      << ") with button state " << button << endl;
                    
                    // Call the virtual drag method
                    if (drag) drag(canvas_x, canvas_y, button);
                    // damage(FL_DAMAGE_OVERLAY);
                }
                return 1;
            }
            case FL_MOUSEWHEEL: {
                // Convert window coordinates to canvas coordinates
                int canvas_x = Fl::event_x() - x() - 2; // -2 for border
                int canvas_y = Fl::event_y() - y() - 2; // -2 for border

                // Check if scroll is within canvas area
                if (canvas_x >= 0 && canvas_x < w() - 4 && canvas_y >= 0 && canvas_y < h() - 4) {
                    // Get scroll direction and amount
                    int scroll_delta_x = Fl::event_dx();
                    int scroll_delta_y = Fl::event_dy();
                    int button_state = Fl::event_state() & FL_BUTTONS;
                    
                    // cout << "Mouse scroll at canvas coordinates: (" << canvas_x << ", " << canvas_y 
                    //     << ") with delta " << scroll_delta_x << ":" << scroll_delta_y << " and button state " << button_state << endl;
                    
                    // Call the virtual scroll method
                    if (scroll) scroll(canvas_x, canvas_y, scroll_delta_x, scroll_delta_y, button_state);
                    
                    // Redraw the widget to reflect any changes
                    // redraw();
                    // damage(FL_DAMAGE_OVERLAY);
                }
                return 1;
            }
            case FL_MOVE: {
                // Convert window coordinates to canvas coordinates
                int canvas_x = Fl::event_x() - x() - 2; // -2 for border
                int canvas_y = Fl::event_y() - y() - 2; // -2 for border
                
                // Check if move is within canvas area
                if (canvas_x >= 0 && canvas_x < w() - 4 && canvas_y >= 0 && canvas_y < h() - 4) {
                    // cout << "Mouse move at canvas coordinates: (" << canvas_x << ", " << canvas_y << ")" << endl;
                    
                    // Call the virtual move method
                    if (move) move(canvas_x, canvas_y);
                }
                return 1;
            }
            case FL_ENTER: {
                // Convert window coordinates to canvas coordinates
                int canvas_x = Fl::event_x() - x() - 2; // -2 for border
                int canvas_y = Fl::event_y() - y() - 2; // -2 for border
                
                // Check if enter is within canvas area
                if (canvas_x >= 0 && canvas_x < w() - 4 && canvas_y >= 0 && canvas_y < h() - 4) {
                    // cout << "Mouse enter at canvas coordinates: (" << canvas_x << ", " << canvas_y << ")" << endl;
                    
                    // Call the virtual enter method
                    if (enter) enter(canvas_x, canvas_y);
                }
                return 1;
            }
            case FL_LEAVE: {
                // Convert window coordinates to canvas coordinates
                int canvas_x = Fl::event_x() - x() - 2; // -2 for border
                int canvas_y = Fl::event_y() - y() - 2; // -2 for border
                
                // cout << "Mouse leave at canvas coordinates: (" << canvas_x << ", " << canvas_y << ")" << endl;
                
                // Call the virtual leave method (note: coordinates may be outside canvas area)
                if (leave) leave(canvas_x, canvas_y);
                return 1;
            }
            default:
                return Fl_Box::handle(event);
        }
    }
    
    // Convert canvas coordinates to window coordinates
    int canvas_to_window_x(int canvas_x) { return x() + 2 + canvas_x; } // +2 for border
    int canvas_to_window_y(int canvas_y) { return y() + 2 + canvas_y; } // +2 for border
    
    // Canvas drawing methods - all work in canvas coordinates (0,0 = top-left of canvas area)
    void line(int left1, int top1, int left2, int top2, unsigned int color, int style = 0) override {
        fl_push_clip(x() + 2, y() + 2, w() - 4, h() - 4);
        fl_color(static_cast<Fl_Color>(color));
        fl_line_style(style);
        fl_line(canvas_to_window_x(left1), canvas_to_window_y(top1), 
                canvas_to_window_x(left2), canvas_to_window_y(top2));
        fl_line_style(0);
        fl_pop_clip();
    }
    
    void circle(int left, int top, int radius, unsigned int color) override {
        fl_push_clip(x() + 2, y() + 2, w() - 4, h() - 4);
        fl_color(static_cast<Fl_Color>(color));
        fl_arc(canvas_to_window_x(left) - radius, canvas_to_window_y(top) - radius, 
               2*radius, 2*radius, 0, 360);
        fl_pop_clip();
    }
    
    void circlef(int left, int top, int radius, unsigned int color) override {
        fl_push_clip(x() + 2, y() + 2, w() - 4, h() - 4);
        fl_color(static_cast<Fl_Color>(color));
        fl_pie(canvas_to_window_x(left) - radius, canvas_to_window_y(top) - radius, 
               2*radius, 2*radius, 0, 360);
        fl_pop_clip();
    }
    
    void rect(int left, int top, int width, int height, unsigned int color) override {
        fl_push_clip(x() + 2, y() + 2, w() - 4, h() - 4);
        fl_color(static_cast<Fl_Color>(color));
        fl_rect(canvas_to_window_x(left), canvas_to_window_y(top), width, height);
        fl_pop_clip();
    }
    
    void rectf(int left, int top, int width, int height, unsigned int color) override {
        fl_push_clip(x() + 2, y() + 2, w() - 4, h() - 4);
        fl_color(static_cast<Fl_Color>(color));
        fl_rectf(canvas_to_window_x(left), canvas_to_window_y(top), width, height);
        fl_pop_clip();
    }
    
    void text(int left, int top, const string& text, unsigned int color, int font = 0, int size = 14) override {
        fl_push_clip(x() + 2, y() + 2, w() - 4, h() - 4);
        fl_color(static_cast<Fl_Color>(color));
        fl_font(font, size);
        fl_draw(text.c_str(), canvas_to_window_x(left), canvas_to_window_y(top));
        fl_pop_clip();
    }
    
    void measure(const string& text, int& width, int& height, int& descent, int font = 0, int size = 14) override {
        fl_font(font, size); // Set the font and size
        fl_measure(text.c_str(), width, height, 1); // Measure text width and height
        descent = fl_descent(); // Set descent: distance from baseline to bottom
    }

    int width() override { return w(); }
    int height() override { return h(); }
    
    void clear() override {
        fl_push_clip(x() + 2, y() + 2, w() - 4, h() - 4);
        fl_color(color());
        fl_rectf(x() + 2, y() + 2, w() - 4, h() - 4);
        fl_pop_clip();
    }
    
    void flush() override {
        Fl::flush();
    }
};
