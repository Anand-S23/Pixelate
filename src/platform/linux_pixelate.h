#ifndef PLATFROM_PIXELATE_H
#define PLATFROM_PIXELATE_H

#define internal static 
#define local_persist static
#define global static 

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   b8;
typedef int16_t  b16;
typedef int32_t  b32;
typedef int64_t  b64;
typedef float    f32;
typedef double   f64;

typedef struct platform_offscreen_buffer
{
    SDL_Texture *texture;
    void *memory;
    int width;
    int height;
    int pitch;
    int bytes_per_pixel;
} platform_offscreen_buffer;

typedef struct window_dimension
{
    int width; 
    int height;
} window_dimension;

#endif