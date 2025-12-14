#include "debug.h"
#include <stdio.h>

static const char* const g_LogSeverityColors[] =
{
    [LVL_DEBUG]        = "\033[2;37m",
    [LVL_INFO]         = "\033[37m",
    [LVL_WARN]         = "\033[1;33m",
    [LVL_ERROR]        = "\033[1;31m",
    [LVL_CRITICAL]     = "\033[1;37;41m",
};

static const char* const g_ColorReset = "\033[0m";

void logf(const char* module, DebugLevel level, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    if (level < MIN_LOG_LEVEL)
        return;

    fputs(g_LogSeverityColors[level], VFS_FD_DEBUG);    // set color depending on level
    fprintf(VFS_FD_DEBUG, "[%s] ", module);             // write module
    vfprintf(VFS_FD_DEBUG, fmt, args);                  // write text
    fputs(g_ColorReset, VFS_FD_DEBUG);                  // reset format
    fputc('\n', VFS_FD_DEBUG);                          // newline

    va_end(args);  
}

// // Early log: safe for boot
// void logearly(const char* module, DebugLevel level, const char* fmt, ...)
// {
//     char buf[128];  // keep stack small
//     va_list args;
//     va_start(args, fmt);
//     int len = ksnprintf(buf, sizeof(buf), fmt, args);  // or your own vsnprintf
//     va_end(args);

//     if (len > 0)
//     {
//         // prepend module/severity if needed
//         for (int i = 0; buf[i]; i++)
//             fb_putc(buf[i]);
//         fb_putc('\n');
//     }
// }
