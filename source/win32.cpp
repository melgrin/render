#include <stdint.h>
#include <math.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

#define local_persist static
#define internal static
#define global static

#include "render.cpp"

#include <windows.h>
#include <windowsx.h> // GET_X_LPARAM, GET_Y_LPARAM
#include <stdio.h>

namespace Platform {

#ifdef RENDERDEV_DEBUG_PRINT
//XXX Warning: this is always behind by one message, at least in remedybg. Now I'm mainly just putting relevant info in the window title (SetWindowText).
void DEBUG_printf(const char* format, ...) {
    local_persist char buf[512];
    va_list args;
    va_start(args, format);
    vsprintf_s(buf, sizeof(buf), format, args);
    va_end(args);
    OutputDebugStringA(buf);
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

void process_keyboard_message(bool isDown, Render::Button_State* state) {
    assert(state->endedDown != isDown); // This happens if you press W and up arrow together, but otherwise, one interrupts the other, and then you're just holding one and nothing is happening.  So need better logic here.
    state->endedDown = isDown;
    ++state->halfTransistionCount;
}

bool process_pending_messages(Render::Keyboard_Input* keyboard, Render::Mouse_Input* mouse) {
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
                if (wasDown != isDown) {
                    if (key == 'W') {
                        process_keyboard_message(isDown, &keyboard->buttons[Render::BUTTON_UP]);
                    } else if (key == 'S') {
                        process_keyboard_message(isDown, &keyboard->buttons[Render::BUTTON_DOWN]);
                    } else if (key == 'A') {
                        process_keyboard_message(isDown, &keyboard->buttons[Render::BUTTON_LEFT]);
                    } else if (key == 'D') {
                        process_keyboard_message(isDown, &keyboard->buttons[Render::BUTTON_RIGHT]);
                    } else if (key == VK_UP) {
                        process_keyboard_message(isDown, &keyboard->buttons[Render::BUTTON_UP]);
                    } else if (key == VK_DOWN) {
                        process_keyboard_message(isDown, &keyboard->buttons[Render::BUTTON_DOWN]);
                    } else if (key == VK_LEFT) {
                        process_keyboard_message(isDown, &keyboard->buttons[Render::BUTTON_LEFT]);
                    } else if (key == VK_RIGHT) {
                        process_keyboard_message(isDown, &keyboard->buttons[Render::BUTTON_RIGHT]);
                    } else if (key == VK_ESCAPE) {
                        running = false;
                    } else if (key == VK_SPACE) {
                    }
                }
                // Since we're handling SYSKEYUP/SYSKEYDOWN, need to do this to restore ALT+F4 functionality.
                bool altKeyWasDown = ((message.lParam & (1 << 29)) != 0);
                if (key == VK_F4 && altKeyWasDown) {
                    running = false;
                }
                break;
            }

            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN: {
                mouse->x = GET_X_LPARAM(message.lParam);
                mouse->y = GET_Y_LPARAM(message.lParam);
                mouse->isButtonPressed = message.wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON);
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
        case WM_RBUTTONDOWN: {
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
    int gameUpdateHz = monitorRefreshHz / 2;
    f32 targetSecondsElapsedPerFrame = 1.0f / (f32) gameUpdateHz;

    if (RegisterClass(&window_class)) {

        HWND window = CreateWindowEx(
            0,
            window_class.lpszClassName,
            "RenderTest",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            instance,
            0);

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
            memory.transientStorage = (u8*) memory.permanentStorage + memory.permanentStorageSize;

            if (memory.permanentStorage && memory.transientStorage) {

                Render::Keyboard_Input keyboardInput[2] = {};
                Render::Keyboard_Input* oldKeyboardInput = &keyboardInput[0];
                Render::Keyboard_Input* newKeyboardInput = &keyboardInput[1];

                Render::Mouse_Input mouseInput = {};

                {
                    LARGE_INTEGER freq;
                    QueryPerformanceFrequency(&freq);
                    Win32::g_perfCountFrequency = freq.QuadPart;
                }

                char windowTitleString[128];

                s64 lastCounter = Win32::get_wall_clock();
                u64 lastCycleCount = __rdtsc();

                Win32::g_running = true;

                while (Win32::g_running) {

                    *newKeyboardInput = {};
                    for (int i = 0; i < arrayCount(newKeyboardInput->buttons); ++i) {
                        newKeyboardInput->buttons[i].endedDown = 
                            oldKeyboardInput->buttons[i].endedDown;
                    }

                    if (!Win32::process_pending_messages(newKeyboardInput, &mouseInput)) {
                        Win32::g_running = false;
                    }

                    Render::Offscreen_Bitmap_Buffer bitmapBuffer = {};
                    bitmapBuffer.memory = Win32::g_bitmap.memory;
                    bitmapBuffer.width  = Win32::g_bitmap.width;
                    bitmapBuffer.height = Win32::g_bitmap.height;
                    bitmapBuffer.pitch  = Win32::g_bitmap.pitch;

                    Render::update(&memory, newKeyboardInput, &mouseInput, &bitmapBuffer);

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

                    if (-1 != sprintf_s(
                            windowTitleString,
                            sizeof(windowTitleString),
                            "mouse: [%d, %d]    size: [%d, %d]    bitmap: [%d, %d]",
                            mouseInput.x, mouseInput.y,
                            dimension.width, dimension.height,
                            Win32::g_bitmap.width, Win32::g_bitmap.height)) {
                        SetWindowTextA(window, windowTitleString);
                    }

                    swap(newKeyboardInput, oldKeyboardInput);

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
                        sprintf_s(fpsBuf, sizeof(fpsBuf), "%.02f ms/f, %.02f f/s, %.02f mc/f\n", millisecondsPerFrame, fps, megaCyclesPerFrame);
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
