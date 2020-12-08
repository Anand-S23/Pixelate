#ifndef PIXELATE_H
#define PIXELATE_H

#include "memory.h"

typedef enum canvas_state
{
    CANVAS_STATE_create, 
    CANVAS_STATE_edit, 
    CANVAS_STATE_export
} canvas_state;

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

typedef struct canvas
{
    pixel *pixel_buffer;
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
    
    b32 dimension_set;
    canvas canvas;
    app_camera camera;
} app_state;

internal void UpdateApp(app_memory *memory, offscreen_buffer *buffer, input *input);

#endif