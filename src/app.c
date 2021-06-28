#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <drift.h>
#include "app.h"

global app_state *state;

internal u32 *CreateLayerBuffer(v2 size)
{
    // TODO: switch to memory arena
    u32 *result = (u32 *)malloc(size.width * size.height * sizeof(u32));
    return result;
}

internal texture CreateBackgroundTexture(v2 dimension, v2 normal_size, int scale,
                                         int bg_scale, v4 bg_color1, v4 bg_color2)
{
    texture_buffer buffer;
    InitTextureBuffer(&buffer, dimension.width, dimension.height);
    ClearTextureBuffer(&buffer, v4(0.f, 0.f, 0.f, 1.f));

    int cell_size = scale * bg_scale;
    for (int j = 0; j < (int)(normal_size.height / bg_scale) + 1; ++j)
    {
        for (int i = 0; i < (int)(normal_size.width / bg_scale) + 1; ++i)
        {
            v4 color;
            if (i % 2 == j % 2)
            {
                // Default color: v4(0.6f, 0.6f, 0.6f, 1.f);
                color = bg_color1;
            }
            else 
            {
                // Default color: v4(0.6f, 0.6f, 0.98f, 1.f);
                color = bg_color2;
            }

            RenderRectToBuffer(&buffer, v2(i * cell_size, j * cell_size),
                               v2(cell_size, cell_size), color);
        }
    }

    ReverseBuffer((u8 *)buffer.memory, buffer.width, buffer.height);
    texture tex = CreateTextureFromData(buffer.memory, buffer.width, buffer.height);
    free(buffer.memory);
    return tex;
}

internal void CreateLayerTexture(layer *layer, v2 dimension,
                                 v2 normal_size, int scale)
{
    texture_buffer buffer;
    InitTextureBuffer(&buffer, dimension.width, dimension.height);
    ClearTextureBuffer(&buffer, v4(0.f, 0.f, 0.f, 0.f));

    int cell_size = scale;
    for (int j = 0; j < normal_size.height; ++j)
    {
        for (int i = 0; i < normal_size.width; ++i)
        {
            v4 color = UnpackRGBA((u32)layer->buffer[j * (int)normal_size.width + i]);
            RenderRectToBuffer(&buffer, v2(i * cell_size, j * cell_size),
                               v2(cell_size, cell_size), color);
        }
    }

    ReverseBuffer((u8 *)buffer.memory, buffer.width, buffer.height);
    texture tex = CreateTextureFromData(buffer.memory, buffer.width, buffer.height);
    free(buffer.memory);
    layer->texture = tex;
}

internal void InitCanvas(canvas *canvas, v2 normal_size, f32 scale, v4 bc1, v4 bc2, int bg_scale)
{
    layer new_layer = {0};
    {
        new_layer.buffer = CreateLayerBuffer(normal_size);
        new_layer.index = 0;
        new_layer.active = 1;
        new_layer.alpha = 1.f;
        new_layer.update_required = 0;
    }

    canvas->normal_size = normal_size;
    Append(&canvas->layers, &new_layer);
    canvas->layer_count = 1;

    canvas->dimension = v2((scale * canvas->normal_size.width),
                           (scale * canvas->normal_size.height));

    f32 canvas_x = platform->window_width / 2.f - canvas->dimension.width / 2.f; 
    f32 canvas_y = platform->window_height / 2.f - canvas->dimension.height / 2.f;
    canvas->origin = v2(canvas_x, canvas_y);

    canvas->primary_color = v4(0.f, 0.f, 0.f, 1.f);
    canvas->secondary_color = v4(1.f, 1.f, 1.f, 1.f);
    canvas->background_color_1 = bc1;
    canvas->background_color_2 = bc2;

    canvas->background = CreateBackgroundTexture(canvas->dimension, normal_size,
                                                 scale, bg_scale, bc1, bc2);
}

internal int GetIndexFromClick(v2 app_cursor, v2 normal_size, int cell_dim)
{
    int i = ((int)app_cursor.x - (int)app_cursor.x % cell_dim) / cell_dim; 
    int j = ((int)app_cursor.y - (int)app_cursor.y % cell_dim) / cell_dim; 

    b32 valid = (i >= 0 && j >= 0 && i <= normal_size.width && j <= normal_size.height);
    return valid ? i + j * normal_size.width : -1;
}

INIT_APP
{
    Assert(sizeof(app_state) <= platform->storage_size);
    state = (app_state *)platform->storage;
    InitRenderer(&state->renderer);

    v2 normal_size = v2(64.f, 64.f);
    int constraint = Min(normal_size.width, normal_size.height);
    state->camera.scale = (int)(1.0f / (f32)(constraint / 64 + 1) * 20);
    InitCanvas(&state->canvas, normal_size, state->camera.scale,
               v4(0.6f, 0.6f, 0.6f, 1.f), v4(0.6f, 0.6f, 1.f, 1.f), 4);
    state->active_layer = (layer *)Top(&state->canvas.layers);
    state->current_mode = CANVAS_MODE_edit;
    platform->initialized = 1;
}

UPDATE_APP
{
    state->delta_t = platform->current_time - platform->last_time;

    // Get mouse input in relation to the canvas
    canvas *canvas = &state->canvas;
    canvas->cursor.x = platform->mouse_x - canvas->origin.x;
    canvas->cursor.y = platform->mouse_y - canvas->origin.y;

    // Move canvas around
    if (platform->middle_mouse_down && 
        state->current_mode == CANVAS_MODE_edit)
    {
        if (state->camera.click_not_set) 
        {
            state->camera.mouse_down_start = v2(canvas->cursor.x, 
                                                canvas->cursor.y);
            state->camera.click_not_set = 0; 
        }
        
        state->camera.offset.x = 
            (canvas->cursor.x - state->camera.mouse_down_start.x);
        state->camera.offset.y = 
            (canvas->cursor.y - state->camera.mouse_down_start.y);

        canvas->origin.x += state->camera.offset.x; 
        canvas->origin.y += state->camera.offset.y; 
    }
    else 
    {
        state->camera.click_not_set = 1;
    }

    if (platform->left_mouse_down &&
        state->current_mode == CANVAS_MODE_edit)
    {
        int index = GetIndexFromClick(state->canvas.cursor, 
                                      state->canvas.normal_size,
                                      state->camera.scale);
        if (index >= 0)
        {
            state->active_layer->buffer[index] =
                PackRGBA(state->canvas.primary_color); 
        }

        state->active_layer->update_required = 1;
    }
    else if (platform->right_mouse_down &&
             state->current_mode == CANVAS_MODE_edit)
    {
        int index = GetIndexFromClick(state->canvas.cursor, 
                                      state->canvas.normal_size,
                                      state->camera.scale);
        if (index >= 0)
        {
            state->active_layer->buffer[index] =
                PackRGBA(state->canvas.secondary_color); 
        }

        state->active_layer->update_required = 1;
    }

    // Render
    ClearScreen(v4(0.25f, 0.25f, 0.25f, 1.0f));
    BeginRenderer(&state->renderer, platform->window_width, platform->window_height);

    //RenderRect(&state->renderer, canvas->origin, canvas->dimension, v4(1, 1, 1, 1));

    RenderTexture(&state->renderer, canvas->origin,
                  canvas->dimension, &canvas->background);

    for (int i = 0; i < canvas->layers.length; ++i)
    {
        layer *current = (layer *)Get(&canvas->layers, i);

        if (current->update_required)
        {
            CreateLayerTexture(current, canvas->dimension,
                               canvas->normal_size, state->camera.scale);
        }

        if (current->texture.id)
        {
            RenderTexture(&state->renderer, canvas->origin,
                          canvas->dimension, &current->texture);
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
