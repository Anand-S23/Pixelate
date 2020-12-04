#ifndef PIXELATE_H
#define PIXELATE_H

#include "memory.h"

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

typedef struct canvas_settings
{
    int width; 
    int height; 
    v4 background_color;
} canvas_settings;

typedef struct canvas
{
    pixel *pixel_buffer;
    //int buffer_width;
    // int buffer_height;
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
    canvas_settings canvas_info;
    canvas canvas;
    app_camera camera;
} app_state;

internal void UpdateApp(app_memory *memory, offscreen_buffer *buffer, input *input);

#endif