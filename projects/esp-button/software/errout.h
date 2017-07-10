#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

inline void errout(char *type, char *file, int line, char *format, ...) {
   va_list args;
   va_start(args, format);

   printf("[%s %s:%d] ", type, file, line);
   printf(format, args);
   printf("\n\r");
}

#define ERROUT(type, format, ...)\
   errout(type, __FILE__, __LINE__, format, ## __VA_ARGS__);

#ifdef VERBOSE
   #undef VERBOSE
   #define VERBOSE(format, ...)\
      ERROUT("VERBOSE", format, ## __VA_ARGS__);
#else
   #define VERBOSE(format, ...)
#endif

#ifdef DEBUG
   #undef DEBUG
   #define DEBUG(format, ...)\
      ERROUT("DEBUG", format, ## __VA_ARGS__);
#else
   #define DEBUG(format, ...)
#endif