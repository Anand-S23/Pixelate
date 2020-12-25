#ifndef PLATFORM_H
#define PLATFORM_H

#if PIXELATE_SLOW
#define Assert(expr) if(!(expr)) {*(int *)0 = 0;}
#else 
#define Assert(expr)
#endif

inline u32 SafeTruncateUInt64(u64 value)
{
    Assert(value <= 0xFFFFFFFF);
    return (u32)value;
}

typedef struct offscreen_buffer
{
    void *memory;
    int width; 
    int height; 
    int pitch; 
    int bytes_per_pixel;
} offscreen_buffer;

enum {
#define Key(name, str) KEY_##name,
#include "platform_key_list.h"
    KEY_MAX
};

typedef struct input
{
    b32 key_down[KEY_MAX];
    int mouse_x;
    int mouse_y;
    b32 left_mouse_down;
    b32 right_mouse_down;
    b32 middle_mouse_down;
    int prev_scroll_value; 
    int scroll_value;
    int scroll_delta;
} input;

typedef struct app_memory
{
    void *storage;
    u64 storage_size; 
    void *transient_storage; 
    u32 transient_storage_size;

    b32 initialized;
} app_memory;

#endif