#include <stdint.h>
#include <math.h>
#include "common.h"
#include "render.cpp"

#include <windows.h>
#include <windowsx.h> // GET_X_LPARAM, GET_Y_LPARAM
#include <stdio.h>

//static bool g_refresh = false; // XXX

namespace Platform {

#ifdef RENDERDEV_DEBUG
//XXX Warning: this is always behind by one message, at least in remedybg. Now I'm mainly just putting relevant info in the window title (SetWindowText).
void DEBUG_printf(const char* format, ...) {
    local_persist char buf[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    OutputDebugStringA(buf);
    OutputDebugStringA(""); // does this fix it? (XXX no)
}
global char DEBUG_window_text[256];
void DEBUG_display(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(DEBUG_window_text, sizeof(DEBUG_window_text), format, args);
    va_end(args);
}
#endif

}

namespace Win32 {

bool g_running;

//void Win32::process_XInput_digital_button(DWORD xinputButtonState, DWORD buttonBit, Render::Button_State* oldState, Render::Button_State* newState) {
//    newState->endedDown = (xinputButtonState & buttonBit) != 0;
//    newState->halfTransistionCount = (oldState->endedDown != newState->endedDown) ? 1 : 0;
//}

struct Offscreen_Bitmap_Buffer {
    BITMAPINFO info;
    void* memory;
    int width;
    int height;
    int pitch; // number of bytes composing a row (row-to-row distance)
};

struct Window_Dimension {
    int width;
    int height;
};

global Offscreen_Bitmap_Buffer g_bitmap;

// R (1), G (1), B (1), pad to align to 32 (1)
global const int BYTES_PER_PIXEL = 4;

Window_Dimension get_window_dimension(HWND window) {
    Window_Dimension result;
    RECT client_rect;
    GetClientRect(window, &client_rect);
    result.width = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;
    return result;
}

void resize_DIB_section(Offscreen_Bitmap_Buffer* bitmap, int width, int height) {

    if (bitmap->memory) {
        VirtualFree(bitmap->memory, 0, MEM_RELEASE); // release = decommit + free
    }

    bitmap->width = width;
    bitmap->height = height;
    bitmap->pitch = width * BYTES_PER_PIXEL;

    bitmap->info.bmiHeader.biSize = sizeof(bitmap->info.bmiHeader);
    bitmap->info.bmiHeader.biWidth = width;
    bitmap->info.bmiHeader.biHeight = -height; // negative = top-down instead of bottom-up
    bitmap->info.bmiHeader.biPlanes = 1;
    bitmap->info.bmiHeader.biBitCount = 32;
    bitmap->info.bmiHeader.biCompression = BI_RGB;

    int bitmap_memory_size = width * height * BYTES_PER_PIXEL;
    bitmap->memory = VirtualAlloc(0, bitmap_memory_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    if (!bitmap->memory) {
        // TODO: error
    }

}

void display_bitmap(HDC device_context, int window_width, int window_height, Offscreen_Bitmap_Buffer* bitmap) {

    // XXX I don't want the image to stretch at all right now.  Eventually maybe want to stretch it but preserve aspect ratio.
    assert(window_width == bitmap->width);
    assert(window_height == bitmap->height);

    StretchDIBits(
        device_context,
        //x, y, width, height,
        //x, y, width, height,
        // to avoid issues from using "dirty rectangle" (?)
        0, 0, window_width, window_height,
        0, 0, bitmap->width, bitmap->height,
        bitmap->memory,
        &bitmap->info,
        DIB_RGB_COLORS,
        SRCCOPY);
}

void process_keyboard_message(bool isDown, bool wasDown, Render::Button_State* state) {
    //assert(state->endedDown != isDown); // This happens if you press W and up arrow together, but otherwise, one interrupts the other, and then you're just holding one and nothing is happening.  So need better logic here.
    if (isDown != state->endedDown) {
        state->endedDown = isDown;
        //++state->halfTransistionCount;
    }
    if (isDown) {
        state->downCount += 1;
    } else if (wasDown) {
        state->upCount += 1;
    }
}

void process_mouse_button_message(bool isDown, Render::Button_State* state) {
    if (isDown != state->endedDown) {
        state->endedDown = isDown;
        //++state->halfTransistionCount;
    }
}

#define ProcessKBMessage(KEYBOARD_BUTTON_INDEX) process_keyboard_message(isDown, wasDown, &input->keyboard.buttons[KEYBOARD_BUTTON_INDEX])

bool process_pending_messages(Render::Input* input) {
    bool running = true;
    MSG message;
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
        switch (message.message) {

            case WM_QUIT: {
                running = false;
                break;
            }

            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: {
                u32 key = (u32) message.wParam;
                bool wasDown = (message.lParam & (1 << 30)) != 0;
                bool isDown = (message.lParam & (1 << 31)) == 0;
                //if (wasDown != isDown) {
                    if      (key == 'W') { ProcessKBMessage(Render::KEYBOARD_BUTTON_UP); }
                    else if (key == 'S') { ProcessKBMessage(Render::KEYBOARD_BUTTON_DOWN); }
                    else if (key == 'A') { ProcessKBMessage(Render::KEYBOARD_BUTTON_LEFT); }
                    else if (key == 'D') { ProcessKBMessage(Render::KEYBOARD_BUTTON_RIGHT); }
                    else if (key == 'G') { ProcessKBMessage(Render::KEYBOARD_BUTTON_G); }
                    else if (key == 'R') { ProcessKBMessage(Render::KEYBOARD_BUTTON_R); }
                    else if (key == 'Q') { ProcessKBMessage(Render::KEYBOARD_BUTTON_Q); }
                    else if (key == 'E') { ProcessKBMessage(Render::KEYBOARD_BUTTON_E); }
                    else if (key == 'T') { ProcessKBMessage(Render::KEYBOARD_BUTTON_T); }
                    else if (key == '1') { ProcessKBMessage(Render::KEYBOARD_BUTTON_1); }
                    else if (key == '2') { ProcessKBMessage(Render::KEYBOARD_BUTTON_2); }
                    else if (key == '3') { ProcessKBMessage(Render::KEYBOARD_BUTTON_3); }
                    else if (key == '4') { ProcessKBMessage(Render::KEYBOARD_BUTTON_4); }
                    else if (key == VK_UP) { ProcessKBMessage(Render::KEYBOARD_BUTTON_UP); }
                    else if (key == VK_DOWN) { ProcessKBMessage(Render::KEYBOARD_BUTTON_DOWN); }
                    else if (key == VK_LEFT) { ProcessKBMessage(Render::KEYBOARD_BUTTON_LEFT); }
                    else if (key == VK_RIGHT) { ProcessKBMessage(Render::KEYBOARD_BUTTON_RIGHT); }
                    else if (key == VK_ESCAPE) {
                        running = false;
                    } else if (key == VK_SPACE) {
                        // To hopefully mitigate the delayed printing.
                        OutputDebugStringA("");
                    }
                //}

                // Since we're handling SYSKEYUP/SYSKEYDOWN, need to do this to restore ALT+F4 functionality.
                bool altKeyWasDown = ((message.lParam & (1 << 29)) != 0);
                if (key == VK_F4 && altKeyWasDown) {
                    running = false;
                }
                break;
            }

            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP: {
                // process_mouse_message
                int x = GET_X_LPARAM(message.lParam);
                int y = GET_Y_LPARAM(message.lParam);
                input->mouse.cursor.position.x = x;
                input->mouse.cursor.position.y = y;
                bool isDown = message.wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON);
                switch (message.message) {
                    case WM_LBUTTONDOWN:
                    case WM_LBUTTONUP: {
                        process_mouse_button_message(isDown, &input->mouse.buttons[Render::MOUSE_BUTTON_LEFT]);
                        break;
                    }
                    case WM_MBUTTONDOWN:
                    case WM_MBUTTONUP: {
                        process_mouse_button_message(isDown, &input->mouse.buttons[Render::MOUSE_BUTTON_MIDDLE]);
                        break;
                    }
                    case WM_RBUTTONDOWN:
                    case WM_RBUTTONUP: {
                        process_mouse_button_message(isDown, &input->mouse.buttons[Render::MOUSE_BUTTON_RIGHT]);
                        break;
                    }
                }
                break;
            }

            case WM_MOUSEWHEEL: {
                int delta = GET_WHEEL_DELTA_WPARAM(message.wParam) / WHEEL_DELTA;
                int modifierKey = GET_KEYSTATE_WPARAM(message.wParam);
                int x = GET_X_LPARAM(message.lParam);
                int y = GET_Y_LPARAM(message.lParam);
                //Platform::DEBUG_printf("delta = %d, x = %d, y = %d, modifier key = %d\n", delta, x, y, modifierKey);
                input->mouse.wheel.delta += delta;
                input->mouse.wheel.position.x = x;
                input->mouse.wheel.position.y = y;
                break;
            }

            case WM_SIZE: {
                assert(!"got here somehow");
                break;
            }

            default: {
                TranslateMessage(&message);
                DispatchMessage(&message);
                break;
            }
        }
    }
    return running;
}

LRESULT CALLBACK main_window_callback(
    HWND window,
    UINT message,
    WPARAM wparam,
    LPARAM lparam
) {
    LRESULT result = 0;

    switch (message) {

        case WM_CLOSE:
        case WM_DESTROY: {
            // I wanted to move this to Win32::process_pending_messages, but it seems like it needs to go through DispatchMessage to be handled correctly.
            g_running = false;
            break;
        }

        case WM_QUIT:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            assert(!"This should have been handled by Win32::process_pending_messages.");
        }

        case WM_SIZE: {
            WORD width = LOWORD(lparam);
            WORD height = HIWORD(lparam);
            if (g_bitmap.width != width || g_bitmap.height != height) {
                resize_DIB_section(&g_bitmap, width, height);
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window, &paint);
            Window_Dimension dimension = get_window_dimension(window);
            display_bitmap(device_context, dimension.width, dimension.height, &g_bitmap);
            EndPaint(window, &paint);
            break;
        }

        // case WM_ACTIVATEAPP

        default:
            result = DefWindowProc(window, message, wparam, lparam);
            break;
    }

    return result;
}

global s64 g_perfCountFrequency;

s64 get_wall_clock() {
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result.QuadPart;
}

f32 get_seconds_elapsed(s64 start, s64 end) {
    f32 secondsElapsed = ((f32)(end - start)) / (f32)g_perfCountFrequency;
    return secondsElapsed;
}

} // namespace Win32

int CALLBACK WinMain(
    HINSTANCE instance,
    HINSTANCE /*prev_instance*/,
    char* /*command_line*/,
    int /*show_code*/
) {

    UINT desiredSchedulerMs = 1;
    bool sleepIsGranular = (timeBeginPeriod(desiredSchedulerMs) == TIMERR_NOERROR);

    WNDCLASS window_class = {};

    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = &Win32::main_window_callback;
    window_class.hInstance = instance;
    window_class.lpszClassName = "RenderTestWindowClass";
    //window_class.hIcon = ;

    int monitorRefreshHz = 60; // TODO
    int updateHz = monitorRefreshHz / 2;
    f32 targetSecondsElapsedPerFrame = 1.0f / (f32) updateHz;

    if (RegisterClass(&window_class)) {

        HWND window = CreateWindowEx(
            0,                                // DWORD      exStyle
            window_class.lpszClassName,       // LPCSTR     className
            "RenderTest",                     // LPCSTR     windowName
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, // DWORD      style
            CW_USEDEFAULT,                    // int        X
            CW_USEDEFAULT,                    // int        Y
            800,//CW_USEDEFAULT,              // int        width
            800,//CW_USEDEFAULT,              // int        height
            0,                                // HWND       wndParent
            0,                                // HMENU      menu
            instance,                         // HINSTANCE  instance
            0);                               // LPVOID     param

        if (window) {

            Win32::Window_Dimension dimension = Win32::get_window_dimension(window);
            Win32::resize_DIB_section(&Win32::g_bitmap, dimension.width, dimension.height);

            HDC device_context = GetDC(window); // OWNDC lets us not need to reacquire then free this every loop

            LPVOID baseAddress = 0;

            Render::Memory memory = {};
            memory.permanentStorageSize = megabytes(64);
            memory.transientStorageSize = megabytes(128);
            u64 totalSize = memory.permanentStorageSize + memory.transientStorageSize;
            memory.permanentStorage = VirtualAlloc(baseAddress, (size_t) totalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

            if (memory.permanentStorage) {
                memory.transientStorage = (u8*) memory.permanentStorage + memory.permanentStorageSize;

                Render::Input input[2] = {};
                Render::Input* newInput = &input[0];
                Render::Input* oldInput = &input[1];

                {
                    LARGE_INTEGER freq;
                    QueryPerformanceFrequency(&freq);
                    Win32::g_perfCountFrequency = freq.QuadPart;
                }

                //char windowTitleString[128];

                s64 lastCounter = Win32::get_wall_clock();
                u64 lastCycleCount = __rdtsc();

                Win32::g_running = true;

                while (Win32::g_running) {
                    Render::Keyboard_Input* oldKeyboardInput = &oldInput->keyboard;
                    Render::Keyboard_Input* newKeyboardInput = &newInput->keyboard;
                    *newKeyboardInput = {};
                    for (int i = 0; i < arrayCount(newKeyboardInput->buttons); ++i) {
                        newKeyboardInput->buttons[i].endedDown = 
                            oldKeyboardInput->buttons[i].endedDown;
                    }

                    Render::Mouse_Input* oldMouseInput = &oldInput->mouse;
                    Render::Mouse_Input* newMouseInput = &newInput->mouse;
                    *newMouseInput = {};
                    for (int i = 0; i < arrayCount(newMouseInput->buttons); ++i) {
                        newMouseInput->buttons[i].endedDown = 
                            oldMouseInput->buttons[i].endedDown;
                    }
                    newMouseInput->cursor = oldMouseInput->cursor;

                    //g_refresh = false;
                    if (!Win32::process_pending_messages(newInput)) {
                        Win32::g_running = false;
                    }

                    for (int i = 0; i < arrayCount(newKeyboardInput->buttons); ++i) {
                        newKeyboardInput->buttons[i].changed = 
                            newKeyboardInput->buttons[i].endedDown != 
                            oldKeyboardInput->buttons[i].endedDown;

                        //assert(newKeyboardInput->buttons[i].halfTransistionCount == 
                        //    (newKeyboardInput->buttons[i].downCount + 
                        //     newKeyboardInput->buttons[i].upCount));
                    }

                    for (int i = 0; i < arrayCount(newMouseInput->buttons); ++i) {
                        newMouseInput->buttons[i].changed = 
                            newMouseInput->buttons[i].endedDown != 
                            oldMouseInput->buttons[i].endedDown;
                    }

                    newMouseInput->cursor.change.x =
                        newMouseInput->cursor.position.x -
                        oldMouseInput->cursor.position.x;

                    newMouseInput->cursor.change.y =
                        newMouseInput->cursor.position.y - 
                        oldMouseInput->cursor.position.y;


                    Render::Offscreen_Bitmap_Buffer bitmapBuffer = {};
                    bitmapBuffer.memory = Win32::g_bitmap.memory;
                    bitmapBuffer.width  = Win32::g_bitmap.width;
                    bitmapBuffer.height = Win32::g_bitmap.height;
                    bitmapBuffer.pitch  = Win32::g_bitmap.pitch;

                    //if (g_refresh) {
                    //    OutputDebugStringA("refresh");
                    //}
                    Render::update(&memory, newInput, &bitmapBuffer);
                    //OutputDebugStringA("\n");

                    // enforce frame rate
                    {
                        s64 workCounter = Win32::get_wall_clock();
                        f32 workSecondsElapsed = Win32::get_seconds_elapsed(lastCounter, workCounter);

                        f32 secondsElapsedForFrame = workSecondsElapsed;
                        if (secondsElapsedForFrame < targetSecondsElapsedPerFrame) {
                            if (sleepIsGranular) {
                                DWORD sleepMs = (DWORD)(1000.0f * (targetSecondsElapsedPerFrame - secondsElapsedForFrame));
                                if (sleepMs > 0) {
                                    Sleep(sleepMs);
                                }
                            }
                            while (secondsElapsedForFrame < targetSecondsElapsedPerFrame) {
                                secondsElapsedForFrame = Win32::get_seconds_elapsed(lastCounter, Win32::get_wall_clock());
                            }
                        } else {
                            // missed the target frame rate
                        }
                    }

                    dimension = Win32::get_window_dimension(window);
                    Win32::display_bitmap(device_context, dimension.width, dimension.height, &Win32::g_bitmap);

                    /*
                    if (-1 != snprintf(
                            windowTitleString,
                            sizeof(windowTitleString),
                            "mouse: [%d, %d]    mouse buttons [L %d, M %d, R %d]    %s",
                            newInput->mouse.cursor.position.x, newInput->mouse.cursor.position.y,
                            newInput->mouse.buttons[Render::MOUSE_BUTTON_LEFT].endedDown,
                            newInput->mouse.buttons[Render::MOUSE_BUTTON_MIDDLE].endedDown,
                            newInput->mouse.buttons[Render::MOUSE_BUTTON_RIGHT].endedDown,
                            Platform::DEBUG_window_text)) {
                        SetWindowTextA(window, windowTitleString);
                    }
                    */

                    swap(newInput, oldInput);

                    s64 endCounter = Win32::get_wall_clock();
                    u64 endCycleCount = __rdtsc();

#ifdef RENDERDEV_FPS
                    s64 counterElapsed = endCounter - lastCounter;
                    u64 cyclesElapsed = endCycleCount - lastCycleCount;
                    f64 millisecondsPerFrame = 1000.0f * Win32::get_seconds_elapsed(lastCounter, endCounter);

                    // counts gives ~microsecond accuracy
                    f64 fps = (f64)Win32::g_perfCountFrequency / (f64)counterElapsed;
                    f64 megaCyclesPerFrame = (f64)cyclesElapsed / 1e6f; // cycle = cpu instruction
                    {
                        char fpsBuf[128];
                        snprintf(fpsBuf, sizeof(fpsBuf), "%.02f ms/f, %.02f f/s, %.02f mc/f\n", millisecondsPerFrame, fps, megaCyclesPerFrame);
                        OutputDebugStringA(fpsBuf);
                    }
#endif



                    lastCounter = endCounter;
                    lastCycleCount  = endCycleCount;

                }
            } else {
                // TODO error VirtualAlloc failed
            }

        } else {
            // TODO error
        }
    } else {
        // TODO error
    }

    return 0;
}
