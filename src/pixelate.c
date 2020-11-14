#include "pixelate.h"
#include "renderer.c"
#include "io.c"

internal void UpdateApp(app_memory *memory, offscreen_buffer *buffer, input *input)
{
    Assert(sizeof(app_state) <= memory->storage_size);

    local_persist int elements = 0;

    app_state *state = (app_state *)memory->storage;
    if (!memory->initialized)
    {
        state->screen.dimension = v2(256, 256);
        elements = (int)state->screen.dimension.x * (int)state->screen.dimension.y;
        state->screen.origin = v2(buffer->width / 2.0f - state->screen.dimension.x / 2.0f, 
                                  buffer->height / 2.0f - state->screen.dimension.y / 2.0f);


        memory->initialized = 1;
    }

    local_persist pixel screen_pixels[256*256] = {0};
    screen_pixels[0].filled = 1;
    screen_pixels[0].color = v3(255, 255, 0);

    state->app_cursor.x = input->mouse_x - state->screen.origin.x; 
    state->app_cursor.y = input->mouse_y - state->screen.origin.y; 

    if (input->right_mouse_down)
    {
        if (state->click_not_set) 
        {
            state->mouse_down_start = v2(state->app_cursor.x, state->app_cursor.y);
            state->click_not_set = 0; 
        }
        state->offset.x = state->app_cursor.x - state->mouse_down_start.x;
        state->offset.y = state->app_cursor.y - state->mouse_down_start.y;

        state->screen.origin.x += state->offset.x; 
        state->screen.origin.y += state->offset.y; 
    }
    else 
    {
        state->click_not_set = 1;
    }

    state->screen.origin.x = Max(Min(state->screen.origin.x, 
                                     buffer->width - state->screen.dimension.x), 0);
    state->screen.origin.y = Max(Min(state->screen.origin.y, 
                                     buffer->height - state->screen.dimension.y), 0);

    ClearBuffer(buffer);

    DrawFilledRect(buffer, state->screen.origin, 
                   v2(256, 256), v3(255, 255, 255));
    
    for (int i = 0; i < state->screen.dimension.y; ++i)
    {
        for (int j = 0; j < state->screen.dimension.x; ++j)
        {
            {
                v3 color = ((j / 8) % 2 ==  (i / 8) % 2) ? 
                            v3(255, 255, 255) : v3(120, 120, 120);
                DrawFilledRect(buffer, v2(state->screen.origin.x + j, state->screen.origin.y + i), 
                               v2(1, 1), color);
            }

        }
    }

    for (int i = 0; i < state->screen.dimension.y; ++i)
    {
        for (int j = 0; j < state->screen.dimension.x; ++j)
        {
            if (screen_pixels[(int)state->screen.dimension.x * i + j].filled == 1)
            {
                DrawFilledRect(buffer, v2(state->screen.origin.x + j, state->screen.origin.y + i), 
                               v2(1, 1), screen_pixels->color);
            }
        }
    }


    // get input 
    // update state 
    // render screen 
    // render ui
}