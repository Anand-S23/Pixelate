#ifndef UI_H
#define UI_H

// NOTE: 
//      UI based on the UI style presented by Ryan Fleury in handmade network game
//      https://github.com/ryanfleury/handmade_network_game

#define UI_MAX_WIDGETS 128
#define UI_MAX_AUTO_LAYOUT_GROUPS 16

#define UI_IDGen()   UIIDInit(__LINE__ + UI_SRC_ID, 0)
#define UI_IDGenI(i) UIIDInit(__LINE__ + UI_SRC_ID, i)

typedef enum widget
{
    UI_button,
    UI_slider,
    UI_text_input, 
    UI_number_picker, 
    UI_color_picker, 
    UI_scroll_bar
} widget;

typedef struct ui_id
{
    u32 primary;
    u32 secondary;
} ui_id;

typedef struct ui_widget
{
    widget type;
    ui_id id;
    v4 rect;
    f32 t_hot;
    f32 t_active;
    
    union
    {
        struct slider
        {
            f32 value;
        } slider;
    };
} ui_widget;

typedef struct ui_input
{
    f32 cursor_x;
    f32 cursor_y;
    b32 left_cursor_down;
    b32 right_cursor_down;
} ui_input;

typedef struct ui
{
    f32 cursor_x;
    f32 cursor_y;
    b32 left_cursor_down;
    b32 right_cursor_down;
    
    u32 widget_count;
    ui_widget widgets[UI_MAX_WIDGETS];
    
    u32 auto_layout_stack_pos;
    struct
    {
        b32 is_column;
        v2 position;
        v2 size;
        f32 progress;
    }
    auto_layout_stack[UI_MAX_AUTO_LAYOUT_GROUPS];
    
    ui_id hot;
    ui_id active;
} ui;

#endif