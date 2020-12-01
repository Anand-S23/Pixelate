#include "pixelate.h"
#include "renderer.c"
#include "io.c"

internal v2 GetIndexFromClick(v2 app_cursor)
{
    v2 result;
    
}

internal void UpdateApp(app_memory *memory, offscreen_buffer *buffer, input *input)
{
    Assert(sizeof(app_state) <= memory->storage_size);

    app_state *state = (app_state *)memory->storage;
    if (!memory->initialized)
    {
        state->screen.dimension = v2(256, 256);
        state->scale = v2(1, 1);
        state->screen.elements = 64 * 64;

        state->screen.origin = v2(buffer->width / 2.0f - state->screen.dimension.x / 2.0f, 
                                  buffer->height / 2.0f - state->screen.dimension.y / 2.0f);

        // TODO: Use memory->storage instead of call malloc
        state->screen.pixel_buffer = (pixel *)calloc(state->screen.elements, sizeof(pixel));

        memory->initialized = 1;
    }

    // UPDATE 

    // Get mouse input in relation to the canvas
    state->app_cursor.x = input->mouse_x - state->screen.origin.x; 
    state->app_cursor.y = input->mouse_y - state->screen.origin.y; 

    // Move canvas around
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

    // Scale canvas
    if (input->scroll_value == input->prev_scroll_value)
    {
        input->scroll_delta = 0; 
    }
    else 
    {
        input->scroll_delta = input->scroll_value - input->prev_scroll_value;
    }

    // Process drawing and erasing 
    if (input->left_mouse_down)
    {
    }
    else if (input->right_mouse_down)
    {
    }

    // RENDER

    ClearBuffer(buffer);

    DrawFilledRect(buffer, state->screen.origin, 
                   state->screen.dimension, v4(255, 255, 255, 255));

    for (int j = 0; j < 64; ++j)
    {
        for (int i = 0; i < 64; ++i)
        {
            v4 color = v4(0, 255, 255, 255);

            if (state->screen.pixel_buffer[i].filled == 0)
            {
                DrawFilledRect(buffer, v2(state->screen.origin.x + i * 4, state->screen.origin.y + j * 4), 
                                v2(4, 4), color);
            }
        }
    }
    
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

    // TODO: Needs to be moved into platfrom
    input->prev_scroll_value = input->scroll_value;
}