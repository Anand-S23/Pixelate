#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <drift.h>
#include "app.h"

global app_state *state;

internal int GetIndexFromClick(v2 app_cursor, int width, int height, int cell_dim)
{
    int i = ((int)app_cursor.x - (int)app_cursor.x % cell_dim) / cell_dim; 
    int j = ((int)app_cursor.y - (int)app_cursor.y % cell_dim) / cell_dim; 

    return (i >= 0 && j >= 0 && i <= width && j <= height) ? i + j * width : -1;
}

INIT_APP
{
    Assert(sizeof(app_state) <= platform->storage_size);
    state = (app_state *)platform->storage;
    InitRenderer(&state->renderer);

    int buffer_width = platform->window_width;
    int buffer_height = platform->window_height;
    state->current_mode = CANVAS_MODE_edit;
    state->canvas.width = 64; 
    state->canvas.height = 64; 
    state->canvas.layers = CreateList();
    u32 *layer = (u32 *)malloc(state->canvas.width * state->canvas.height * sizeof(u32));
    Push(&state->canvas.layers, layer);
    state->active_layer = state->canvas.layers.head;
    int constraint = Min(state->canvas.width, state->canvas.height);
    state->camera.scale = (int)(1.0f / (f32)(constraint / 64 + 1) * 20);
    state->canvas.dimension = v2((state->camera.scale * state->canvas.width), 
                                 (state->camera.scale * state->canvas.height));

    state->canvas.origin = v2(buffer_width / 2.0f - state->canvas.dimension.x / 2.0f, 
                              buffer_height / 2.0f - state->canvas.dimension.y / 2.0f);

    state->canvas.current_color = v4(0.f, 0.f, 0.f, 1.f);

    platform->initialized = 1;
}

UPDATE_APP
{
    state->delta_t = platform->current_time - platform->last_time;

    // Get mouse input in relation to the canvas
    state->canvas.cursor.x = platform->mouse_x - state->canvas.origin.x;
    state->canvas.cursor.y = platform->mouse_y - state->canvas.origin.y;

    // Move canvas around
    if (platform->middle_mouse_down && 
        state->current_mode == CANVAS_MODE_edit)
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

    // Process drawing and erasing 
    if (platform->left_mouse_down && 
        state->current_mode == CANVAS_MODE_edit)
    {
        int index = GetIndexFromClick(state->canvas.cursor, 
                                      state->canvas.width, 
                                      state->canvas.height, 
                                      state->camera.scale);
        if (index >= 0)
        {
            // state->active_layer->data[index] = u32_color(state->canvas.current_color); 
        }
    }
    else if (platform->right_mouse_down && state->current_mode == CANVAS_MODE_edit)
    {
        int index = GetIndexFromClick(state->canvas.cursor, 
                                      state->canvas.width,
                                      state->canvas.height,
                                      state->camera.scale);

        if (index >= 0)
        {
            // state->active_layer->buffer[index] = u32_color(v4(0, 0, 0, 0)); 
        }
    }


    // Render
    ClearScreen(v4(0.3f, 0.3f, 0.3f, 1.0f));
    BeginRenderer(&state->renderer, platform->window_width, platform->window_height);

    if (state->current_mode == CANVAS_MODE_edit)
    {
        for (int n = 0; n < state->canvas.layers.length; ++n)
        {
            u32 *layer = Get(&state->canvas.layers, n);
            for (int j = 0; j < state->canvas.width; ++j)
            {
                for (int i = 0; i < state->canvas.height; ++i)
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

                    RenderRect(&state->renderer,
                               v2(state->canvas.origin.x + i * state->camera.scale, 
                                  state->canvas.origin.y + j * state->camera.scale),
                               v2(state->camera.scale, state->camera.scale), color);

                    /*
                    RenderRect(&state->renderer,
                               v4(state->canvas.origin.x + i * state->camera.scale, 
                                  state->canvas.origin.y + j * state->camera.scale),
                               v2(state->camera.scale, state->camera.scale), 
                               layer[i + j * state->canvas.width]);
                    */
                }
            }
        }
    }

    SubmitRenderer(&state->renderer);
    platform->SwapBuffers();
}

DRIFT_MAIN
{
    drift_application app = {
        .name = "Pixelate",
        .window_width = 1280,
        .window_height = 720
    };

    platform = platform_;
    return app;
}
