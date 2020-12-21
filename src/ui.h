#ifndef UI_H
#define UI_H

#define UI_MAX_WIDGETS 128
#define UI_MAX_AUTO_LAYOUT_GROUPS 16

#define UIIDGen() UIIDInit(__LINE__ + UI_SRC_ID, 0)
#define UIIDGenI(i) UIIDInit(__LINE__ + UI_SRC_ID, i)

typedef enum widget_type
{
    UI_WIDGET_button,
    UI_WIDGET_slider,
} widget_type;

typedef struct ui_id
{
    u32 primary;
    u32 secondary;
} ui_id;

typedef struct ui_widget
{
    widget_type type;
    ui_id id;
    v4 rect;
    f32 t_hot;
    f32 t_active;
    
    union
    {
        struct Slider
        {
            f32 value;
        } slider;
    };
} ui_widget;

typedef struct ui
{
    f32 mouse_x;
    f32 mouse_y;
    b32 left_mouse_down;
    b32 right_mouse_down;

    offscreen_buffer *buffer;
    
    u32 widget_count;
    ui_widget widgets[UI_MAX_WIDGETS];
    
    u32 auto_layout_stack_pos;
    struct
    {
        b32 is_column;
        v2 position;
        v2 size;
        f32 progress;
    } auto_layout_stack[UI_MAX_AUTO_LAYOUT_GROUPS];
    
    ui_id hot;
    ui_id active;
} ui;

#endif