#ifndef RENDERDEV_PLATFORM_H
#define RENDERDEV_PLATFORM_H

namespace Platform {
    void report_error(const char* format, ...);
    // #ifdef RENDERDEV_DEBUG ...
    void DEBUG_printf(const char* format, ...);
    void DEBUG_display(const char* format, ...);
}

#endif // RENDERDEV_PLATFORM_H
