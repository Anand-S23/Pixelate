#include "renderer.c"
#include "memory.c"
#include "ui.c"
#include "io.c"

#include "pixelate.h"
#include "linked_list.c"

internal int GetIndexFromClick(v2 app_cursor, int width, int height, int cell_dim)
{
    int i = ((int)app_cursor.x - (int)app_cursor.x % cell_dim) / cell_dim; 
    int j = ((int)app_cursor.y - (int)app_cursor.y % cell_dim) / cell_dim; 

    return (i >= 0 && j >= 0 && i <= width && j <= height) ? i + j * width : -1;
}

internal u32 *CreateLayer(app_state *state)
{
    u32 *result = AllocateMemoryArena(&state->permanent_arena, 
                                      state->canvas.width * 
                                      state->canvas.height * 
                                      sizeof(u32));
    
    return result;
}

internal void CreateCanvas(app_state *state, int buffer_width, 
                           int buffer_height, b32 new)
{
    if (new)
    {
        state->canvas.layers = CreateLinkedList();
        node *inital_layer = Push(&state->canvas.layers, CreateLayer(state));
        state->active_layer = inital_layer;
    }

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
        UIPannel(&state->ui, UIIDGen(), "Create a new image", v4(100, 100, 380, 280));
        {
            local_persist int canvas_width = 64;
            local_persist int canvas_height = 64;
            
            if (UIPannelButton(&state->ui, UIIDGen(), "Ok", v4(0, 0, 75, 30)))
            {
                state->canvas.width  = canvas_width; 
                state->canvas.height = canvas_height; 
                CreateCanvas(state, buffer->width, buffer->height, 1);

                state->current_mode = CANVAS_MODE_edit;
            }
        }
        UIEndPannel(&state->ui);
    }
    UIEndFrame(&state->ui);
}

internal void SaveCanvasData(app_state *state)
{
    pixel_buffer write_buffer = {0};

    // width, height, # of layers, layer data
    int num_layers = state->canvas.layers.size;
    write_buffer.size = (3 * sizeof(int)) + 
                        (state->canvas.width * state->canvas.height * 
                         num_layers * sizeof(u32));

    write_buffer.position = 0;
    write_buffer.data = AllocateMemoryArena(&state->transient_arena, write_buffer.size);

    memcpy((int *)write_buffer.data + write_buffer.position, (void *)&state->canvas.width, sizeof(int));
    write_buffer.position += sizeof(int);

    memcpy((int *)write_buffer.data + write_buffer.position, (void *)&state->canvas.height, sizeof(int));
    write_buffer.position += sizeof(int);

    memcpy((int *)write_buffer.data + write_buffer.position, (void *)&num_layers, sizeof(int));
    write_buffer.position += sizeof(int);

    for (int n = 0; n < num_layers; ++n)
    {
        u32 *layer = Get(&state->canvas.layers, n);
        for (int j = 0; j < state->canvas.height; ++j)
        {
            for (int i = 0; i < state->canvas.width; ++i)
            {
                u32 pixel_color = layer[i + j * state->canvas.width];

                memcpy((u32 *)write_buffer.data + write_buffer.position, 
                       (void *)&pixel_color, sizeof(u32));
                write_buffer.position += sizeof(u32);
            }
        }
    }

    // TODO: Let user choose save location
    PlatformWriteFile("data/test.bin", write_buffer.size, write_buffer.data);
}

internal void LoadCanvasData(app_state *state, 
                             int buffer_width, int buffer_height)
{
    // TODO: Let user choose load file
    read_file_result file_read_buffer = 
        PlatformReadFile("W:\\pixelate\\data\\test.bin");

    int num_layers;
    u32 position = 0;

    *(int *)&state->canvas.width = 
        *((int *)file_read_buffer.memory + position);
    position += sizeof(int);

    *(int *)&state->canvas.height = 
        *((int *)file_read_buffer.memory + position);
    position += sizeof(int);

    *(int *)&num_layers = 
        *((int *)file_read_buffer.memory + position);
    position += sizeof(int);

    for (int n = 0; n < num_layers; ++n)
    {
        u32 *layer = CreateLayer(state);
        for (int j = 0; j < state->canvas.height; ++j)
        {
            for (int i = 0; i < state->canvas.width; ++i)
            {
                u32 pixel_color;

                *(u32 *)&pixel_color = *((u32 *)file_read_buffer.memory + position);
                position += sizeof(u32);

                layer[i + j * state->canvas.width] = pixel_color;
            }
        }
        Append(&state->canvas.layers, layer);
    }

    PlatformFreeFileMemory(file_read_buffer.memory);
    state->active_layer = state->canvas.layers.head;
    CreateCanvas(state, buffer_width, buffer_height, 0);
}

internal void WriteCanvasDataToPNG(app_state *state)
{
}

internal void UpdateApp(app_memory *memory, offscreen_buffer *buffer, input *input)
{
    Assert(sizeof(app_state) <= memory->storage_size);

    app_state *state = (app_state *)memory->storage;
    if (!memory->initialized)
    {
        state->permanent_arena = 
            InitMemoryArena(memory->storage, memory->storage_size);
        state->transient_arena = 
            InitMemoryArena(memory->transient_storage, memory->transient_storage_size);
        AllocateMemoryArena(&state->permanent_arena, sizeof(app_state));

        state->current_mode = CANVAS_MODE_blank;

        memory->initialized = 1;
    }

    // Get mouse input in relation to the canvas
    state->canvas.cursor.x = input->mouse_x - state->canvas.origin.x;
    state->canvas.cursor.y = input->mouse_y - state->canvas.origin.y;

    // Move canvas around
    if (input->middle_mouse_down && 
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

    // Scale canvas
    if (input->scroll_value == input->prev_scroll_value && 
        state->current_mode == CANVAS_MODE_edit)
    {
        input->scroll_delta = 0; 
    }
    else 
    {
        input->scroll_delta = input->scroll_value - input->prev_scroll_value;
    }

    // Process drawing and erasing 
    if (input->left_mouse_down && 
        state->current_mode == CANVAS_MODE_edit)
    {
        int index = GetIndexFromClick(state->canvas.cursor, 
                                      state->canvas.width, 
                                      state->canvas.height, 
                                      state->camera.scale);
        if (index >= 0)
        {
            state->active_layer->buffer[index] = u32_color(state->canvas.current_color); 
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
            state->active_layer->buffer[index] = u32_color(v4(0, 0, 0, 0)); 
        }
    }

    ClearBuffer(buffer, v4(0.22f, 0.22f, 0.24f, 1.f));

    if (state->current_mode == CANVAS_MODE_edit)
    {
        for (int n = 0; n < state->canvas.layers.size; ++n)
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

                    DrawFilledRect(buffer, 
                                   v4(state->canvas.origin.x + i * state->camera.scale, 
                                      state->canvas.origin.y + j * state->camera.scale, 
                                      state->camera.scale, state->camera.scale), color);

                    DrawFilledRectU32(buffer, 
                                      v4(state->canvas.origin.x + i * state->camera.scale, 
                                         state->canvas.origin.y + j * state->camera.scale,
                                         state->camera.scale, state->camera.scale), 
                                         layer[i + j * state->canvas.width]);
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
    DrawFilledRect(buffer, v4(0, 0, buffer->width, 20), v4(0.5f, 0.5f, 0.5f, 1.f));
    DrawFilledRect(buffer, v4(0, 1, buffer->width, 20), v4(0.8f, 0.8f, 0.8f, 1.f));

    UIBeginFrame(&state->ui, buffer, input);
    {
        if (UIButtonMenu(&state->ui, UIIDGen(), "Pixelate Menu",  
                         v4(5, 1, 60, 20), v4(5, 20, 100, 20)))
        {
            if (UIButton(&state->ui, UIIDGen(), "New Canvas"))
            {
                if (state->current_mode == CANVAS_MODE_blank)
                {
                    state->current_mode = CANVAS_MODE_create;
                }
            }

            if (UIButton(&state->ui, UIIDGen(), "Save"))
            {
                if (state->current_mode == CANVAS_MODE_edit)
                {
                    SaveCanvasData(state);
                }
            }

            if (UIButton(&state->ui, UIIDGen(), "Load"))
            {
                if (state->current_mode == CANVAS_MODE_blank ||
                    state->current_mode == CANVAS_MODE_create)
                {
                    LoadCanvasData(state, buffer->width, buffer->height);
                    state->current_mode = CANVAS_MODE_edit;
                }
                else if (state->current_mode == CANVAS_MODE_edit)
                {
                    // TODO: Ask to save
                    LoadCanvasData(state, buffer->width, buffer->height);
                }
            }

            if (UIButton(&state->ui, UIIDGen(), "Export"))
            {
            }
        }
        UIPopColumn(&state->ui);
    }
    UIEndFrame(&state->ui);

}