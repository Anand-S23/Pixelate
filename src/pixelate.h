#ifndef PIXELATE_H
#define PIXELATE_H

typedef enum canvas_mode
{
    CANVAS_MODE_blank,
    CANVAS_MODE_create, 
    CANVAS_MODE_edit, 
    CANVAS_MODE_export
} canvas_mode;

typedef struct pixel
{
    v3 color; 
    b32 filled; 
} pixel;

typedef struct app_camera
{
    v2 offset; 
    int scale;
    b32 click_not_set;
    v2 mouse_down_start; 
} app_camera;

typedef struct pixel_buffer
{
    pixel *data;
    int width; 
    int height;
};

#include "linked_list.h"

typedef struct canvas
{
    // LINKED LIST pixel_buffer
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
    app_camera camera;
    canvas_mode current_mode;
} app_state;

internal void UpdateApp(app_memory *memory, offscreen_buffer *buffer, input *input);

#endif