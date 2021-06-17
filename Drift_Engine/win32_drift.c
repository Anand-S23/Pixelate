#include <windows.h> 
#include <xinput.h>
#include <glad/glad.h>
#include <wglext.h>

#include "drift.h"
#include "win32_drift.h"

global drift_platform Global_Platform = {0};
global HDC Global_Device_Context;
global HGLRC Global_Render_Context;
global i64 Global_Perf_Count_Frequency;
global WINDOWPLACEMENT Global_WindowPosition = { sizeof(Global_WindowPosition) };

// X Input get state
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state); 
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED; 
}
global x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_ 

// X Input set state
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state); 
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED; 
}
global x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_ 

// App Code
inline FILETIME Win32GetLastWriteTime(char *filename)
{
    FILETIME last_write_time = {0};
    WIN32_FIND_DATA find_data;
    HANDLE find_handle = FindFirstFileA(filename, &find_data);

    if (find_handle != INVALID_HANDLE_VALUE)
    {
        FindClose(find_handle);
        last_write_time = find_data.ftLastWriteTime;
    }

    return last_write_time;
}

internal win32_app_code Win32LoadAppCode(char *src_dll, char *temp_dll)
{
    win32_app_code app_code = {0};

    app_code.dll_last_write_time = Win32GetLastWriteTime(src_dll);
    CopyFile(src_dll, temp_dll, 0);
    app_code.dll = LoadLibraryA(temp_dll);
    
    if (!app_code.dll)
    {
        return app_code;
    }
    
    app_code.Init = (init_app *)GetProcAddress(app_code.dll, "Init");
    app_code.Update = (update_app *)GetProcAddress(app_code.dll, "Update");
    app_code.DriftMain = (drift_main *)GetProcAddress(app_code.dll, "DriftMain");
    
    if (!app_code.Init || !app_code.Update|| !app_code.DriftMain)
    {
        app_code.Init = InitAppStub;
        app_code.Update = UpdateAppStub;
        app_code.DriftMain = DriftMainStub;
        return app_code;
    }

    app_code.is_valid = 1;
    return app_code;
}

internal void Win32AppCodeUnload(win32_app_code *app_code)
{
    if(app_code->dll)
    {
        FreeLibrary(app_code->dll);
    }

    app_code->Init = InitAppStub;
    app_code->Update = UpdateAppStub;
    app_code->DriftMain = DriftMainStub;
    
    app_code->dll = 0;
    app_code->is_valid = 0;
}

// Logging
internal void Win32LogInternal(int type, char *file, int line,
                               char *format, ...)
{
    char *type_message = "INFO";
    b32 message_box = 0;

    if (type & LOG_WARNING)
    {
        type_message = "WARNING";
    }
    else if (type & LOG_ERROR)
    {
        type_message = "ERROR";
        message_box = 1; 
    }

    char str_buffer[4096] = {0};
    char text[4000] = {0};

    va_list args;
    va_start(args, format);
    vsnprintf(text, 4000, format, args);
    va_end(args);

    wsprintf(str_buffer, "[%s] (%s:%d) - %s\n", type_message, file, line, text);
    OutputDebugStringA(str_buffer);

    if (message_box)
    {
        MessageBoxA(0, text, "Error", MB_OK);
    }
}

// Files
internal void Win32FreeFileMemory(void *memory)
{
    VirtualFree(memory, 0, MEM_RELEASE);
}

internal read_file_result Win32ReadFile(char *filename)
{
    read_file_result result = {0};

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
                    W32LogError("Read file fail");
                    Win32FreeFileMemory(result.memory);
                    result.memory = 0;
                }
            } 
            else 
            {
                W32LogError("Read file memory allocation failed");
            }
        } 
        else 
        {
            W32LogError("GetFileSizeEx fail");
        }

        CloseHandle(file_handle);
    } 
    else
    {
        W32LogError("Invalid file handle");
    }

    return result;
}

internal b32 Win32WriteFile(char *filename, u32 memory_size, void *memory)
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
            W32LogError("Write file fail");
        }

        CloseHandle(file_handle);
    } 
    else
    {
        W32LogError("Invalid file handle");
    }

    return result;
}

// X Input
internal void Win32LoadXInput()
{
    HMODULE xinput_library = LoadLibrary("xinput1_4.dll");
    if (!xinput_library) 
    { 
        HMODULE xinput_library = LoadLibrary("xinput1_3.dll"); 
    }

    if (xinput_library)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(xinput_library, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(xinput_library, "XInputSetState");
    }
    else 
    {
        W32LogWarning("xinput library load fail");
    }
}

// OpenGL
internal void *Win32LoadOpenGLProcedure(char *name)
{
    void *p = (void *)wglGetProcAddress(name);

    if(!p || p == (void *)0x1 || p == (void *)0x2 ||
       p == (void *)0x3 || p == (void *) - 1)
    {
        return 0;
    }

    return p;
}

PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
PFNWGLMAKECONTEXTCURRENTARBPROC wglMakeContextCurrentARB;
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

internal void Win32LoadWGLFunctions(HINSTANCE instance)
{
    wglChoosePixelFormatARB    = (PFNWGLCHOOSEPIXELFORMATARBPROC)    Win32LoadOpenGLProcedure("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) Win32LoadOpenGLProcedure("wglCreateContextAttribsARB");
    wglMakeContextCurrentARB   = (PFNWGLMAKECONTEXTCURRENTARBPROC)   Win32LoadOpenGLProcedure("wglMakeContextCurrentARB");
    wglSwapIntervalEXT         = (PFNWGLSWAPINTERVALEXTPROC)         Win32LoadOpenGLProcedure("wglSwapIntervalEXT");
}

internal b32 Win32InitOpenGL(HDC *device_context, HINSTANCE instance)
{
    b32 result = 0;
    
    PIXELFORMATDESCRIPTOR pfd = {0};
    {
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        pfd.cColorBits = 32;
        pfd.cAlphaBits = 8;
        pfd.iLayerType = PFD_MAIN_PLANE;
    }

    int format_index = ChoosePixelFormat(*device_context, &pfd);

    PIXELFORMATDESCRIPTOR pixel_format;
    DescribePixelFormat(*device_context, format_index,
                        sizeof(PIXELFORMATDESCRIPTOR), &pixel_format);

    SetPixelFormat(*device_context, format_index, &pixel_format);

    HGLRC gl_rc = wglCreateContext(*device_context);
    wglMakeCurrent(*device_context, gl_rc);

    Win32LoadWGLFunctions(instance);
    
    return result;
}

internal void Win32CleanUpOpenGL(HDC *device_context)
{
    wglMakeCurrent(*device_context, 0);
    wglDeleteContext(Global_Render_Context);
}

internal void Win32SwapBuffers(void)
{
    wglSwapLayerBuffers(Global_Device_Context, WGL_SWAP_MAIN_PLANE);
}

internal void ToggleVSync()
{
    wglSwapIntervalEXT(Global_Platform.v_sync);
}
        
// Time
inline LARGE_INTEGER Win32GetWallClock()
{
    LARGE_INTEGER result; 
    QueryPerformanceCounter(&result);
    return result;
}

inline f32 Win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    f32 result = (f32)(end.QuadPart - start.QuadPart) / (f32)Global_Perf_Count_Frequency;
    return result;
}

// Core
internal v2 GetWindowDimension(HWND window)
{
    v2 result = {0};

    RECT client_rect;
    GetClientRect(window, &client_rect);
    result.x = (float)(client_rect.right - client_rect.left);
    result.y = (float)(client_rect.bottom - client_rect.top);

    return result;
}

internal void ToggleFullscreen(HWND window)
{
    DWORD style = GetWindowLongA(window, GWL_STYLE);
    if (style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO monitor_info = { sizeof(monitor_info) };
        if (GetWindowPlacement(window, &Global_WindowPosition) &&
            GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY),
                           &monitor_info))
        {
            SetWindowLongA(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window, HWND_TOP, 
                         monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right - monitor_info.rcMonitor.left, 
                         monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top, 
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
        else 
        {
            SetWindowLongA(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
            SetWindowPlacement(window, &Global_WindowPosition);
            SetWindowPos(window,NULL, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
}

internal void LoadDriftApplicationDefaults(drift_application *app)
{
    if (!app->name)
    {
        app->name = "App";
    }

    if (!app->window_width)
    {
        app->window_width = 1280;
    }

    if (!app->window_height)
    {
        app->window_height = 720;
    }
        
    app->window_style = (!app->window_style) ?
        WS_OVERLAPPEDWINDOW : app->window_style;

    if (app->window_exact)
    {
        RECT window_rect = {0, 0, app->window_width, app->window_height};
        AdjustWindowRectEx(&window_rect, (DWORD)app->window_style, 0, 0);
        app->window_width = window_rect.right - window_rect.left;
        app->window_height = window_rect.bottom - window_rect.top;
    }
}

internal void Win32VibrateController(u8 controller_id,
                                     f32 left_motor_speed, f32 right_motor_speed)
{
    XINPUT_VIBRATION vibration;
    ZeroMemory( &vibration, sizeof(XINPUT_VIBRATION) );
    vibration.wLeftMotorSpeed = (WORD)(left_motor_speed * 65535);
    vibration.wRightMotorSpeed = (WORD)(right_motor_speed * 65535);
    XInputSetState(controller_id, &vibration);
}

internal f32 Win32ProcessInputStickValue(SHORT value, SHORT deadzone)
{
    f32 result = 0; 

    if (value < -deadzone)
    {
        result = (f32)value / 32768.0f;
    }
    else if (value > deadzone)
    {
        result = (f32)value / 32768.0f;
    }

    return result;
}

internal void Win32UpdateButtonState(button_state *new_state, 
                                     button_state *old_state, 
                                     DWORD xinput_button_state,
                                     DWORD button_bit)
{
    new_state->down = (xinput_button_state & button_bit);
    new_state->begin_down = (!old_state->down && new_state->down);
    new_state->release = (old_state->down && !new_state->down) ? 1 : 0;
}

internal void Win32ProcessControllerInput(controller_input *new_input,
                                          controller_input *old_input,
                                          int controller_index)
{
    XINPUT_STATE controller_state; 
    if (XInputGetState(controller_index, &controller_state) == ERROR_SUCCESS)
    {
        new_input->controller_id = controller_index;
        new_input->is_connected = 1;
        XINPUT_GAMEPAD *pad = &controller_state.Gamepad; 

        new_input->left_stick_x = Win32ProcessInputStickValue(
            pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        new_input->left_stick_y = Win32ProcessInputStickValue(
            pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

        new_input->right_stick_x = Win32ProcessInputStickValue(
            pad->sThumbRX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        new_input->right_stick_y = Win32ProcessInputStickValue(
            pad->sThumbRY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

        new_input->left_trigger = (f32)pad->bLeftTrigger / 255.f;
        new_input->right_trigger = (f32)pad->bRightTrigger / 255.f;

        Win32UpdateButtonState(&new_input->buttons[BUTTON_up],
                               &old_input->buttons[BUTTON_up],
                               pad->wButtons,
                               XINPUT_GAMEPAD_DPAD_UP);
        Win32UpdateButtonState(&new_input->buttons[BUTTON_down],
                               &old_input->buttons[BUTTON_down],
                               pad->wButtons,
                               XINPUT_GAMEPAD_DPAD_DOWN);
        Win32UpdateButtonState(&new_input->buttons[BUTTON_left],
                               &old_input->buttons[BUTTON_left],
                               pad->wButtons,
                               XINPUT_GAMEPAD_DPAD_LEFT);
        Win32UpdateButtonState(&new_input->buttons[BUTTON_right],
                               &old_input->buttons[BUTTON_right],
                               pad->wButtons,
                               XINPUT_GAMEPAD_DPAD_RIGHT);

        Win32UpdateButtonState(&new_input->buttons[BUTTON_start],
                               &old_input->buttons[BUTTON_start],
                               pad->wButtons,
                               XINPUT_GAMEPAD_START);
        Win32UpdateButtonState(&new_input->buttons[BUTTON_back],
                               &old_input->buttons[BUTTON_back],
                               pad->wButtons,
                               XINPUT_GAMEPAD_BACK);

        Win32UpdateButtonState(&new_input->buttons[BUTTON_left_thumb_stick],
                               &old_input->buttons[BUTTON_left_thumb_stick],
                               pad->wButtons,
                               XINPUT_GAMEPAD_LEFT_THUMB);
        Win32UpdateButtonState(&new_input->buttons[BUTTON_right_thumb_stick],
                               &old_input->buttons[BUTTON_right_thumb_stick],
                               pad->wButtons,
                               XINPUT_GAMEPAD_RIGHT_THUMB);
        Win32UpdateButtonState(&new_input->buttons[BUTTON_left_shoulder],
                               &old_input->buttons[BUTTON_left_shoulder],
                               pad->wButtons,
                               XINPUT_GAMEPAD_LEFT_SHOULDER);
        Win32UpdateButtonState(&new_input->buttons[BUTTON_right_shoulder],
                               &old_input->buttons[BUTTON_right_shoulder],
                               pad->wButtons,
                               XINPUT_GAMEPAD_RIGHT_SHOULDER);

        Win32UpdateButtonState(&new_input->buttons[BUTTON_a],
                               &old_input->buttons[BUTTON_a],
                               pad->wButtons,
                               XINPUT_GAMEPAD_A);
        Win32UpdateButtonState(&new_input->buttons[BUTTON_b],
                               &old_input->buttons[BUTTON_b],
                               pad->wButtons,
                               XINPUT_GAMEPAD_B);
        Win32UpdateButtonState(&new_input->buttons[BUTTON_x],
                               &old_input->buttons[BUTTON_x],
                               pad->wButtons,
                               XINPUT_GAMEPAD_X);
        Win32UpdateButtonState(&new_input->buttons[BUTTON_y],
                               &old_input->buttons[BUTTON_y],
                               pad->wButtons,
                               XINPUT_GAMEPAD_Y);
    }
    else
    {
        new_input->is_connected = 0;
    }
}

LRESULT CALLBACK Win32WindowProc(HWND window, UINT message, 
                                 WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0; 

    switch (message)
    {
        case WM_CLOSE: 
        case WM_DESTROY: 
        case WM_QUIT:
        {
            Global_Platform.running = 0; 
        } break;

        case WM_LBUTTONDOWN:
        {
            Global_Platform.left_mouse_down = 1;
        } break;

        case WM_LBUTTONUP:
        {
            Global_Platform.left_mouse_down = 0;
        } break;

        case WM_RBUTTONDOWN:
        {
            Global_Platform.right_mouse_down = 1;
        } break;

        case WM_RBUTTONUP:
        {
            Global_Platform.right_mouse_down = 0;
        } break;

        case WM_MBUTTONDOWN:
        {
            Global_Platform.middle_mouse_down = 1;
        } break;

        case WM_MBUTTONUP:
        {
            Global_Platform.middle_mouse_down = 0;
        } break;

        case WM_MOUSEMOVE:
        {
            POINT mouse_pos;
            GetCursorPos(&mouse_pos);
            ScreenToClient(window, &mouse_pos);
            Global_Platform.mouse_x = (f32)mouse_pos.x;
            Global_Platform.mouse_y = (f32)mouse_pos.y;
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            u32 vk_code  = (u32)w_param;
            b32 was_down = ((l_param & (1 << 30)) != 0);
            b32 is_down  = ((l_param & (1 << 31)) == 0);

            u32 key_index = 0;

            if (is_down != was_down)
            {
                if (vk_code >= 'A' && vk_code <= 'Z')
                {
                    key_index = KEY_a + (vk_code - 'A');
                }
                else if (vk_code >= '0' && vk_code <= '9')
                {
                    key_index = KEY_0 + (vk_code - '0');
                }
                else
                {
                    if (vk_code == VK_ESCAPE)
                    {
                        key_index = KEY_esc;
                    }
                    else if (vk_code >= VK_F1 && vk_code <= VK_F12)
                    {
                        key_index = KEY_f1 + vk_code - VK_F1;
                    }
                    else if (vk_code == VK_BACK)
                    {
                        key_index = KEY_backspace;
                    }
                    else if (vk_code == VK_DELETE)
                    {
                        key_index = KEY_delete;
                    }
                    else if (vk_code == VK_TAB)
                    {
                        key_index = KEY_tab;
                    }
                    else if (vk_code == VK_SPACE)
                    {
                        key_index = KEY_space;
                    }
                    else if (vk_code == VK_RETURN)
                    {
                        key_index = KEY_enter;
                    }
                    else if (vk_code == VK_CONTROL)
                    {
                        key_index = KEY_ctrl;
                    }
                    else if (vk_code == VK_SHIFT)
                    {
                        key_index = KEY_shift;
                    }
                    else if (vk_code == VK_MENU)
                    {
                        key_index = KEY_alt;
                    }
                    else if (vk_code == VK_UP)
                    {
                        key_index = KEY_up;
                    }
                    else if (vk_code == VK_LEFT)
                    {
                        key_index = KEY_left;
                    }
                    else if (vk_code == VK_DOWN)
                    {
                        key_index = KEY_down;
                    }
                    else if (vk_code == VK_RIGHT)
                    {
                        key_index = KEY_right;
                    }
                }

                Global_Platform.keys[key_index].down = is_down;
                Global_Platform.keys[key_index].begin_down = is_down;
                Global_Platform.keys[key_index].release = was_down;
            }

            b32 alt_key_was_down = ((l_param & (1 << 29)) != 0);
            if ((vk_code == VK_F4) && alt_key_was_down)
            {
                Global_Platform.running = 0;
            }
            if ((vk_code == VK_RETURN) && alt_key_was_down)
            {
                if (window)
                {
                    ToggleFullscreen(window);
                }
            }
        } break;

        default: 
        {
            result = DefWindowProcA(window, message, w_param, l_param);
        } break;
    }

    return result; 
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, 
                   LPSTR cmd_line, int cmd_show)
{
    char exe_path[256];
    char exe_directory[256];
    char src_dll_path[256];
    char temp_dll_path[256];
    DWORD exe_path_size = GetModuleFileNameA(0, exe_path, sizeof(exe_path));

    CopyMemory(exe_directory, exe_path, exe_path_size);
    char *one_past_last_slash = exe_directory;

    for (int i = 0; exe_directory[i]; ++i)
    {
        if (exe_directory[i] == '\\')
        {
            one_past_last_slash = exe_directory + i + 1;
        }
    }
    *one_past_last_slash = 0;

    wsprintf(src_dll_path, "%sdrift.dll", exe_directory);
    wsprintf(temp_dll_path, "%sdrift_temp.dll", exe_directory);

    win32_app_code app_code = Win32LoadAppCode(src_dll_path, temp_dll_path);

    WNDCLASSA window_class = {0};
    {
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc = Win32WindowProc;
        window_class.hInstance = instance;
        window_class.lpszClassName = "DriftWindowClass";
        window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    }

    b32 sleep_is_granular = (timeBeginPeriod(1) == TIMERR_NOERROR);

    LARGE_INTEGER counter_per_second;
    QueryPerformanceFrequency(&counter_per_second);
    Global_Perf_Count_Frequency = counter_per_second.QuadPart;

    if (RegisterClassA(&window_class))
    {
        drift_application app = app_code.DriftMain(&Global_Platform);
        LoadDriftApplicationDefaults(&app);
        Win32LoadXInput();

        HWND window = CreateWindowExA(0, window_class.lpszClassName, app.name,
                                      (DWORD)app.window_style, 
                                      CW_USEDEFAULT, CW_USEDEFAULT, 
                                      app.window_width, app.window_height,
                                      0, 0, instance, 0);

        if (window)
        {
            Global_Device_Context = GetDC(window);
            Win32InitOpenGL(&Global_Device_Context, instance);

            // int moniter_refresh_hz = GetDeviceCaps(Global_Device_Context, VREFRESH);
            int moniter_refresh_hz = 60;
            int game_update_hz = moniter_refresh_hz / 2;
            f32 target_fps = 1.0f / (f32)game_update_hz;

            LARGE_INTEGER last_counter = Win32GetWallClock(); 
            u64 last_cycle_count = __rdtsc();

            ShowWindow(window, cmd_show);
            UpdateWindow(window);

            v2 window_size = GetWindowDimension(window);

            // Initalize Platform
            {
                Global_Platform.running = 1;
                Global_Platform.fullscreen = app.fullscreen;
                Global_Platform.v_sync = app.v_sync;
                Global_Platform.window_width = (int)window_size.x;
                Global_Platform.window_height = (int)window_size.y;

                Global_Platform.storage_size = Megabytes(64); 
                Global_Platform.transient_storage_size = Megabytes(100);
                Global_Platform.storage =
                    VirtualAlloc(0, Global_Platform.storage_size,
                                 MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
                Global_Platform.transient_storage =
                    VirtualAlloc(0, Global_Platform.transient_storage_size,
                                 MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

                Global_Platform.SwapBuffers = Win32SwapBuffers;
                Global_Platform.ReadFile = Win32ReadFile;
                Global_Platform.FreeFileMemory = Win32FreeFileMemory;
                Global_Platform.WriteFile = Win32WriteFile;
                Global_Platform.Log = Win32LogInternal;
                Global_Platform.VibrateController = Win32VibrateController;
            }

            if (Global_Platform.fullscreen)
            {
                ToggleFullscreen(window);
            }

            if (Global_Platform.v_sync)
            {
                ToggleVSync();
            }

            app_code.Init();

            controller_input new_input[4] = {0};
            controller_input old_input[4] = {0};

            while (Global_Platform.running)
            {
                FILETIME new_dll_write_time = Win32GetLastWriteTime(src_dll_path);
                if (CompareFileTime(&new_dll_write_time, &app_code.dll_last_write_time))
                {
                    Win32AppCodeUnload(&app_code);
                    app_code = Win32LoadAppCode(src_dll_path, temp_dll_path);
                }

                Global_Platform.last_time = Global_Platform.current_time;
                Global_Platform.current_time += 1.f / target_fps;

                // Input
                for (int i = 0; i < (int)KEY_MAX; ++i)
                {
                    Global_Platform.keys[i].release = 0;
                    Global_Platform.keys[i].begin_down = 0;
                }

                for (int i = 0; i < 4; ++i)
                {
                    old_input[i] = new_input[i];
                    ZeroMemory(&new_input[i], sizeof(controller_input));

                    Win32ProcessControllerInput(&new_input[i], &old_input[i], i);
                    Global_Platform.controllers[i] = new_input[i];
                }

                Global_Platform.controller = Global_Platform.controllers[0];

                MSG message;
                while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

                app_code.Update();

                LARGE_INTEGER work_counter = Win32GetWallClock();
                f32 work_seconds_elapsed = Win32GetSecondsElapsed(work_counter, last_counter);

                f32 seconds_elapsed_for_frame = work_seconds_elapsed; 
                if (seconds_elapsed_for_frame < target_fps)
                {
                    while (seconds_elapsed_for_frame < target_fps)
                    {
                        if (sleep_is_granular)
                        {
                            DWORD sleep_ms = (DWORD)(1000.0f * (target_fps - seconds_elapsed_for_frame));
                            Sleep(sleep_ms);
                        }
                        seconds_elapsed_for_frame = Win32GetSecondsElapsed(last_counter, 
                                                                           Win32GetWallClock());
                    }
                }
                else
                {
                    // NOTE: Miss frame
                    W32LogWarning("Frame missed");
                }

                LARGE_INTEGER end_counter = Win32GetWallClock();
                u64 end_cycle_count = __rdtsc();

                i64 counter_elapsed = end_counter.QuadPart - last_counter.QuadPart;
                u64 cycle_elpased = end_cycle_count - last_cycle_count;

                i32 ms_per_frame = (i32)((1000 * counter_elapsed) / Global_Perf_Count_Frequency);
                i32 fps = (i32)(Global_Perf_Count_Frequency / counter_elapsed);
                i32 mcpf = (i32)(cycle_elpased / (1000 * 1000));
                f32 seconds_elapsed_for_work = (f32)counter_elapsed / (f32)Global_Perf_Count_Frequency;

                W32Log("%d", fps);
                last_counter = end_counter;
                last_cycle_count = end_cycle_count;
            }

            ShowWindow(window, SW_HIDE);
            ReleaseDC(window, Global_Device_Context);
            Win32CleanUpOpenGL(&Global_Device_Context);
        }
        else
        {
            W32LogError("Invaild window");
        }
    }
    else 
    {
        W32LogError("Window registeration fail");
    }

    return 0; 
}
