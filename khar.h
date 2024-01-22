#ifndef KHAR_H_
#define KHAR_H_

/* Allows for defining which events your program cares about.
 * If `KHAR_LEAN_AND_MEAN` is defined, the events you want must
 * be declared individually.
 * This can be done globally, by compiler flag defines, or this
 * header can be modified appropriately.
 */
#ifndef KHAR_LEAN_AND_MEAN
    #define KHAR_USE_KEY_EVENTS
    #define KHAR_USE_MOTION_EVENTS
    #define KHAR_USE_BUTTON_EVENTS
    #define KHAR_USE_WHEEL_EVENTS
    #define KHAR_USE_RESIZE_EVENTS
    #define KHAR_USE_POINTER_EVENTS
#endif

/* If defined, on Windows a console will appear when you run the program
 * with the stdout output.
 * Should be only enabled in debug mode.
 */
// #define KHAR_SPAWN_CONSOLE

/* user defined */

/* Instead of the normal main function, you need to define this.
 */
int khar_main(int argc, char *argv[]);


/* library */

#ifdef _WIN32
    #include "khar_win32.h"
#else
    #include "khar_linux.h"
#endif


typedef enum
{
    khar_event_type_Quit,    /* Program wants to exit. */
    khar_event_type_Key,     /* Key on keyboard was pressed or released. */
    khar_event_type_Motion,  /* Relative movement of mouse not affected by window or screen. */
    khar_event_type_Button,  /* Mouse button was pressed or released. */
    khar_event_type_Wheel,   /* Mouse wheel was rolled. */
    khar_event_type_Resize,  /* Window was resized. */
    khar_event_type_Pointer, /* Absolute position of mouse pointer changed. */

    KHAR_EVENT_TYPE_COUNT
} khar_event_type_t;


/* Key codes are defined inside the platform headers.
 */
typedef struct
{
    unsigned code;
    int is_down;
    int repeat;
} khar_event_key_t;


typedef struct
{
    float x, y;
} khar_event_motion_t;


typedef enum
{
    khar_button_Left = 0,
    khar_button_Middle,
    khar_button_Right,
} khar_button_t;

typedef struct
{
    int x, y;
    int is_down;
    khar_button_t type;
} khar_event_button_t;


typedef struct
{
    int x, y;
    int scroll;
} khar_event_wheel_t;


typedef struct
{
    int width, height;
} khar_event_resize_t;

typedef struct
{
    int x, y;
} khar_event_pointer_t;

typedef struct
{
    khar_event_type_t type;
    khar_window_t window;

    union {
        khar_event_key_t     key;
        khar_event_motion_t  motion;
        khar_event_button_t  button;
        khar_event_wheel_t   wheel;
        khar_event_resize_t  resize;
        khar_event_pointer_t pointer;
    };
} khar_event_t;


/* Returns 1 if an event is available and writes it at the provided pointer.
 * Returns 0 if no event is available and contents of event are undefined. 
 */
int khar_poll_events(khar_event_t *event);


/* Creates a window with provided parameters.
 * Return value is a valid window, on failure displays an error message and kills the program.
 */
khar_window_t khar_create_window(int width, int height, const char *title, int resizable, int pos_x, int pos_y);


/* Displays an error message to the user and kills the program.
 * Specifics of how the message will be displayed are platform dependent.
 */
void khar_display_error(const char *message);


/* Sets the title of the provided window to the value string.
 */
void khar_set_window_title(khar_window_t window, char *value);


/* Can hide and reveal the cursor.
 * If `hidden` is 1 then it makes the cursor invisible and restricted to the `window` area.
 * In this state pointer events will not be submitted.
 * If `hidden` is 0 then the cursor behaves normally and `window` is ignored.
 */
void khar_hide_cursor(khar_window_t window, int hidden);


/* Can make the window go fullscreen or back.
 * If `full` is 1 then it makes the `window` fullscreen.
 * If `full` is 0 then the window will become a normal window with a border.
 * Having more then 1 window fullscreen is undefined.
 */
void khar_fullscreen(khar_window_t window, int full);


/* Moves the cursor to specified position.
 * If `window` is 0, the position is screen relative.
 * If `window` is specified, the position is client relative.
 */
void khar_set_cursor_position(khar_window_t window, int x, int y);


/* Needs to be defined globally.
 */
#ifdef KHAR_VULKAN

// Not a nice solution but simple to change. Can also be <vulkan.h>.
#include "volk/volk.h"

/* Creates a vulkan surface for provided window as per VK_KHR_surface extension specification.
 * https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_surface.html
 */

#if _WIN32
    #define KHAR_VK_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#else
    #define KHAR_VK_SURFACE_EXTENSION_NAME VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif

VkResult khar_create_surface(khar_window_t window, VkInstance instance, const VkAllocationCallbacks *p_allocator, VkSurfaceKHR *p_surface);

#endif


#endif // KHAR_H_
