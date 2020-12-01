#ifndef PIXELATE_H
#define PIXELATE_H

typedef struct pixel
{
    v3 color; 
    b32 filled; 
} pixel;

typedef struct canvas
{
    pixel *pixel_buffer;
    v2 dimension; 
    v2 origin; 
    int elements;
    v4 current_color;
} canvas;

typedef struct app_state
{
    canvas screen;

    v2 app_cursor;
    b32 click_not_set;
    v2 mouse_down_start; 
    v2 offset; 
    v2 scale;
} app_state;

internal void UpdateApp(app_memory *memory, offscreen_buffer *buffer, input *input);

#endif