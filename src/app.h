#ifndef APP_H
#define APP_H

typedef enum canvas_mode
{
    CANVAS_MODE_blank,
    CANVAS_MODE_create, 
    CANVAS_MODE_edit, 
    CANVAS_MODE_export
} canvas_mode;

#define default_pixel_buffer_size 1024

typedef struct pixel_buffer
{
    void *data; 
    u32 position;
    u32 size;
} pixel_buffer;

typedef struct app_camera
{
    v2 offset; 
    int scale;
    b32 click_not_set;
    v2 mouse_down_start; 
} app_camera;

typedef struct layer
{
    u32 *buffer;
    u32 index;
    b32 active;
    f32 alpha;
    b32 update_required;
    texture texture;
} layer;

typedef struct canvas
{
    linked_list layers;
    u32 layer_count;
    v2 normal_size;
    v2 origin;
    v2 dimension;
    v2 cursor;
    v4 primary_color;
    v4 secondary_color;
    v4 background_color_1;
    v4 background_color_2;
    texture background;
} canvas;

typedef struct app_state
{
    renderer renderer;
    f32 delta_t;

    ui ui;
    
    canvas canvas;
    layer *active_layer;
    app_camera camera;
    canvas_mode current_mode;
} app_state;

#endif
