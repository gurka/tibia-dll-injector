#ifndef TRACE_H_
#define TRACE_H_

#include <cstdio>
#include <ctime>
#include <cstdarg>
#include <cstring>

// Used to remove directories in __FILE__ ("network/server.cc" => "server.cc")
#ifdef _WIN32
#define FILE (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define FILE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

// Used to turn trace groups on or off, comment out to turn off
#define TRACE_ERROR_ON
#define TRACE_INFO_ON
//#define TRACE_DEBUG_ON

// Trace groups
#ifdef TRACE_ERROR_ON
  #define TRACE_ERROR(...) Trace::trace(FILE, __LINE__, "ERROR", __VA_ARGS__)
#else
  #define TRACE_ERROR(...)
#endif

#ifdef TRACE_INFO_ON
  #define TRACE_INFO(...) Trace::trace(FILE, __LINE__, "INFO", __VA_ARGS__)
#else
  #define TRACE_INFO(...)
#endif

#ifdef TRACE_DEBUG_ON
  #define TRACE_DEBUG(...) Trace::trace(FILE, __LINE__, "DEBUG", __VA_ARGS__)
#else
  #define TRACE_DEBUG(...)
#endif

class Trace {
 public:
  static void trace(const char* file, int line, const char* prefix, ...) {
    // Get current date and time
    time_t now = time(0);
    char time_str[32];
    struct tm* tstruct = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %X", tstruct);

    // Extract variadic function arguments
    va_list args;
    va_start(args, prefix);  // Start to extract after the "prefix"-argument
    const char* format = va_arg(args, const char*);  // Extract the format string
    char message[1024];
    // Use the rest of the arguments together with the
    // format string to construct the actual trace message
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    printf("[%s][%s:%d] %s: %s\n", time_str, file, line, prefix, message);
  }
};

#endif  // TRACE_H_
