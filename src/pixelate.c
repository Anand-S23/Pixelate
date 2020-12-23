#include "renderer.c"
#include "memory.c"
#include "ui.c"
#include "io.c"

#include "pixelate.h"

internal int GetIndexFromClick(v2 app_cursor, int width, int height, int cell_dim)
{
    int i = ((int)app_cursor.x - (int)app_cursor.x % cell_dim) / cell_dim; 
    int j = ((int)app_cursor.y - (int)app_cursor.y % cell_dim) / cell_dim; 

    return (i >= 0 && j >= 0 && i <= width && j <= height) ? i + j * width : -1;
}

internal void CreateCanvas(app_state *state, int buffer_width, int buffer_height)
{
    state->canvas.pixel_buffer = AllocateMemoryArena(&state->permanent_arena, 
                                                     state->canvas.width * 
                                                     state->canvas.height * 
                                                     sizeof(pixel));

    int constraint = Min(state->canvas.width, state->canvas.height);
    state->camera.scale = (int)(1.0f / (f32)(constraint / 64 + 1) * 20);
    state->canvas.dimension = v2((state->camera.scale * state->canvas.width), 
                                 (state->camera.scale * state->canvas.height));

    state->canvas.origin = v2(buffer_width / 2.0f - state->canvas.dimension.x / 2.0f, 
                              buffer_height / 2.0f - state->canvas.dimension.y / 2.0f);

    state->canvas.current_color = v4(0.f, 0.f, 0.f, 1.f);
}

internal void GetCanvasSettings(offscreen_buffer *buffer, app_state *state, 
                                input *input)
{
    UIBeginFrame(&state->ui, buffer, input);
    {

        UIBeginWindow(&state->ui, UIIDGen(), "Create a new image", v4(100, 100, 380, 280));
        {
            local_persist int canvas_width = 64;
            local_persist int canvas_height = 64;
            
            if (UIButton(&state->ui, UIIDGen(), "Ok"))
            {
                state->canvas.width  = canvas_width; 
                state->canvas.height = canvas_height; 
                CreateCanvas(state, buffer->width, buffer->height);

                state->current_mode = CANVAS_MODE_edit;
            }
        }
        UIEndWindow(&state->ui);
    }
    UIEndFrame(&state->ui);
}

internal void UpdateApp(app_memory *memory, offscreen_buffer *buffer, input *input)
{
    Assert(sizeof(app_state) <= memory->storage_size);

    app_state *state = (app_state *)memory->storage;
    if (!memory->initialized)
    {
        state->permanent_arena = InitMemoryArena(memory->storage, 
                                                 memory->storage_size);
        state->transient_arena = InitMemoryArena(memory->transient_storage, 
                                                 memory->transient_storage_size);
        AllocateMemoryArena(&state->permanent_arena, sizeof(app_state));

        state->current_mode = CANVAS_MODE_create;

        memory->initialized = 1;
    }

    // Get mouse input in relation to the canvas
    state->canvas.cursor.x = input->mouse_x - state->canvas.origin.x;
    state->canvas.cursor.y = input->mouse_y - state->canvas.origin.y;

    // Move canvas around
    if (input->middle_mouse_down && state->current_mode == CANVAS_MODE_edit)
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
    if (input->scroll_value == input->prev_scroll_value && state->current_mode == CANVAS_MODE_edit)
    {
        input->scroll_delta = 0; 
    }
    else 
    {
        input->scroll_delta = input->scroll_value - input->prev_scroll_value;
    }

    // Process drawing and erasing 
    if (input->left_mouse_down && state->current_mode == CANVAS_MODE_edit)
    {
        int index = GetIndexFromClick(state->canvas.cursor, 
                                    state->canvas.width, 
                                    state->canvas.height, 
                                    state->camera.scale);
        if (index >= 0)
        {
            state->canvas.pixel_buffer[index].filled = 1; 
            state->canvas.pixel_buffer[index].color = v3(0.f, 0.f, 0.f); 
        }
    }
    else if (input->right_mouse_down && state->current_mode == CANVAS_MODE_edit)
    {
        int index = GetIndexFromClick(state->canvas.cursor, 
                                    state->canvas.width,
                                    state->canvas.height,
                                    state->camera.scale);

        if (index >= 0)
        {
            state->canvas.pixel_buffer[index].filled = 0; 
        }
    }

    ClearBuffer(buffer, v4(0.22f, 0.22f, 0.24f, 1.f));

    if (state->current_mode == CANVAS_MODE_edit)
    {
        for (int j = 0; j < state->canvas.width; ++j)
        {
            for (int i = 0; i < state->canvas.height; ++i)
            {
                if (state->canvas.pixel_buffer[i + j * 64].filled == 0)
                {
                    v4 color;
                    if (j % 2 == i % 2)
                    {
                        color = v4(0.6f, 0.6f, 0.6f, 1.f);
                    }
                    else 
                    {
                        color = v4(0.6f, 0.6f, 0.98f, 1.f);
                    }

                    DrawFilledRect(buffer, 
                                v4(state->canvas.origin.x + i * state->camera.scale, 
                                    state->canvas.origin.y + j * state->camera.scale, 
                                    state->camera.scale, state->camera.scale), color);
                }
                else if (state->canvas.pixel_buffer[i + j * 64].filled == 1)
                {
                    DrawFilledRect(buffer, 
                                v4(state->canvas.origin.x + i * state->camera.scale, 
                                    state->canvas.origin.y + j * state->camera.scale,
                                    state->camera.scale, state->camera.scale), 
                                    state->canvas.current_color);
                }
            }
        }
    }
    else if (state->current_mode == CANVAS_MODE_create)
    {
        // Get canvas create information 
        GetCanvasSettings(buffer, state, input);
    }


    // UI for tools
    UIBeginFrame(&state->ui, buffer, input);
    {
        if (UIMenu(&state->ui, UIIDGen(), "Pixelate Menu",  
                   v4(0, 0, 50, 50), v4(50, 0, 100, 50)))
        {
            if (UIButton(&state->ui, UIIDGen(), "Menu option 1"))
            {
            }

            if (UIButton(&state->ui, UIIDGen(), "Menu option 2"))
            {
            }

            if (UIButton(&state->ui, UIIDGen(), "Menu option 3"))
            {
            }
        }
        UIPopColumn(&state->ui);
    }
    UIEndFrame(&state->ui);
}