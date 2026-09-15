#pragma once

#include "custom-types.h"

void OpenX11Window();

void CloseX11Window();

void DestroyXImage();

void CreateXImage(FrameBuffer *buf, int width, int height);

void ClearBuffer(FrameBuffer *buf);

void AddToBuffer(FrameBuffer *buf, int x, int y, uint32_t data);

void PresentBuffer(FrameBuffer *buf);

int X11Input(Camera *cam, Event *ev, int *width, int *height);

#ifdef X11_IMPL

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrender.h>


#define RENDER_IMPL
#include "render.h"

static Display *display;
static int screen;
static Window window;
static GC gc;
static XImage *img;
static XEvent event;

static float sens = 0.008;

static int pointerCaptured = 0;

void OpenX11Window() {
        display = XOpenDisplay(NULL);

        screen = DefaultScreen(display);

        window = XCreateSimpleWindow(
                display, RootWindow(display, screen), 0, 0,
                640, 480, 0, BlackPixel(display, screen),
                WhitePixel(display, screen)
        );

        XMapWindow(display, window);
        XFlush(display);

        gc = XCreateGC(display, window, 0, NULL);

        XSelectInput(display, window, KeyPressMask | KeyReleaseMask | StructureNotifyMask | PointerMotionMask);

        XWindowChanges changes;
        changes.width = 640;
        changes.height = 480;
        changes.stack_mode = Above;

        XConfigureWindow(display, window, CWWidth | CWHeight | CWStackMode,&changes);

        XGrabPointer(display, window, 1, PointerMotionMask, GrabModeAsync,
                     GrabModeAsync, None, None, CurrentTime);
        XFixesHideCursor(display, window);
        pointerCaptured = 1;
}

void CloseX11Window() {
        XDestroyImage(img);
        XFreeGC(display, gc);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
}

void DestroyXImage() {
        if (img != NULL) {
                XDestroyImage(img);
                img = NULL;
        }
}

void CreateXImage(FrameBuffer *buf, int width, int height) {
        img = XCreateImage(
                display, DefaultVisual(display, screen),
                DefaultDepth(display, screen), ZPixmap, 0,
                (char *)buf->data, width, height, 32, 0
        );
}

void PresentBuffer(FrameBuffer *buf) {
        int width = buf->width;
        int height = buf->height;

        int newWidth = width * 2;
        int newHeight = height * 2;

        FrameBuffer scaledBuffer = {
                (uint32_t*)malloc(newWidth * newHeight * sizeof(uint32_t)),
                newWidth,
                newHeight
        };

        for (int x = 0; x < width; x++) {
                for (int y = 0; y < height; y++) {
                        scaledBuffer.data[(x * 2) + ((y * 2) * newWidth)] = buf->data[x + (y * width)];
                        scaledBuffer.data[(x * 2 + 1) + ((y * 2) * newWidth)] = buf->data[x + (y * width)];

                        scaledBuffer.data[(x * 2) + ((y * 2 + 1) * newWidth)] = buf->data[x + (y * width)];
                        scaledBuffer.data[(x * 2 + 1) + ((y * 2 + 1) * newWidth)] = buf->data[x + (y * width)];
                }
        }

        XImage *scaledImg = XCreateImage(
                display, DefaultVisual(display, screen),
                DefaultDepth(display, screen), ZPixmap, 0,
                (char*)(scaledBuffer.data), scaledBuffer.width, scaledBuffer.height, 32, 0
        );

        XPutImage(display, window, gc, scaledImg, 0, 0, 0, 0, newWidth, newHeight);
        XFlush(display);

        XDestroyImage(scaledImg);
}

void AddToBuffer(FrameBuffer *buf, int x, int y, uint32_t data) {
        buf->data[x + (y * buf->width)] = data;
}

void ClearBuffer(FrameBuffer *buf) {
        memset(
                buf->data, 0x00000000,
                (buf->width * buf->height * sizeof(uint32_t))
        );
}

int X11Input(Camera *cam, Event *ev, int *width, int *height) {
        int resized = 0;
        int keyPressed;

        while (XPending(display)) {
                XNextEvent(display, &event);
                if (event.type == ConfigureNotify) {
                        *width = event.xconfigure.width;
                        *height = event.xconfigure.height;

                        resized = 1;
                } else if (pointerCaptured == 1 && event.type == MotionNotify) {
                        int dx = event.xmotion.x - (*width / 2);
                        int dy = event.xmotion.y - (*height / 2);

                        if (dx != 0 || dy != 0) {
                                cam->yaw -= dx * sens;
                                cam->pitch -= dy * sens;
                        }
                } else if (event.type == KeyPress || event.type == KeyRelease) {
                        keyPressed = 1;

                        if (event.type == KeyRelease) keyPressed = 0;

                        KeySym key = XLookupKeysym(&event.xkey, 0);

                        switch (key) {
                        case XK_q:
                                ev->quit = 1;
                                break;
                        case XK_w:
                                ev->keys.w = keyPressed;
                                break;
                        case XK_a:
                                ev->keys.a = keyPressed;
                                break;
                        case XK_s:
                                ev->keys.s = keyPressed;
                                break;
                        case XK_d:
                                ev->keys.d = keyPressed;
                                break;
                        case XK_space:
                                ev->keys.space = keyPressed;
                                break;
                        case XK_Shift_L:
                                ev->keys.shift = keyPressed;
                                break;
                        case XK_v:
                                if (keyPressed == 0) break;
                                
                                if (ev->wireframeMode == 0) {
                                        ev->wireframeMode = 1;
                                } else {
                                        ev->wireframeMode = 0;
                                }

                                break;
                        case XK_Escape:
                                if (keyPressed == 0) break;
                                
                                if (pointerCaptured == 0) {
                                        XGrabPointer(
                                                display, window, 1,
                                                PointerMotionMask,
                                                GrabModeAsync,
                                                GrabModeAsync, None, None,
                                                CurrentTime
                                        );
                                        
                                        XFixesHideCursor(display, window);
                                        pointerCaptured = 1;
                                } else {
                                        XUngrabPointer(display, CurrentTime);
                                        XFixesShowCursor(display, window);
                                        pointerCaptured = 0;
                                }
                        }
                }
        }

        if (pointerCaptured == 1)
                XWarpPointer(display, None, window, 0, 0, 0, 0, *width / 2, *height / 2);
        if (resized == 1)
                UpdatePerspectiveMatrix(*width, *height);

        return resized;
}
#endif