#ifndef IO_H
#define IO_H

typedef struct debug_read_file_result
{
    void *memory;
    u32 size;
} debug_read_file_result;

internal debug_read_file_result DEBUGPlatformReadEntireFile(char *filename);
internal void DEBUGPlatformFreeFileMemory(void *memory);
internal b32 DEBUGPlatformWriteFile(char *filename, u32 memory_size, void *memory);

#endif
