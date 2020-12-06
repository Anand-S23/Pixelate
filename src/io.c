#include "io.h"

#if 0
// NOTE: primitive code and will be changed
internal debug_read_file_result DEBUGPlatformReadEntireFile(char *filename)
{
    debug_read_file_result result = {0};

    HANDLE file_handle = CreateFileA(filename, GENERIC_READ, 0, 0,
                                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (file_handle != INVALID_HANDLE_VALUE) 
    {
        LARGE_INTEGER file_size;
        if (GetFileSizeEx(file_handle, &file_size)) 
        {
            u32 file_size32 = SafeTruncateUInt64(file_size.QuadPart);
            result.memory = VirtualAlloc(0, file_size32, 
                                         MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (result.memory) 
            {
                DWORD bytes_read;
                if (ReadFile(file_handle, result.memory, file_size32, &bytes_read, 0) && 
                    bytes_read == file_size32) 
                {
                    result.size = file_size32;
                } 
                else 
                {
                    // TODO: logging
                    DEBUGPlatformFreeFileMemory(result.memory);
                    result.memory = 0;
                }
            } 
            else 
            {
                // TODO: logging
            }
        } 
        else 
        {
            // TODO: logging
        }

        CloseHandle(file_handle);
    } 
    else
    {
        // TODO: logging
    }

    return result;
}

internal void DEBUGPlatformFreeFileMemory(void *memory)
{
    VirtualFree(memory, 0, MEM_RELEASE);
}

internal b32 DEBUGPlatformWriteFile(char *filename, u32 memory_size, void *memory)
{
    b32 result = 0;

    HANDLE file_handle = CreateFileA(filename, GENERIC_WRITE, 0, 0,
                                     CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if (file_handle != INVALID_HANDLE_VALUE) 
    {
        DWORD bytes_written;
        if (WriteFile(file_handle, memory, memory_size, &bytes_written, 0))
        {
            result = (bytes_written == memory_size);
        } 
        else 
        {
            // TODO: logging
        }

        CloseHandle(file_handle);
    } 
    else
    {
        // TODO: logging
    }

    return result;
}

inline FILETIME Win32GetLastWriteTime(const char *filename)
{
    FILETIME result = {0};

    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx(filename, GetFileExInfoStandard, &data))
    {
        result = data.ftLastWriteTime;
    }
    return result;
}

internal win32_app_code Win32LoadGameCode(char *source_dll, char *temp_dll, char *lock_file)
{
    win32_app_code result = {0};

    WIN32_FILE_ATTRIBUTE_DATA ignored;
    if (!GetFileAttributesEx(lock_file, GetFileExInfoStandard, &ignored))
    {
        result.last_dll_write_time = Win32GetLastWriteTime(source_dll);

        CopyFile(source_dll, temp_dll, FALSE);
        result.dll = LoadLibraryA(temp_dll);
        if (result.dll)
        {
            result.UpdateAndRender = (game_update_and_render *)
                GetProcAddress(result.dll, "GameUpdateAndRender");

            result.is_valid = (result.UpdateAndRender && result.GetSoundSamples);
        }
    }
    if (!result.is_valid)
    {
        result.UpdateAndRender = 0;
        result.GetSoundSamples = 0;
    }
    return result;
}

internal void Win32UnloadGameCode(win32_app_code *app_code)
{
    if (app_code->dll)
    {
        FreeLibrary(app_code->dll);
        app_code->dll = 0;
    }

    app_code->is_valid = 0;
    app_code->UpdateAndRender = 0;
    app_code->GetSoundSamples = 0;
}
#endif 