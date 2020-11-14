#ifndef PIXELATE_H
#define PIXELATE_H

typedef struct pixel
{
    v3 color;
    b32 filled; 
} pixel;

typedef struct draw_screen
{
    v2 dimension; 
    v2 origin; 
    int elements;
} draw_screen;

typedef struct app_state
{
    draw_screen screen;

    v2 app_cursor;
    b32 click_not_set;
    v2 mouse_down_start; 
    v2 offset; 
} app_state;

internal void UpdateApp(app_memory *memory, offscreen_buffer *buffer, input *input);

#endif