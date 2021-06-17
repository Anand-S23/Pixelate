#ifndef IO_H
#define IO_H

typedef struct read_file_result
{
    void *memory;
    u32 size;
} read_file_result;

internal read_file_result PlatformReadFile(char *filename);
internal void PlatformFreeFileMemory(void *memory);
internal b32 PlatformWriteFile(char *filename, u32 memory_size, void *memory);

#endif