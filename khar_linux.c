#include "khar.h"

#include <stdio.h>
#include <stdlib.h>


khar_linux_t khar_linux = {0};


void khar_display_error(const char *message)
{
    fprintf(stderr,
        "khar error message:\n"
        "  %s\n"
        ,
        message
    );

    exit(1);
}


int khar_poll_events(khar_event_t *event)
{
    XEvent xevent;

    XEventsQueued(khar_linux.display, QueuedAfterFlush);

    while (QLength(khar_linux.display)) {
        XNextEvent(khar_linux.display, &xevent);

        switch (xevent.type) {
            case ClientMessage: {
                if ((unsigned int)(xevent.xclient.data.l[0]) == khar_linux.WM_DELETE_WINDOW) {
                    event->type = khar_event_type_Quit;
                    return 1;
                }
            } break;

#ifdef KHAR_USE_KEY_EVENTS
            case KeyPress:
            case KeyRelease: {
                KeySym key_sym = XkbKeycodeToKeysym(khar_linux.display, xevent.xkey.keycode, 0, 0);

                if (key_sym != NoSymbol) {
                    event->type = khar_event_type_Key;
                    event->key.code = key_sym;
                    event->key.is_down = xevent.type == KeyPress;
                    event->key.repeat = 1;
                    return 1;
                }
            } break;
#endif

#if defined(KHAR_USE_BUTTON_EVENTS) || defined(KHAR_USE_WHEEL_EVENTS)
        case ButtonPress:
        case ButtonRelease: {
            int button_code = xevent.xbutton.button;

            if (button_code == 4 || button_code == 5) {
#ifdef KHAR_USE_WHEEL_EVENTS
                if (xevent.type == ButtonRelease)
                    break;

                event->type = khar_event_type_Wheel;
                event->wheel.scroll = button_code == 4 ? 1 :-1;
                event->wheel.x = xevent.xbutton.x;
                event->wheel.y = xevent.xbutton.y;
                return 1;
#endif
            }
#ifdef KHAR_USE_BUTTON_EVENTS
            else {
                event->type = khar_event_type_Button;
                event->button.type = (khar_button_t)button_code; // NOTE: Works out to fit the enumeration.
                event->button.is_down = xevent.type == ButtonPress;
                event->button.x = xevent.xbutton.x;
                event->button.y = xevent.xbutton.y;
                return 1;
            }
#endif
        } break;
#endif

#ifdef KHAR_USE_MOTION_EVENTS
            case GenericEvent: {
                if (xevent.xcookie.extension == khar_linux.xi_op_code
                 && XGetEventData(khar_linux.display, &xevent.xcookie))
                {
                    if (xevent.xcookie.evtype == XI_RawMotion) {
                        XIRawEvent* raw_event = (XIRawEvent*)xevent.xcookie.data;

                        if (raw_event->valuators.mask_len) {
                            event->motion.x = 0.0f;
                            event->motion.y = 0.0f;

                            if (XIMaskIsSet(raw_event->valuators.mask, 0)) {
                                event->motion.x = (int)raw_event->raw_values[0];
                            }

                            if (XIMaskIsSet(raw_event->valuators.mask, 1)) {
                                event->motion.y = (int)raw_event->raw_values[1];
                            }

                            if (event->motion.x != 0.0f || event->motion.y != 0.0f) {
                                event->type = khar_event_type_Motion;
                                XFreeEventData(khar_linux.display, &xevent.xcookie);
                                return 1;
                            }
                        }
                    }

                    XFreeEventData(khar_linux.display, &xevent.xcookie);
                }
            } break;
#endif

#ifdef KHAR_USE_RESIZE_EVENTS
            case Expose: {
                if (xevent.xexpose.count == 0) {
                    event->type = khar_event_type_Resize;
                    event->resize.width  = xevent.xexpose.width;
                    event->resize.height = xevent.xexpose.height;
                    return 1;
                }
            } break;
#endif

#ifdef KHAR_USE_POINTER_EVENTS
            case MotionNotify: {
                event->type = khar_event_type_Pointer;
                event->pointer.x = xevent.xmotion.x;
                event->pointer.y = xevent.xmotion.y;
                return 1;
            } break;
#endif

        }
    }

    // TODO: Figure out.
    XFlush(khar_linux.display);

    return 0;
}


static void khar_linux_send_event_to_wm(Window window, Atom type,
                                        long a, long b, long c, long d, long e)
{
    XEvent event = { ClientMessage };
    event.xclient.window = window;
    event.xclient.format = 32;
    event.xclient.message_type = type;
    event.xclient.data.l[0] = a;
    event.xclient.data.l[1] = b;
    event.xclient.data.l[2] = c;
    event.xclient.data.l[3] = d;
    event.xclient.data.l[4] = e;

    XSendEvent(
        khar_linux.display,
        khar_linux.root,
        0,
        SubstructureNotifyMask | SubstructureRedirectMask,
        &event
    );
}


void khar_fullscreen(khar_window_t window, int full)
{
    khar_linux_send_event_to_wm(
        window,
        khar_linux._NET_WM_STATE,
        full,
        khar_linux._NET_WM_STATE_FULLSCREEN,
        0, 1, 0
    );
}


void khar_set_cursor_position(khar_window_t window, int x, int y)
{
    XWarpPointer(khar_linux.display, None, window, 0, 0, 0, 0, x, y);
}


void khar_set_window_title(khar_window_t window, char *value)
{
    XStoreName(khar_linux.display, window, value);
}


void khar_hide_cursor(khar_window_t window, int hidden)
{
    if (hidden) {
        int grab_result = XGrabPointer(
            khar_linux.display, 
            window,
            1,
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
            GrabModeAsync,
            GrabModeAsync,
            window,
            None,
            CurrentTime
        );

        if (grab_result != GrabSuccess) {
            // TODO: Write something out.
            return;
        }

        Cursor invisible_cursor;
        Pixmap bitmap_no_data;
        XColor black;
        static char no_data[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        black.red = black.green = black.blue = 0;

        bitmap_no_data = XCreateBitmapFromData(khar_linux.display, window, no_data, 8, 8);
        invisible_cursor = XCreatePixmapCursor(
            khar_linux.display,
            bitmap_no_data,
            bitmap_no_data,
            &black, &black, 0, 0
        );

        XDefineCursor(khar_linux.display, window, invisible_cursor);
        XFreeCursor(khar_linux.display, invisible_cursor);
        XFreePixmap(khar_linux.display, bitmap_no_data);
    }
    else {
        XUngrabPointer(khar_linux.display, CurrentTime);
        XUndefineCursor(khar_linux.display, window);
    }
}


#ifdef KHAR_VULKAN

VkResult khar_create_surface(khar_window_t window, VkInstance instance, const VkAllocationCallbacks *p_allocator, VkSurfaceKHR *p_surface)
{
    VkXlibSurfaceCreateInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .dpy = khar_linux.display,
        .window = window,
    };

    return vkCreateXlibSurfaceKHR(instance, &info, p_allocator, p_surface);
}

#endif


khar_window_t khar_create_window(int width, int height, const char *title, int resizable, int pos_x, int pos_y)
{
    long visual_mask = VisualScreenMask;

    int visual_count;
    XVisualInfo visual_template = {
        .screen = khar_linux.default_screen,
    };

    XVisualInfo *visual_info = XGetVisualInfo(khar_linux.display, visual_mask, &visual_template, &visual_count);

    Colormap colormap = XCreateColormap(khar_linux.display, khar_linux.root, visual_info->visual, AllocNone);

    long event_mask = 0;
    // TODO: Figure out the purpouse of this shit.
    event_mask |= KeyPressMask;
    event_mask |= KeyReleaseMask;
    event_mask |= StructureNotifyMask; // TODO: Figure out what this does.
    event_mask |= ExposureMask;

    XSetWindowAttributes window_attributes = {
        .colormap = colormap,
        .background_pixel = 0xFFFFFFFF,
        .border_pixel = 0,
        .event_mask = event_mask,
    };

    int screen_width  = DisplayWidth(khar_linux.display,  khar_linux.default_screen);
    int screen_height = DisplayHeight(khar_linux.display, khar_linux.default_screen);

    if (pos_x < 0) {
        pos_x = (screen_width  - width) / 2;
    }

    if (pos_y < 0) {
        pos_y = (screen_height - height) / 2;
    }

    Window window = XCreateWindow(
        khar_linux.display,
        khar_linux.root,
        pos_x, pos_y, width, height,
        0, visual_info->depth,
        InputOutput,
        visual_info->visual,
        CWBackPixel | CWBorderPixel | CWEventMask | CWColormap,
        &window_attributes
    );

    XFree(visual_info);

    long select_event_mask = 0;
#ifdef KHAR_USE_KEY_EVENTS
    select_event_mask |= KeyPressMask;
    select_event_mask |= KeyReleaseMask;
#endif
#if defined(KHAR_USE_BUTTON_EVENTS) || defined(KHAR_USE_WHEEL_EVENTS)
    select_event_mask |= ButtonPressMask;
    select_event_mask |= ButtonReleaseMask;
#endif
#ifdef KHAR_USE_RESIZE_EVENTS
    select_event_mask |= ExposureMask;
#endif
#ifdef KHAR_USE_POINTER_EVENTS
    select_event_mask |= PointerMotionMask;
#endif
#if 0
    select_event_mask |= KeymapStateMask;
#endif

    XSelectInput(khar_linux.display, window, select_event_mask);

    XStoreName(khar_linux.display, window, title);

    XMapWindow(khar_linux.display, window);

    XSetWMProtocols(khar_linux.display, window, &khar_linux.WM_DELETE_WINDOW, 1);

    if (!resizable) {
        XSizeHints hints = {
            .flags = PMinSize| PMaxSize,
            .min_width = width, .min_height = height,
            .max_width = width, .max_height = height,
        };

        XSetWMNormalHints(khar_linux.display, window, &hints);
    }

    XFlush(khar_linux.display);

    return window;
}


int main(int argc, char *argv[])
{
#if 0
    if (!XInitThreads()) {
        khar_display_error("Failed to initialize thread support in XLib!");
    }
#endif
    khar_linux.display = XOpenDisplay(NULL);
    khar_linux.default_screen = DefaultScreen(khar_linux.display);
    khar_linux.root = RootWindow(khar_linux.display, khar_linux.default_screen);
    khar_linux.WM_DELETE_WINDOW = XInternAtom(khar_linux.display, "WM_DELETE_WINDOW", 0);
    khar_linux._NET_WM_STATE = XInternAtom(khar_linux.display, "_NET_WM_STATE", 0);
    khar_linux._NET_WM_STATE_FULLSCREEN = XInternAtom(khar_linux.display, "_NET_WM_STATE_FULLSCREEN", 0);

#ifdef KHAR_USE_MOTION_EVENTS
    int xi_event, xi_error;
    if (!XQueryExtension(khar_linux.display, "XInputExtension", &khar_linux.xi_op_code, &xi_event, &xi_error)) {
        fprintf(stderr, "khar error: X Input extension not available.\n");
        exit(1);
    }

    int xi_major = 2, xi_minor = 0;
    if (XIQueryVersion(khar_linux.display, &xi_major, &xi_minor) == BadRequest) {
        fprintf(stderr, "khar error: XI2 not available. Server supports %d.%d\n", xi_major, xi_minor);
        exit(1);
    }

    unsigned char mask[XIMaskLen(XI_RawMotion)] = { 0 };
    XIEventMask event_mask = {
        .deviceid = XIAllMasterDevices,
        .mask_len = sizeof(mask),
        .mask = mask
    };
    XISetMask(mask, XI_RawMotion);

    XISelectEvents(khar_linux.display, khar_linux.root, &event_mask, 1);
#endif

    int res = khar_main(argc, argv);

    // FIXME: Segfaults because instance get's destroyed before this.
    // XCloseDisplay(khar_linux.display);

    return res;
}
