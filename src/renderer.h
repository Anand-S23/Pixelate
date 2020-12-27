#ifndef RENDERER_H
#define RENDERER_h

typedef struct Texture
{
    unsigned char *data; 
    int width; 
    int height; 
    int channels;
} Texture; 

internal u32 V4ToU32(v4 vec)
{
    u8 r = (u8)(vec.r * 255.f);
    u8 g = (u8)(vec.g * 255.f);
    u8 b = (u8)(vec.b * 255.f);
    u8 a = (u8)(vec.a * 255.f);

    u32 result = ((a << 24) | (r << 16) | (g << 8) | b);

    return result;
}
#define u32_color(v) V4ToU32(v)

internal void ClearBuffer(offscreen_buffer *buffer, v4 color);
internal void DrawFilledRect(offscreen_buffer *buffer, v4 rect, v4 color);
internal Texture LoadTexture(char *filename);
internal void BlitTextureToBuffer(offscreen_buffer *buffer, Texture image, v2 position);

#endif
