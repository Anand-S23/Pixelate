#include <Windows.h> 
#include <stdint.h>

#include "win32_pixelate.h"
#include "math.h"
#include "platform.h"
#include "pixelate.c"

global b32 Running = 1; 
global win32_offscreen_buffer Global_Backbuffer; 

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
                u32 vk_code = (u32)message.wParam;
                b32 was_down = ((message.lParam & (1 << 30)) != 0);
                b32 is_down = ((message.lParam & (1 << 31)) == 0);

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
        }
    }
}

internal window_dimension GetWindowDimension(HWND window)
{
    window_dimension result;

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

        case WM_SIZE: 
        {
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
    LARGE_INTEGER perf_count_frequency_result; 
    QueryPerformanceFrequency(&perf_count_frequency_result);
    i64 perf_count_frequency = perf_count_frequency_result.QuadPart;
    
    Win32ResizeDIBSection(&Global_Backbuffer, 1280, 720);
    WNDCLASSA window_class = {0};

    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = Win32WindowProc;
    window_class.hInstance = instance;
    window_class.lpszClassName = "PixelateWindowClass";

    if (RegisterClassA(&window_class))
    {
        HWND window = CreateWindowExA(0, window_class.lpszClassName, "Pixelate", 
                                      WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
                                      CW_USEDEFAULT, CW_USEDEFAULT, 
                                      1280, 720, 0, 0, instance, 0); 

        if (window)
        {
            app_memory memory = {0};
            memory.storage_size = Megabytes(64); 
            memory.storage = VirtualAlloc(0, memory.storage_size,
                                          MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            LARGE_INTEGER last_counter; 
            QueryPerformanceCounter(&last_counter);
            u64 last_cycle_count = __rdtsc();

            input input = {0};

            while (Running)
            {
                Win32ProcessMessages(&input);

                {
                    POINT point;
                    GetCursorPos(&point);
                    ScreenToClient(window, &point);
                    input.mouse_x = (f32)point.x;
                    input.mouse_y = (f32)point.y;
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

                HDC device_context = GetDC(window);
                window_dimension dimension = GetWindowDimension(window);
                Win32DisplayBufferInWindow(device_context,
                                           dimension.width, dimension.height,
                                           &Global_Backbuffer);
                ReleaseDC(window, device_context);

                u64 end_cycle_count = __rdtsc();

                LARGE_INTEGER end_counter;
                QueryPerformanceCounter(&end_counter);

                u64 cycle_elpased = end_cycle_count - last_cycle_count;
                i64 counter_elapsed = end_counter.QuadPart - last_counter.QuadPart; 
                i32 ms_per_frame = (i32)((1000 * counter_elapsed) / perf_count_frequency);
                i32 fps = (i32)(perf_count_frequency / counter_elapsed);
                i32 mcpf = (i32)(cycle_elpased / (1000 * 1000));

                char str_buffer[256];
                wsprintf(str_buffer, "%dms/f - %dFPS - %dmc/f\n", ms_per_frame, fps, mcpf);
                //OutputDebugStringA(str_buffer);

                last_counter = end_counter;
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