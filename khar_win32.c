/* TODO:
 * [ ] Proper multi-window support.
 * [ ] Per window data. (Required for proper cursor image support and other things)
 */

#include "khar.h"

#include <locale.h>
#include <stdlib.h>
#include <wchar.h>


khar_win32_t khar_win32 = {0};


void khar_display_error(const char *message)
{
    MessageBoxA(NULL, message, "Khar", MB_ICONERROR);
    exit(1);
}


static void khar_win32_update_cursor_image(void)
{
    SetCursor(LoadCursor(NULL, IDC_ARROW));
}


static int khar_win32_register_leave_event(HWND window)
{
    TRACKMOUSEEVENT track_mouse_event = {
        .cbSize = sizeof(track_mouse_event),
        .dwFlags = TME_LEAVE,
        .hwndTrack = window,
        .dwHoverTime = HOVER_DEFAULT,
    };

    return TrackMouseEvent(&track_mouse_event);
}


LRESULT CALLBACK win32_window_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param)
{
    if (khar_win32.ignoring_events) {
        if (u_msg == WM_EXITSIZEMOVE) {
            khar_win32.ignoring_events = 0;
        }

        return (DefWindowProc(hwnd, u_msg, w_param, l_param));
    }

    int res = 0;

    switch (u_msg) {
        case WM_ENTERSIZEMOVE: {
            khar_win32.ignoring_events = 1;
        } break;

        case WM_MOUSEMOVE: {
            if (!khar_win32.cursor_in_client) {
                if (khar_win32_register_leave_event(hwnd)) {
                    khar_win32.cursor_in_client = 1;
                }

                khar_win32_update_cursor_image();
            }
        } break;

        case WM_MOUSELEAVE: {
            khar_win32.cursor_in_client = 0;
        } break;

#ifdef KHAR_USE_RESIZE_EVENTS
        case WM_WINDOWPOSCHANGED: {
            unsigned win_flags = ((WINDOWPOS*)l_param)->flags;

            if (win_flags & SWP_NOSIZE)
                return (DefWindowProc(hwnd, u_msg, w_param, l_param));
        }
#endif
#ifdef KHAR_USE_WHEEL_EVENTS
        case WM_MOUSEWHEEL:
#endif
#ifdef KHAR_USE_MOTION_EVENTS
        case WM_INPUT:
#endif
#ifdef KHAR_USE_BUTTON_EVENTS
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
#endif
#ifdef KHAR_USE_KEY_EVENTS
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
#endif
        case WM_CLOSE:
            // Since we are discarding events that occur in resize mode, this shouldn't
            // happen too often. But in case the buffer fills up we simply discard overflow.
            // Does happen with certain things like message boxes and menus.
            if (khar_win32.msg_count == KHAR_WIN32_MAX_BUFFERED_EVENTS) {
                return (DefWindowProc(hwnd, u_msg, w_param, l_param));
            }

            khar_win32.msg_buffer[khar_win32.msg_count++] = (khar_win32_msg_t) {
                .hwnd = hwnd,
                .u_msg = u_msg,
                .w_param = w_param,
                .l_param = l_param,
            };
            break;

        default:
            return (DefWindowProc(hwnd, u_msg, w_param, l_param));
    }

    return res;
}


int khar_poll_events(khar_event_t *event)
{
poll_start:
    do {
        if (khar_win32.msg_pos == khar_win32.msg_count) {
            khar_win32.msg_count = 0;
            khar_win32.msg_pos   = 0;

            MSG msg;

            if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                return 0;

            if (msg.message == WM_QUIT) {
                event->type = khar_event_type_Quit;
                event->window = 0;
                return 1;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } while (khar_win32.msg_count == 0);

    khar_win32_msg_t msg = khar_win32.msg_buffer[khar_win32.msg_pos++];

    event->window = msg.hwnd;

    switch (msg.u_msg) {
        case WM_CLOSE:
            event->type = khar_event_type_Quit;
            break;

#ifdef KHAR_USE_RESIZE_EVENTS
        case WM_WINDOWPOSCHANGED: {
            RECT win_rect;
            GetClientRect(msg.hwnd, &win_rect);

            event->type = khar_event_type_Resize;
            event->resize.width  = win_rect.right  - win_rect.left;
            event->resize.height = win_rect.bottom - win_rect.top;
        } break;
#endif

#ifdef KHAR_USE_WHEEL_EVENTS
        case WM_MOUSEWHEEL: {
            event->type = khar_event_type_Wheel;
            event->wheel.scroll = ((signed short)(msg.w_param >> 16)) / 120;

            // Why the fuck is the wheel event screen relative when the button one,
            // which has the same interface, is client relative?
            POINT point = {
                .x = (signed short)(msg.l_param & 0xFFFF),
                .y = (signed short)(msg.l_param >> 16),
            };

            ScreenToClient(msg.hwnd, &point);

            event->wheel.x = point.x;
            event->wheel.y = point.y;
        } break;
#endif

#ifdef KHAR_USE_BUTTON_EVENTS
        case WM_LBUTTONUP:
            event->button.type = khar_button_Left;
            event->button.is_down = 0;
            goto button_end;
        case WM_MBUTTONUP:
            event->button.type = khar_button_Middle;
            event->button.is_down = 0;
            goto button_end;
        case WM_RBUTTONUP:
            event->button.type = khar_button_Right;
            event->button.is_down = 0;
            goto button_end;
        case WM_LBUTTONDOWN:
            event->button.type = khar_button_Left;
            event->button.is_down = 1;
            goto button_end;
        case WM_MBUTTONDOWN:
            event->button.type = khar_button_Middle;
            event->button.is_down = 1;
            goto button_end;
        case WM_RBUTTONDOWN:
            event->button.type = khar_button_Right;
            event->button.is_down = 1;
            goto button_end;
        button_end:
            event->type = khar_event_type_Button;
            event->button.x = (int)(msg.l_param & 0xFFFF);
            event->button.y = (int)(msg.l_param >> 16);
            break;
#endif

#ifdef KHAR_USE_MOTION_EVENTS
        case WM_INPUT: {
            HRAWINPUT raw_input_handle = (HRAWINPUT)msg.l_param;

            unsigned data_size; // = 48; // Only on x64. Would avoid the first GetRawInputData call.
            RAWINPUT raw_input;

            // NOTE: We need to query the size otherwise the api shits itself.
            //       The next call sometimes fails.
            GetRawInputData(raw_input_handle, RID_INPUT, NULL, &data_size, sizeof(RAWINPUTHEADER));

            if (GetRawInputData(raw_input_handle, RID_INPUT, &raw_input, &data_size, sizeof(RAWINPUTHEADER)) == (unsigned)-1) {
                // NOTE: After the above mentioned bug was fixed, this branch doesn't seem to be
                //       ever taken.
                goto poll_start;
            }

            if ((raw_input.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE) {
                khar_display_error("Relative mouse input not supported!\n");
            }

            event->type = khar_event_type_Motion;
            event->motion.x = (float)raw_input.data.mouse.lLastX;
            event->motion.y = (float)raw_input.data.mouse.lLastY;
        } break;
#endif

#ifdef KHAR_USE_KEY_EVENTS
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            unsigned code = (unsigned)msg.w_param;

            if (code == VK_SHIFT) {
                code = MapVirtualKey((msg.l_param & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
            }
            else if (code == VK_CONTROL) {
                code = (msg.l_param & 0x01000000) ? VK_RCONTROL : VK_LCONTROL;
            }
            else if (code == VK_MENU) {
                code = (msg.l_param & 0x01000000) ? VK_RMENU : VK_LMENU;
            }

            event->type = khar_event_type_Key;
            event->key.code = code;
            event->key.is_down = msg.u_msg == WM_KEYDOWN || msg.u_msg == WM_SYSKEYDOWN;
            event->key.repeat = msg.l_param & 0xFF;
        } break;
#endif
    }

    return 1;
}


khar_window_t khar_create_window(int width, int height, const char *title, int resizable, int pos_x, int pos_y)
{
    WNDCLASSEX win_class = {
        .cbSize = sizeof(WNDCLASSEX),
        .lpfnWndProc = win32_window_proc,
        .hInstance = khar_win32.instance,
        .lpszClassName = title,
    };

    if (!RegisterClassEx(&win_class)) {
        khar_display_error("Failed to register window!");
    }

    RECT window_rect = { 0, 0, width, height };
    AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);

    width  = window_rect.right  - window_rect.left;
    height = window_rect.bottom - window_rect.top;

    int screen_width  = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);

    if (pos_x < 0) {
        pos_x = (screen_width  - width) / 2;
    }

    if (pos_y < 0) {
        pos_y = (screen_height - height) / 2;
    }

    HWND window = CreateWindowExA(
        0,
        title, title,
        (WS_OVERLAPPEDWINDOW | WS_VISIBLE) ^ (resizable ? 0 : WS_THICKFRAME | WS_MAXIMIZEBOX),
        pos_x, pos_y, width, height,
        NULL, NULL,
        khar_win32.instance,
        NULL
    );

    if (!window) {
        khar_display_error("Failed to create window!");
    }

    khar_win32_register_leave_event(window);

#ifdef KHAR_USE_MOTION_EVENTS
    RAWINPUTDEVICE raw_input_device = {
        .usUsagePage = 0x01, // HID_USAGE_PAGE_GENERIC
        .usUsage = 0x02,     // HID_USAGE_GENERIC_MOUSE
        .dwFlags = 0,
        .hwndTarget = window,
    };

    if (!RegisterRawInputDevices(&raw_input_device, 1, sizeof(raw_input_device))) {
        khar_display_error("Failed to register raw input device!");
    }
#endif


    return window;
}


void khar_set_window_title(khar_window_t window, char *value)
{
    SetWindowTextA(window, value);
}

static void khar_win32_clip_cursor(void)
{
    RECT windowRect;
    GetClientRect(khar_win32.cursor_window, &windowRect);
    ClientToScreen(khar_win32.cursor_window, (POINT*)&windowRect.left);
    ClientToScreen(khar_win32.cursor_window, (POINT*)&windowRect.right);

    int width  = (windowRect.right  - windowRect.left) / 2;
    int height = (windowRect.bottom - windowRect.top)  / 2;

    windowRect.left   += width;
    windowRect.top    += height;
    windowRect.right  = windowRect.left;
    windowRect.bottom = windowRect.top;

    ClipCursor(&windowRect);
}


void khar_hide_cursor(khar_window_t window, int hidden)
{
    if (!khar_win32.is_cursor_hidden && hidden) {
        khar_win32.is_cursor_hidden = 1;
        khar_win32.cursor_window = window;

        khar_win32_clip_cursor();

        SetCursor(NULL);
    } else if (khar_win32.is_cursor_hidden && !hidden) {
        khar_win32.is_cursor_hidden = 0;

        ClipCursor(NULL);

        khar_win32_update_cursor_image();
    }
}


void khar_fullscreen(khar_window_t window, int full)
{
    DWORD style = GetWindowLong(window, GWL_STYLE);
    int is_overlapped = style & WS_OVERLAPPEDWINDOW;

    if (is_overlapped && full) {
        MONITORINFO monitor_info = { sizeof(monitor_info) };

        if (GetWindowPlacement(window, &khar_win32.pre_fullscreen_window_placement)
         && GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitor_info))
        {
            SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);

            SetWindowPos(
                window,
                HWND_TOP,
                monitor_info.rcMonitor.left,
                monitor_info.rcMonitor.top,
                monitor_info.rcMonitor.right  - monitor_info.rcMonitor.left,
                monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED
            );
        }
    } else if (!is_overlapped && !full) {
        SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);

        SetWindowPlacement(window, &khar_win32.pre_fullscreen_window_placement);

        SetWindowPos(
            window,
            NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED
        );
    }

    if (khar_win32.is_cursor_hidden && window == khar_win32.cursor_window) {
        khar_win32_clip_cursor();
    }
}


void khar_set_cursor_position(khar_window_t window, int x, int y)
{
    POINT point = { x, y };

    if (window) {
        ClientToScreen(window, &point);
    }

    SetCursorPos(point.x, point.y);
}


#ifdef KHAR_VULKAN

VkResult khar_create_surface(khar_window_t window, VkInstance instance, const VkAllocationCallbacks *p_allocator, VkSurfaceKHR *p_surface)
{
    VkWin32SurfaceCreateInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = khar_win32.instance,
        .hwnd = window,
    };

    return vkCreateWin32SurfaceKHR(instance, &info, p_allocator, p_surface);
}

#endif



#ifdef KHAR_SPAWN_CONSOLE
int main(void)
{
    khar_win32.instance = GetModuleHandle(NULL);
    
    DWORD out_mode = 0;
    khar_win32.stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    
    if (khar_win32.stdout_handle == INVALID_HANDLE_VALUE) {
        khar_display_error("Failed to retrieve stdout handle!");
    }
    
    if (!GetConsoleMode(khar_win32.stdout_handle, &out_mode)) {
        khar_display_error("Failed to retrieve console mode!");
    }
    
    khar_win32.out_mode_old = out_mode;
    out_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    
    if (!SetConsoleMode(khar_win32.stdout_handle, out_mode)) {
        khar_display_error("Failed to set console mode!");
    }
#else
int WinMain(
        HINSTANCE hinstance,
        HINSTANCE previous_instance,
        LPSTR command_line,
        int n_show_cmd
) {
    (void)previous_instance; (void)command_line; (void)n_show_cmd;

    khar_win32.instance = hinstance;
#endif

    setlocale(LC_CTYPE, ""); // NOTE: Might need to be LC_ALL


    int res = khar_main(__argc, __argv);

#ifdef KHAR_SPAWN_CONSOLE
    if (!SetConsoleMode(khar_win32.stdout_handle, khar_win32.out_mode_old)) {
        khar_display_error("Failed to restore console mode!");
    }
#endif

    return res;
}
