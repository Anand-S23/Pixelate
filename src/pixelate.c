#include "pixelate.h"
#include "renderer.c"
#include "io.c"

internal void UpdateApp(app_memory *memory, offscreen_buffer *buffer, input *input)
{
    Assert(sizeof(app_state) <= memory->storage_size);

    app_state *state = (app_state *)memory->storage;
    if (!memory->initialized)
    {
        state->screen.dimension = v2(256, 256);
        state->screen.elements = (int)state->screen.dimension.x * (int)state->screen.dimension.y;
        state->screen.origin = v2(buffer->width / 2.0f - state->screen.dimension.x / 2.0f, 
                                  buffer->height / 2.0f - state->screen.dimension.y / 2.0f);

        state->scale = v2(1, 1);

        memory->initialized = 1;
    }

    state->app_cursor.x = input->mouse_x - state->screen.origin.x; 
    state->app_cursor.y = input->mouse_y - state->screen.origin.y; 

    if (input->scroll_value == input->prev_scroll_value)
    {
        input->scroll_delta = 0; 
    }
    else 
    {
        input->scroll_delta = input->scroll_value - input->prev_scroll_value;
    }

    if (input->middle_mouse_down)
    {
        if (state->click_not_set) 
        {
            state->mouse_down_start = v2(state->app_cursor.x, state->app_cursor.y);
            state->click_not_set = 0; 
        }
        
        state->offset.x = (state->app_cursor.x - state->mouse_down_start.x);
        state->offset.y = (state->app_cursor.y - state->mouse_down_start.y);

        state->screen.origin.x += state->offset.x; 
        state->screen.origin.y += state->offset.y; 
    }
    else 
    {
        state->click_not_set = 1;
    }

    if (input->left_mouse_down)
    {
        state->scale.x = -1;
        state->scale.y = -1;
        state->screen.dimension.x += state->scale.x;
        state->screen.dimension.y += state->scale.y;
    }
    else if (input->right_mouse_down)
    {
        state->scale.x = 1;
        state->scale.y = 1;
        state->screen.dimension.x += state->scale.x;
        state->screen.dimension.y += state->scale.y;
    }

    ClearBuffer(buffer);

    DrawFilledRect(buffer, state->screen.origin, 
                   state->screen.dimension, v4(255, 255, 255, 255));
    
#if 0
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
            if (canvas[(int)state->screen.dimension.x * i + j].filled)
            {
                DrawFilledRect(buffer, v2(state->screen.origin.x + j*8, state->screen.origin.y + i*8), 
                               v2(8, 8), canvas[(int)state->screen.dimension.x * i + j].color);
            }
        }
    }
#endif

    input->prev_scroll_value = input->scroll_value;
}