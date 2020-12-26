#include <Windows.h> 
#include <stdint.h>

#include "win32_pixelate.h"
#include "math.h"
#include "platform.h"
#include "pixelate.c"

global b32 Running; 
global win32_offscreen_buffer Global_Backbuffer; 
global i64 Global_Perf_Count_Frequency;
global b32 DEBUGGlobalShowCursor;
global WINDOWPLACEMENT Global_WindowPosition = { sizeof(Global_WindowPosition) };

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
            SetWindowPos(window, 0, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
}

internal void Win32ProcessMessages(input *input)
{
    MSG message; 
    while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
    {
        switch (message.message)
        {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                u32 vk_code  = (u32)message.wParam;
                b32 was_down = ((message.lParam & (1 << 30)) != 0);
                b32 is_down  = ((message.lParam & (1 << 31)) == 0);

                u32 key_index = 0;

                if (is_down != was_down)
                {
                    if(vk_code >= 'A' && vk_code <= 'Z')
                    {
                        key_index = KEY_a + (vk_code - 'A');
                    }
                    else if (vk_code >= '0' && vk_code <= '9')
                    {
                        key_index = KEY_0 + (vk_code - '0');
                    }
                }

                b32 alt_key_was_down = ((message.lParam & (1 << 29)) != 0);
                if ((vk_code == VK_F4) && alt_key_was_down)
                {
                    Running = 0; 
                }
                if ((vk_code == VK_RETURN) && alt_key_was_down)
                {
                    if (message.hwnd)
                    {
                        ToggleFullscreen(message.hwnd);
                    }
                }
            } break; 

            case WM_MOUSEWHEEL:
            {
                input->prev_scroll_value = input->scroll_value;
                input->scroll_value = (int)GET_WHEEL_DELTA_WPARAM(message.wParam);
            } break;

            default: 
            {
                TranslateMessage(&message);
                DispatchMessageA(&message); 
            } break;

            input->prev_scroll_value = input->scroll_value;
        }
    }
}

internal window_dimension GetWindowDimension(HWND window)
{
    window_dimension result = {0};

    RECT client_rect;
    GetClientRect(window, &client_rect);
    result.width = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;

    return result;
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *buffer, 
                                    int width, int height)
{
    if (buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_FREE);
    }

    buffer->width = width; 
    buffer->height = height; 
    buffer->bytes_per_pixel = 4;

    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = -buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    int memory_size = buffer->width * buffer->height * buffer->bytes_per_pixel;
    buffer->memory = VirtualAlloc(0, memory_size, MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = buffer->width * buffer->bytes_per_pixel;
}

internal void Win32DisplayBufferInWindow(HDC device_context, 
                                         int window_width, int window_height, 
                                         win32_offscreen_buffer *buffer)
{
    StretchDIBits(device_context, 
                  0, 0, window_width, window_height,
                  0, 0, buffer->width, buffer->height, 
                  buffer->memory, &buffer->info, 
                  DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32WindowProc(HWND window, UINT message, 
                                 WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0; 

    switch (message)
    {
        case WM_CLOSE: 
        case WM_DESTROY: 
        {
            Running = 0; 
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            Assert("Keyboard input detected here");
        } break; 

        case WM_PAINT: 
        {
            window_dimension dimension = GetWindowDimension(window);

            PAINTSTRUCT paint; 
            HDC device_context = BeginPaint(window, &paint);
            Win32DisplayBufferInWindow(device_context, 
                                       dimension.width, dimension.height, 
                                       &Global_Backbuffer);
            EndPaint(window, &paint);
        } break;

        case WM_SIZE: 
        {
            int width  = (int)LOWORD(l_param);
            int height = (int)HIWORD(l_param);
            Win32ResizeDIBSection(&Global_Backbuffer, width, height);
        } break;

        default: 
        {
            result = DefWindowProcA(window, message, w_param, l_param);
        } break;
    }

    return result; 
}

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

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, 
                   LPSTR cmd_line, int cmd_show)
{
    LARGE_INTEGER perf_count_frequency_result; 
    QueryPerformanceFrequency(&perf_count_frequency_result);
    i64 perf_count_frequency = perf_count_frequency_result.QuadPart;

    UINT desired_scheduler_ms = 1; 
    b32 sleep_is_granular = (timeBeginPeriod(desired_scheduler_ms) == TIMERR_NOERROR);
    
    WNDCLASSA window_class = {0};

    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = Win32WindowProc;
    window_class.hInstance = instance;
    window_class.lpszClassName = "PixelateWindowClass";
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);

    int moniter_refresh_hz = 60; 
    int game_update_hz = moniter_refresh_hz / 2;
    f32 target_fps = 1.0f / (f32)game_update_hz;

    if (RegisterClassA(&window_class))
    {
        HWND window = CreateWindowExA(0, window_class.lpszClassName, "Pixelate", 
                                      WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
                                      CW_USEDEFAULT, CW_USEDEFAULT, 
                                      1280, 720, 0, 0, instance, 0);

        if (window)
        {
            // ToggleFullscreen(window);

            DWORD dw_style = GetWindowLongPtr(window, GWL_STYLE);
            DWORD dw_ex_style = GetWindowLongPtr( window, GWL_EXSTYLE);
            RECT rect = { 0, 0, 1280, 720 };
            AdjustWindowRectEx(&rect, dw_style, 0, dw_ex_style);
            SetWindowPos(window, NULL, 0, 0, rect.right - rect.left, 
                         rect.bottom - rect.top, 
                         SWP_NOZORDER | SWP_NOMOVE);
            
            window_dimension dim = GetWindowDimension(window);
            char str_buffer[256];
            wsprintf(str_buffer, "%d %d %d %d\n", Global_Backbuffer.width, Global_Backbuffer.height, dim.width, dim.height);
            OutputDebugStringA(str_buffer);

            app_memory memory = {0};
            memory.storage_size = Megabytes(64); 
            memory.transient_storage_size = Megabytes(100);
            memory.storage = VirtualAlloc(0, memory.storage_size,
                                          MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            memory.transient_storage = VirtualAlloc(0, memory.transient_storage_size,
                                                        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            LARGE_INTEGER last_counter = Win32GetWallClock(); 
            QueryPerformanceCounter(&last_counter);
            u64 last_cycle_count = __rdtsc();

            input input = {0};

            Running = 1;

            while (Running)
            {
                Win32ProcessMessages(&input);

                {
                    POINT point;
                    GetCursorPos(&point);
                    ScreenToClient(window, &point);
                    input.mouse_x = (u32)point.x;
                    input.mouse_y = (u32)point.y;
                    input.left_mouse_down = GetKeyState(VK_LBUTTON) & (1 << 15);
                    input.right_mouse_down = GetKeyState(VK_RBUTTON) & (1 << 15);
                    input.middle_mouse_down = GetKeyState(VK_MBUTTON) & (1 << 15);
                }

                offscreen_buffer buffer = {0};
                {
                    buffer.memory = Global_Backbuffer.memory;
                    buffer.width = Global_Backbuffer.width; 
                    buffer.height = Global_Backbuffer.height; 
                    buffer.pitch = Global_Backbuffer.pitch; 
                    buffer.bytes_per_pixel = Global_Backbuffer.bytes_per_pixel;
                }

                UpdateApp(&memory, &buffer, &input);

                window_dimension dimension = GetWindowDimension(window);

                HDC device_context = GetDC(window);
                Win32DisplayBufferInWindow(device_context,
                                           dimension.width, dimension.height,
                                           &Global_Backbuffer);
                ReleaseDC(window, device_context);

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
                    // TODO: Logging
                }

#if 0
                u64 cycle_elpased = end_cycle_count - last_cycle_count;
                i64 counter_elapsed = end_counter.QuadPart - last_counter.QuadPart; 
                i32 ms_per_frame = (i32)((1000 * counter_elapsed) / perf_count_frequency);
                i32 fps = (i32)(perf_count_frequency / counter_elapsed);
                i32 mcpf = (i32)(cycle_elpased / (1000 * 1000));

                char str_buffer[256];
                wsprintf(str_buffer, "%dms/f - %dFPS - %dmc/f\n", ms_per_frame, fps, mcpf);
                //OutputDebugStringA(str_buffer);
#endif

                LARGE_INTEGER end_counter = Win32GetWallClock();
                last_counter = end_counter;

                u64 end_cycle_count = __rdtsc();
                u64 cycle_elpased = end_cycle_count - last_cycle_count;
                last_cycle_count = end_cycle_count;
            }
        }
        else
        {
            // TODO: Logging
        }
    }
    else 
    {
        // TODO: Logging
    }

    return 0; 
}