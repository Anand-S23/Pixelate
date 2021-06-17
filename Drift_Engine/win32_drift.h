#ifndef WIN32_DRIFT_H
#define WIN32_DRIFT_H

typedef struct win32_app_code
{
    init_app *Init;
    update_app *Update;
    drift_main *DriftMain;

    HMODULE dll;
    FILETIME dll_last_write_time;
    b32 is_valid;
} win32_app_code;

internal void Win32LogInternal(int type, char *file,
                              int line, char *format, ...);

#define _W32Log(...)
#define W32Log(...) Win32LogInternal(LOG, __FILENAME__, __LINE__, __VA_ARGS__)
#define W32LogWarning(...) Win32LogInternal(LOG_WARNING, __FILENAME__, __LINE__, __VA_ARGS__)
#define W32LogError(...) Win32LogInternal(LOG_ERROR, __FILENAME__, __LINE__, __VA_ARGS__)

#endif
