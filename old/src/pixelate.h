#ifndef PIXELATE_H
#define PIXELATE_H

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

#include "linked_list.h"

typedef struct canvas
{
    linked_list layers;
    int width; 
    int height;
    v2 dimension;
    v2 origin;
    v2 cursor;
    v4 current_color;
} canvas;

typedef struct app_state
{
    memory_arena permanent_arena;
    memory_arena transient_arena;

    ui ui;
    
    canvas canvas;
    node *active_layer;
    app_camera camera;
    canvas_mode current_mode;
} app_state;

internal void UpdateApp(app_memory *memory, offscreen_buffer *buffer, input *input);

#endif