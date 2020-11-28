#include <windows.h>
#include <SDL.h>
#include <stdint.h>

#include "platform_pixelate.h"
#include "math.h"
#include "platform.h"
#include "pixelate.c"

global b32 Running = 1;
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

internal void PlatfromProcessMessages(input *input)
{
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
        
    }
}