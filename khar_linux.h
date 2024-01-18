#ifndef KHAR_LINUX_
#define KHAR_LINUX_

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/Xutil.h>


typedef struct
{
    Display *display;
    int default_screen;
    Window root;
    Atom WM_DELETE_WINDOW;
    Atom _NET_WM_STATE;
    Atom _NET_WM_STATE_FULLSCREEN;

    int xi_op_code;
} khar_linux_t;

extern khar_linux_t khar_linux;

typedef Window khar_window_t;


/* keys */

#define KEY_BACK          XK_BackSpace
#define KEY_TAB           XK_Tab
#define KEY_RETURN        XK_Return
#define KEY_CAPS_LOCK     XK_Caps_Lock
#define KEY_ESCAPE        XK_Escape
#define KEY_HOME          XK_Home
#define KEY_END           XK_End
#define KEY_INSERT        XK_Insert
#define KEY_DELETE        XK_Delete
#define KEY_UP            XK_Up
#define KEY_DOWN          XK_Down
#define KEY_LEFT          XK_Left
#define KEY_RIGHT         XK_Right
#define KEY_PAGE_UP       XK_Page_Up
#define KEY_PAGE_DOWN     XK_Page_Down
#define KEY_SUPER_LEFT    XK_Super_L
#define KEY_SUPER_RIGHT   XK_Super_R

#define KEY_SHIFT_LEFT    XK_Shift_L
#define KEY_SHIFT_RIGHT   XK_Shift_R
#define KEY_CONTROL_LEFT  XK_Control_L
#define KEY_CONTROL_RIGHT XK_Control_R
#define KEY_ALT_LEFT      XK_Alt_L
#define KEY_ALT_RIGHT     XK_Alt_R

#define KEY_F1            XK_F1
#define KEY_F2            XK_F2
#define KEY_F3            XK_F3
#define KEY_F4            XK_F4
#define KEY_F5            XK_F5
#define KEY_F6            XK_F6
#define KEY_F7            XK_F7
#define KEY_F8            XK_F8
#define KEY_F9            XK_F9
#define KEY_F10           XK_F10
#define KEY_F11           XK_F11
#define KEY_F12           XK_F12

#define KEY_GRAVE         XK_grave
#define KEY_MINUS         XK_minus
#define KEY_EQUALS        XK_equal
#define KEY_BRACKET_LEFT  XK_bracketleft
#define KEY_BRACKET_RIGHT XK_bracketright
#define KEY_BACKSLASH     XK_backslash
#define KEY_SEMICOLON     XK_semicolon
#define KEY_APOSTROPHE    XK_apostrophe
#define KEY_COMMA         XK_comma
#define KEY_PERIOD        XK_period
#define KEY_SLASH         XK_slash
#define KEY_SPACE         XK_space

#define KEY_0             XK_0
#define KEY_1             XK_1
#define KEY_2             XK_2
#define KEY_3             XK_3
#define KEY_4             XK_4
#define KEY_5             XK_5
#define KEY_6             XK_6
#define KEY_7             XK_7
#define KEY_8             XK_8
#define KEY_9             XK_9

#define KEY_A             XK_a
#define KEY_B             XK_b
#define KEY_C             XK_c
#define KEY_D             XK_d
#define KEY_E             XK_e
#define KEY_F             XK_f
#define KEY_G             XK_g
#define KEY_H             XK_h
#define KEY_I             XK_i
#define KEY_J             XK_j
#define KEY_K             XK_k
#define KEY_L             XK_l
#define KEY_M             XK_m
#define KEY_N             XK_n
#define KEY_O             XK_o
#define KEY_P             XK_p
#define KEY_Q             XK_q
#define KEY_R             XK_r
#define KEY_S             XK_s
#define KEY_T             XK_t
#define KEY_U             XK_u
#define KEY_V             XK_v
#define KEY_W             XK_w
#define KEY_X             XK_x
#define KEY_Y             XK_y
#define KEY_Z             XK_z


#endif // KHAR_LINUX_

