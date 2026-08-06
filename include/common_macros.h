#ifndef _COMMMON_MACROS_H
#define _COMMMON_MACROS_H


#include <cstdio>



#if 1
#  define LOG_COLOR_RESET   "\033[0m"
#  define LOG_COLOR_RED     "\033[1;31m"
#  define LOG_COLOR_GREEN   "\033[1;32m"
#  define LOG_COLOR_YELLOW  "\033[1;33m"
#  define LOG_COLOR_BLUE    "\033[1;34m"
#else
#  define LOG_COLOR_RESET   ""
#  define LOG_COLOR_RED     ""
#  define LOG_COLOR_GREEN   ""
#  define LOG_COLOR_YELLOW  ""
#  define LOG_COLOR_BLUE    ""
#endif


#define  PRINT_INFO_G(format, ...)  printf(LOG_COLOR_GREEN format LOG_COLOR_RESET "\n", ##__VA_ARGS__)
#define  PRINT_INFO(format, ...)    printf(format "\n", ##__VA_ARGS__)
#define  PRINT_ERROR(format, ...)   printf(LOG_COLOR_RED format LOG_COLOR_RESET "\n", ##__VA_ARGS__)

// // 黄色警告（可选）
// #define PRINT_WARN(format, ...) \
//     printf(LOG_COLOR_YELLOW format LOG_COLOR_RESET "\n", ##__VA_ARGS__)

// // 蓝色调试（可选）
// #define PRINT_DEBUG(format, ...) \
//     printf(LOG_COLOR_BLUE format LOG_COLOR_RESET "\n", ##__VA_ARGS__)


#endif
