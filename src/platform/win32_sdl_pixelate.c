#include <windows.h>
#include <SDL.h>
#include <stdint.h>

#include "win32_sdl_pixelate.h"
#include "math.h"
#include "platform.h"
#include "pixelate.c"


global b32 Global_Running;
global platform_offscreen_buffer Global_Backbuffer; 
global i64 Global_Perf_Count_Frequency;

// TODO: Toggle fullscreen

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
            input->left_mouse_down = 
                (event->button.button == SDL_BUTTON_LEFT) ? 1 : 0; 
            input->right_mouse_down = 
                (event->button.button == SDL_BUTTON_RIGHT) ? 1 : 0; 
        } break;

        case SDL_MOUSEBUTTONUP:
        {
            input->left_mouse_down = 
                (event->button.button == SDL_BUTTON_LEFT) ? 0 : 1; 
            input->right_mouse_down = 
                (event->button.button == SDL_BUTTON_RIGHT) ? 0 : 1; 
        } break;

        case SDL_MOUSEMOTION:
        {
            SDL_GetMouseState(&input->mouse_x, 
                              &input->mouse_y);
        } break;
    }
}

inline LARGE_INTEGER Win32GetWallClock()
{
    LARGE_INTEGER result; 
    QueryPerformanceCounter(&result);
    return result;
}

inline f32 Win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    f32 result = (f32)(end.QuadPart - start.QuadPart) / (f32)Global_Perf_Count_Frequency;
    return result;
}

int main(int argc, char **argv) 
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | 
             SDL_INIT_HAPTIC | SDL_INIT_AUDIO);

    // Create our window.
    SDL_Window *window = SDL_CreateWindow("Pixelate",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          1280, 720,
                                          SDL_WINDOW_RESIZABLE);

    if (window)
    {
        LARGE_INTEGER perf_count_frequency_result; 
        QueryPerformanceFrequency(&perf_count_frequency_result);
        i64 perf_count_frequency = perf_count_frequency_result.QuadPart;

        UINT desired_scheduler_ms = 1; 
        b32 sleep_is_granular = (timeBeginPeriod(desired_scheduler_ms) == TIMERR_NOERROR);

        int moniter_refresh_hz = 60; 
        int game_update_hz = moniter_refresh_hz / 2;
        f32 target_fps = 1.0f / (f32)game_update_hz;

        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 
                                                    SDL_RENDERER_ACCELERATED);

        if (renderer)
        {
            Global_Running = 1;
            window_dimension dimension = GetWindowDimension(window);
            PlatformResizeTexture(renderer, &Global_Backbuffer, dimension.width, dimension.height);

            app_memory memory = {0};
            memory.storage_size = Megabytes(64); 
            memory.transient_storage_size = Megabytes(100);
            memory.storage = VirtualAlloc(0, memory.storage_size,
                                          MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            memory.transient_storage = VirtualAlloc(0, memory.transient_storage_size,
                                                        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            LARGE_INTEGER last_counter = Win32GetWallClock(); 
            QueryPerformanceCounter(&last_counter);
            u64 last_cycle_count = __rdtsc();

            input input = {0};

            while (Global_Running)
            {
                SDL_Event event;
                while (SDL_PollEvent(&event))
                {
                    PlatformProcessMessages(&event, &input);
                }

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

                LARGE_INTEGER work_counter = Win32GetWallClock();
                f32 work_seconds_elapsed = Win32GetSecondsElapsed(work_counter, last_counter);

                f32 seconds_elapsed_for_frame = work_seconds_elapsed; 
                if (seconds_elapsed_for_frame < target_fps)
                {
                    while (seconds_elapsed_for_frame < target_fps)
                    {
                        if (sleep_is_granular)
                        {
                            DWORD sleep_ms = (DWORD)(1000.0f * (target_fps - seconds_elapsed_for_frame));
                            Sleep(sleep_ms);
                        }
                        seconds_elapsed_for_frame = Win32GetSecondsElapsed(last_counter, 
                                                                        Win32GetWallClock());
                    }
                }
                else
                {
                    // NOTE: Miss frame
                    // TODO: Logging
                }

#if 0
                u64 cycle_elpased = end_cycle_count - last_cycle_count;
                i64 counter_elapsed = end_counter.QuadPart - last_counter.QuadPart; 
                i32 ms_per_frame = (i32)((1000 * counter_elapsed) / perf_count_frequency);
                i32 fps = (i32)(perf_count_frequency / counter_elapsed);
                i32 mcpf = (i32)(cycle_elpased / (1000 * 1000));

                char str_buffer[256];
                wsprintf(str_buffer, "%dms/f - %dFPS - %dmc/f\n", ms_per_frame, fps, mcpf);
                //OutputDebugStringA(str_buffer);
#endif

                LARGE_INTEGER end_counter = Win32GetWallClock();
                last_counter = end_counter;

                u64 end_cycle_count = __rdtsc();
                u64 cycle_elpased = end_cycle_count - last_cycle_count;
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