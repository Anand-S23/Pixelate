#ifndef DRIFT_UI_H
#define DRIFT_UI_H

#define UI_MAX_WIDGETS 128
#define UI_MAX_AUTO_LAYOUT_GROUPS 16
#define UI_MAX_WIDGET_ELEMENTS 16

#define UI_SRC_ID 1000
#define UIIDGen() UIIDInit(__LINE__ + UI_SRC_ID, 0)
#define UIIDGenI(i) UIIDInit(__LINE__ + UI_SRC_ID, i)

typedef enum widget_type
{
    UI_WIDGET_button,
    UI_WIDGET_slider,
    UI_WIDGET_pannel
} widget_type;

typedef struct ui_id
{
    u32 primary;
    u32 secondary;
} ui_id;

typedef struct window
{
    v2 position;
    v2 size;
    b32 is_open;
    i32 flags;
} window;

typedef struct ui_widget
{
    widget_type type;
    ui_id id;
    v2 position;
    v2 size;
    char *text;
    v3 text_color;
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

typedef struct ui
{
    f32 mouse_x;
    f32 mouse_y;
    b32 left_mouse_down;
    b32 right_mouse_down;

    renderer *renderer;
    font ui_font;
    
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

    ui_id open_menu;
} ui;

#endif
