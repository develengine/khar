#ifndef KHAR_WIN32_
#define KHAR_WIN32_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// HELL
#undef near
#undef far

// Probably overkill.
#define KHAR_WIN32_MAX_BUFFERED_EVENTS 2048


typedef struct
{
    HWND hwnd;
    UINT u_msg;
    WPARAM w_param;
    LPARAM l_param;
} khar_win32_msg_t;


typedef struct
{
    HINSTANCE instance;

    HANDLE stdout_handle;
    DWORD out_mode_old;

    khar_win32_msg_t msg_buffer[KHAR_WIN32_MAX_BUFFERED_EVENTS];
    int msg_count, msg_pos;

    int is_cursor_hidden;
    HWND cursor_window;

    // TODO: Should be per window.
    WINDOWPLACEMENT pre_fullscreen_window_placement;
    int ignoring_events;
    int cursor_in_client;
} khar_win32_t;


extern khar_win32_t khar_win32;


typedef HWND khar_window_t;


/* keys */

#define KEY_BACK          VK_BACK
#define KEY_TAB           VK_TAB
#define KEY_RETURN        VK_RETURN
#define KEY_CAPS_LOCK     VK_CAPITAL
#define KEY_ESCAPE        VK_ESCAPE
#define KEY_HOME          VK_HOME
#define KEY_END           VK_END
#define KEY_INSERT        VK_INSERT
#define KEY_DELETE        VK_DELETE
#define KEY_UP            VK_UP
#define KEY_DOWN          VK_DOWN
#define KEY_LEFT          VK_LEFT
#define KEY_RIGHT         VK_RIGHT
#define KEY_PAGE_UP       VK_PRIOR
#define KEY_PAGE_DOWN     VK_NEXT
#define KEY_SUPER_LEFT    VK_LWIN
#define KEY_SUPER_RIGHT   VK_RWIN

#define KEY_SHIFT_LEFT    VK_LSHIFT
#define KEY_SHIFT_RIGHT   VK_RSHIFT
#define KEY_CONTROL_LEFT  VK_LCONTROL
#define KEY_CONTROL_RIGHT VK_RCONTROL
#define KEY_ALT_LEFT      VK_LMENU
#define KEY_ALT_RIGHT     VK_RMENU

#define KEY_F1            VK_F1
#define KEY_F2            VK_F2
#define KEY_F3            VK_F3
#define KEY_F4            VK_F4
#define KEY_F5            VK_F5
#define KEY_F6            VK_F6
#define KEY_F7            VK_F7
#define KEY_F8            VK_F8
#define KEY_F9            VK_F9
#define KEY_F10           VK_F10
#define KEY_F11           VK_F11
#define KEY_F12           VK_F12

#define KEY_GRAVE         VK_OEM_3
#define KEY_MINUS         VK_OEM_MINUS
#define KEY_EQUALS        VK_OEM_PLUS
#define KEY_BRACKET_LEFT  VK_OEM_4
#define KEY_BRACKET_RIGHT VK_OEM_6
#define KEY_BACKSLASH     VK_OEM_5
#define KEY_SEMICOLON     VK_OEM_1
#define KEY_APOSTROPHE    VK_OEM_7
#define KEY_COMMA         VK_OEM_COMMA
#define KEY_PERIOD        VK_OEM_PERIOD
#define KEY_SLASH         VK_OEM_2
#define KEY_SPACE         VK_SPACE

#define KEY_0             '0'
#define KEY_1             '1'
#define KEY_2             '2'
#define KEY_3             '3'
#define KEY_4             '4'
#define KEY_5             '5'
#define KEY_6             '6'
#define KEY_7             '7'
#define KEY_8             '8'
#define KEY_9             '9'

#define KEY_A             'A'
#define KEY_B             'B'
#define KEY_C             'C'
#define KEY_D             'D'
#define KEY_E             'E'
#define KEY_F             'F'
#define KEY_G             'G'
#define KEY_H             'H'
#define KEY_I             'I'
#define KEY_J             'J'
#define KEY_K             'K'
#define KEY_L             'L'
#define KEY_M             'M'
#define KEY_N             'N'
#define KEY_O             'O'
#define KEY_P             'P'
#define KEY_Q             'Q'
#define KEY_R             'R'
#define KEY_S             'S'
#define KEY_T             'T'
#define KEY_U             'U'
#define KEY_V             'V'
#define KEY_W             'W'
#define KEY_X             'X'
#define KEY_Y             'Y'
#define KEY_Z             'Z'

#endif // KHAR_WIN32_
