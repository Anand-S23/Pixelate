#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "renderer.h"

internal void ClearBuffer(offscreen_buffer *buffer)
{
    int width = buffer->width;
    int height = buffer->height;
    
    u8 *row = (u8 *)buffer->memory;
    for (int y = 0; y < height; ++y)
    {
        u32 *pixel = (u32 *)row;
        for (int x = 0; x < width; ++x)
        {
            u8 blue = (u8)0;
            u8 green = (u8)0;
            u8 red = (u8)0;

            *pixel++ = ((red << 16) | (green << 8) | blue);
        }

        row += buffer->pitch;
    }
}

internal void DrawFilledRect(offscreen_buffer *buffer,
                             v4 rect, v4 color)
{
    i32 min_x = Max(0, (i32)rect.x);
    i32 min_y = Max(0, (i32)rect.y);
    i32 max_x = Min((buffer->width), (min_x + (i32)rect.width));
    i32 max_y = Min((buffer->height), (min_y + (i32)rect.height));
    
    u8 *row = (u8 *)buffer->memory + 
              min_x * buffer->bytes_per_pixel + 
              min_y * buffer->pitch; 

    for(i32 j = min_y; j < max_y; ++j)
    {
        u32 *pixel = (u32 *)row;
        for(i32 i = min_x; i < max_x; ++i)
        {
            *pixel++ = (((u8)color.a << 24) | 
                        ((u8)color.r << 16) | 
                        ((u8)color.g <<  8) | 
                         (u8)color.b);
        }

        row += buffer->pitch; 
    }
}

internal void DrawColorPicker(offscreen_buffer *buffer, 
                              v2 position, v2 dimension, v3 actual_color)
{
    i32 min_x = Max(0, (i32)position.x);
    i32 min_y = Max(0, (i32)position.y);
    i32 max_x = Min((buffer->width), (min_x + (i32)dimension.width));
    i32 max_y = Min((buffer->height), (min_y + (i32)dimension.height));

    
    u8 *row = (u8 *)buffer->memory + (i32)position.x*buffer->bytes_per_pixel + (i32)position.y*buffer->pitch; 
    for(i32 j = min_y; j < max_y; ++j)
    {
        local_persist v3 color = {0};
        color.r = 255 - (f32)j;
        color.g = 255 - (f32)j;
        color.b = 255 - (f32)j;

        u32 *pixel = (u32 *)row;
        for(i32 i = min_x; i < max_x; ++i)
        {
            *pixel++ = (((u8)color.r << 16) | ((u8)color.g << 8) | (u8)color.b);
            color.g = (--color.g < 0) ? 0 : --color.g;
            color.b = (--color.b < 0) ? 0 : --color.b;
        }

        row += buffer->pitch; 
    }
}

internal Texture LoadTexture(char *filename)
{
    int width, height, channels;

    Texture texture = {0};
    texture.data = (unsigned char *)stbi_load(filename, &width, &height, &channels, 3);
    texture.width = width;
    texture.height = height;
    texture.channels = channels;

    return texture; 
}

internal void BlitTextureToBuffer(offscreen_buffer *buffer, 
                                  Texture image, v2 position)
{
    i32 min_x = Max(0, (i32)position.x);
    i32 min_y = Max(0, (i32)position.y);
    i32 max_x = Min((buffer->width), (min_x + (i32)image.width));
    i32 max_y = Min((buffer->height), (min_y + (i32)image.height));

    u8 *row = (u8 *)buffer->memory + (i32)position.x*buffer->bytes_per_pixel + (i32)position.y*buffer->pitch; 
    for(i32 j = min_y; j < max_y; ++j)
    {
        u32 *pixel = (u32 *)row;
        for(i32 i = min_x; i < max_x; ++i)
        {
            unsigned char *pixel_offset = image.data + (i + image.width * j) * image.channels;
            unsigned char r = pixel_offset[0];
            unsigned char g = pixel_offset[1];
            unsigned char b = pixel_offset[2];

            *pixel = ((r << 16) | (g << 8) | b);
        }

        row += buffer->pitch;
    }
}