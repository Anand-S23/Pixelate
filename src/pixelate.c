#include "pixelate.h"

#include "renderer.c"
#include "memory.c"
#include "io.c"

internal int GetIndexFromClick(v2 app_cursor, int width, int height, int cell_dim)
{
    int i = ((int)app_cursor.x - (int)app_cursor.x % cell_dim) / cell_dim; 
    int j = ((int)app_cursor.y - (int)app_cursor.y % cell_dim) / cell_dim; 

    return (i >= 0 && j >= 0 && i <= width && j <= height) ? i + j * width : -1;
}

internal void CreateCanvas(app_state *state, int buffer_width, int buffer_height)
{
    state->canvas.pixel_buffer = AllocateMemoryArena(&state->permanent_arena, 
                                                     state->canvas_info.width * 
                                                     state->canvas_info.height * 
                                                     sizeof(pixel));

    int constraint = Min(state->canvas_info.width, state->canvas_info.height);
    state->camera.scale = (int)(1.0f / (f32)(constraint / 64 + 1) * 20);
    state->canvas.dimension = v2((state->camera.scale * state->canvas_info.width), 
                                 (state->camera.scale * state->canvas_info.height));

    state->canvas.origin = v2(buffer_width / 2.0f - state->canvas.dimension.x / 2.0f, 
                              buffer_height / 2.0f - state->canvas.dimension.y / 2.0f);

    state->canvas.current_color = v4(0, 0, 0, 255);
}

internal void GetCanvasSettings(offscreen_buffer *buffer, app_state *state)
{
    v2 size = v2(buffer->width / 2 - 100, buffer->height / 2);
    v2 position = v2(buffer->width / 2 - size.x / 2, 
                     buffer->height / 2 - size.y / 2);
    DrawFilledRect(buffer, position, size, v4(255, 255, 0, 255));

    /* NOTE: psuedo code when ui done

    */

   // NOTE: Remove this code when ui in place
   state->canvas_info.width = 64; 
   state->canvas_info.height = 64; 
   state->dimension_set = 1;
}


internal void UpdateApp(app_memory *memory, offscreen_buffer *buffer, input *input)
{
    Assert(sizeof(app_state) <= memory->storage_size);

    app_state *state = (app_state *)memory->storage;
    if (!memory->initialized)
    {
        if (!state->dimension_set)
        {
            GetCanvasSettings(buffer, state);
            goto end;
        }

        state->permanent_arena = InitMemoryArena(memory->storage, 
                                                 memory->storage_size);
        state->transient_arena = InitMemoryArena(memory->transient_storage, 
                                                 memory->transient_storage_size);
        AllocateMemoryArena(&state->permanent_arena, sizeof(app_state));

        CreateCanvas(state, buffer->width, buffer->height);

        memory->initialized = 1;
    }

    ClearBuffer(buffer);

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
        int index = GetIndexFromClick(state->canvas.cursor, 
                                      state->canvas_info.width, 
                                      state->canvas_info.height, 
                                      state->camera.scale);
        if (index >= 0)
        {
            state->canvas.pixel_buffer[index].filled = 1; 
            state->canvas.pixel_buffer[index].color = v3(0, 0, 0); 
        }
    }
    else if (input->right_mouse_down)
    {
        int index = GetIndexFromClick(state->canvas.cursor, 
                                      state->canvas_info.width,
                                      state->canvas_info.height,
                                      state->camera.scale);

        if (index >= 0)
        {
            state->canvas.pixel_buffer[index].filled = 0; 
        }
    }

    for (int j = 0; j < state->canvas_info.width; ++j)
    {
        for (int i = 0; i < state->canvas_info.height; ++i)
        {
            if (state->canvas.pixel_buffer[i + j * 64].filled == 0)
            {
                v4 color;
                if (j % 2 == i % 2)
                {
                    color = v4(150, 150, 150, 255);
                }
                else 
                {
                    color = v4(150, 150, 250, 255);
                }
                DrawFilledRect(buffer, 
                               v2(state->canvas.origin.x + i * state->camera.scale, 
                                  state->canvas.origin.y + j * state->camera.scale), 
                               v2(state->camera.scale, state->camera.scale), color);
            }
            else if (state->canvas.pixel_buffer[i + j * 64].filled == 1)
            {
                DrawFilledRect(buffer, 
                               v2(state->canvas.origin.x + i * state->camera.scale, 
                                  state->canvas.origin.y + j * state->camera.scale), 
                               v2(state->camera.scale, state->camera.scale), 
                                  state->canvas.current_color);
            }
        }
    }

    // DrawFilledRect(buffer, state->canvas.origin, 
    //                state->canvas.dimension, 
    //                v4(155, 155, 155, 155));

    
    end:;
}