#ifndef RENDERER_H
#define RENDERER_h

typedef struct Texture
{
    unsigned char *data; 
    int width; 
    int height; 
    int channels;
} Texture; 

internal void ClearBuffer(offscreen_buffer *buffer);
internal void DrawFilledRect(offscreen_buffer *buffer, v2 position, v2 dimension, v3 color);
internal Texture LoadTexture(char *filename);
internal void BlitTextureToBuffer(offscreen_buffer *buffer, Texture image, v2 position);

#endif
