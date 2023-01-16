#ifndef LOG_H
#define LOG_H

#include <gtk/gtk.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define CONCATE(s1, s2) s1##s2

#define TAG __FILE__
#define n_print(...) g_print(TAG ": " __VA_ARGS__)
#define n_debug(...) g_debug(TAG ": " __VA_ARGS__)
#define n_warning(...) g_warning(TAG ": " __VA_ARGS__)
#define n_error(...) g_error(TAG ": " __VA_ARGS__)

#endif
