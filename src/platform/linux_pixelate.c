#include <windows.h> // NOTE: Only for testing purposes
#include <SDL.h>
#include <stdint.h>

#include "platform_pixelate.h"
#include "math.h"
#include "platform.h"
#include "pixelate.c"

global b32 Global_Running;
global platform_offscreen_buffer Global_Backbuffer; 

internal window_dimension GetWindowDimension(SDL_Window *window)
{
    window_dimension result;
    SDL_GetWindowSize(window, &result.width, &result.height);
    return result;
}

internal void PlatformResizeTexture(SDL_Renderer *renderer, 
                                    platform_offscreen_buffer *buffer,
                                    int width, int height)
{
    if (buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_FREE);
    }

    if (buffer->texture)
    {
        SDL_DestroyTexture(buffer->texture);
    }

    buffer->texture = SDL_CreateTexture(renderer,
                                        SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        width, height);

    buffer->width = width; 
    buffer->height = height;
    buffer->bytes_per_pixel = 4;

    int memory_size = buffer->width * buffer->height * buffer->bytes_per_pixel;
    buffer->memory = VirtualAlloc(0, memory_size, MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = buffer->width * buffer->bytes_per_pixel;
}

internal void PlatformDisplayBufferInWindow(SDL_Window *window, 
                                            SDL_Renderer *renderer,
                                            platform_offscreen_buffer *buffer)
{
    SDL_UpdateTexture(buffer->texture, 0,
                      buffer->memory, buffer->pitch);
    SDL_RenderCopy(renderer, buffer->texture, 0, 0);
    SDL_RenderPresent(renderer);
}

internal void PlatformProcessMessages(SDL_Event *event, input *input)
{
    switch (event->type)
    {
        case SDL_QUIT:
        {
            Global_Running = 0;
        } break;
        
        case SDL_KEYUP:
        case SDL_KEYDOWN:
        {
            if (event->key.repeat == 0)
            {
                switch (event->key.keysym.scancode)
                {
                    case SDL_SCANCODE_SPACE:
                    {
                    } break;

                    case SDL_SCANCODE_ESCAPE:
                    {
                    } break;
                }
           }
        } break;

        case SDL_MOUSEBUTTONDOWN:
        {
        } break;

        case SDL_MOUSEBUTTONUP:
        {
        } break;

        case SDL_MOUSEMOTION:
        {
        //    SDL_GetMouseState(&Global_State.input.mouse_x, 
        //                      &Global_State.input.mouse_y);
        } break;
    }
}

int main(int argc, char **argv) 
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | 
             SDL_INIT_HAPTIC | SDL_INIT_AUDIO);

    u64 PerfCountFrequency = SDL_GetPerformanceFrequency();

    // Create our window.
    SDL_Window *window = SDL_CreateWindow("Pixelate",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          1280, 720,
                                          SDL_WINDOW_RESIZABLE);

    if (window)
    {
        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 
                                                    SDL_RENDERER_ACCELERATED);

        if (renderer)
        {
            Global_Running = 1;
            window_dimension dimension = GetWindowDimension(window);
            PlatformResizeTexture(renderer, &Global_Backbuffer, dimension.width, dimension.height);

            app_memory memory = {0};
            memory.storage_size = Megabytes(64); 
            memory.storage = VirtualAlloc(0, memory.storage_size,
                                          MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            u64 last_counter = SDL_GetPerformanceCounter();
            u64 last_cycle_count = __rdtsc();

            input input = {0};

            while (Global_Running)
            {
                SDL_Event event;
                while (SDL_PollEvent(&event))
                {
                    PlatformProcessMessages(&event, &input);
                }


                /*
                {
                    POINT point;
                    GetCursorPos(&point);
                    ScreenToClient(window, &point);
                    input.mouse_x = (f32)point.x;
                    input.mouse_y = (f32)point.y;
                    input.left_mouse_down = GetKeyState(VK_LBUTTON) & (1 << 15);
                    input.right_mouse_down = GetKeyState(VK_RBUTTON) & (1 << 15);
                    input.middle_mouse_down = GetKeyState(VK_MBUTTON) & (1 << 15);
                }
                */

                offscreen_buffer buffer = {0};
                {
                    buffer.memory = Global_Backbuffer.memory;
                    buffer.width = Global_Backbuffer.width; 
                    buffer.height = Global_Backbuffer.height; 
                    buffer.pitch = Global_Backbuffer.pitch; 
                    buffer.bytes_per_pixel = Global_Backbuffer.bytes_per_pixel;
                }

                UpdateApp(&memory, &buffer, &input);

                PlatformDisplayBufferInWindow(window, renderer, 
                                              &Global_Backbuffer);

                u64 end_cycle_count = __rdtsc();
                u64 end_counter = SDL_GetPerformanceCounter();

#if 0
                u64 cycle_elpased = end_cycle_count - last_cycle_count;
                i64 counter_elapsed = end_counter.QuadPart - last_counter.QuadPart; 
                i32 ms_per_frame = (i32)((1000 * counter_elapsed) / perf_count_frequency);
                i32 fps = (i32)(perf_count_frequency / counter_elapsed);
                i32 mcpf = (i32)(cycle_elpased / (1000 * 1000));

                printf(str_buffer, "%dms/f - %dFPS - %dmc/f\n", ms_per_frame, fps, mcpf);
#endif

                last_counter = end_counter;
                last_cycle_count = end_cycle_count;
            }
        }
        else
        {
            // TODO: Logging
        }
    }
    else
    {
        // TODO: Logging
    }

    SDL_Quit();
    return 0;
}