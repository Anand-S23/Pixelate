#include "pixelate.h"

#include "renderer.c"
#include "memory.c"
#include "io.c"

internal int GetIndexFromClick(v2 app_cursor, int width, int cell_dim)
{
    int i = (float)((int)app_cursor.x / cell_dim);
    int j = (float)((int)app_cursor.y / cell_dim);
    return (i >= 0 && j >= 0) ? i + j * width : -1;
}

internal void CreateCanvas(app_state *state, int buffer_width, int buffer_height)
{
    state->canvas.pixel_buffer = AllocateMemoryArena(&state->permanent_arena, 
                                                     state->canvas_info.width * 
                                                     state->canvas_info.height * 
                                                     sizeof(pixel));

    int constraint = Min(state->canvas_info.width, state->canvas_info.height);
    int scale = (int)(1.0f / (f32)(constraint / 64 + 1)) * 20;
    state->canvas.dimension = v2((scale * state->canvas_info.width), 
                                 (scale * state->canvas_info.height));

    state->canvas.origin = v2(buffer_width / 2.0f - state->canvas.dimension.x / 2.0f, 
                              buffer_height / 2.0f - state->canvas.dimension.y / 2.0f);

    state->canvas.current_color = v4(0, 0, 0, 255);
}

internal void GetCanvasSettings(offscreen_buffer *buffer, app_state *state)
{
    DrawFilledRect(buffer, v2(buffer->width/2, buffer->height/2), 
                   v2(256, 256), v4(255, 255, 0, 255));
}


internal void UpdateApp(app_memory *memory, offscreen_buffer *buffer, input *input)
{
    Assert(sizeof(app_state) <= memory->storage_size);

    app_state *state = (app_state *)memory->storage;
    if (!memory->initialized)
    {
#if 0
        if (!state->dimension_set)
        {
            GetCanvasSettings(buffer, state);
            goto end;
        }
#endif

        state->permanent_arena = InitMemoryArena(memory->storage, 
                                                 memory->storage_size);
        state->transient_arena = InitMemoryArena(memory->transient_storage, 
                                                 memory->transient_storage_size);
        AllocateMemoryArena(&state->permanent_arena, sizeof(app_state));

        CreateCanvas(state, buffer->width, buffer->height);

        memory->initialized = 1;
    }

    // UPDATE 

    // Get mouse input in relation to the canvas
    state->canvas.cursor.x = input->mouse_x - state->canvas.origin.x; 
    state->canvas.cursor.y = input->mouse_y - state->canvas.origin.y; 

    // Move canvas around
    if (input->middle_mouse_down)
    {
        if (state->camera.click_not_set) 
        {
            state->camera.mouse_down_start = v2(state->canvas.cursor.x, 
                                                state->canvas.cursor.y);
            state->camera.click_not_set = 0; 
        }
        
        state->camera.offset.x = 
            (state->canvas.cursor.x - state->camera.mouse_down_start.x);
        state->camera.offset.y = 
            (state->canvas.cursor.y - state->camera.mouse_down_start.y);

        state->canvas.origin.x += state->camera.offset.x; 
        state->canvas.origin.y += state->camera.offset.y; 
    }
    else 
    {
        state->camera.click_not_set = 1;
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
        int index = GetIndexFromClick(state->canvas.cursor, 64, 4);
        if (index >= 0)
        {
            state->canvas.pixel_buffer[index].filled = 1; 
            state->canvas.pixel_buffer[index].color = v3(0, 0, 0); 
        }
    }
    else if (input->right_mouse_down)
    {
        int index = GetIndexFromClick(state->canvas.cursor, 64, 4);
        if (index >= 0)
        {
            state->canvas.pixel_buffer[index].filled = 0; 
        }
    }

    // RENDER

    ClearBuffer(buffer);

    DrawFilledRect(buffer, state->canvas.origin, 
                   state->canvas.dimension, v4(255, 255, 255, 255));

    // NOTE: Currently hard coded for 64 x 64 
    for (int j = 0; j < 64; ++j)
    {
        for (int i = 0; i < 64; ++i)
        {
            if (state->canvas.pixel_buffer[i + j * 64].filled == 0)
            {
                v4 color;
                if ((j / 64) % 2 == (i / 64) % 2)
                {
                    color = v4(100, 100, 100, 255);
                }
                else 
                {
                    color = v4(50, 50, 150, 255);
                }
                DrawFilledRect(buffer, 
                               v2(state->canvas.origin.x + i * 4, 
                                  state->canvas.origin.y + j * 4), 
                                v2(4, 4), color);
            }
            else if (state->canvas.pixel_buffer[i + j * 64].filled == 1)
            {
                DrawFilledRect(buffer, 
                               v2(state->canvas.origin.x + i * 4,
                                  state->canvas.origin.y + j * 4), 
                               v2(4, 4), state->canvas.current_color);
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

    end:;
}