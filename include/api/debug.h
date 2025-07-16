#ifndef DEBUG_H__
#define DEBUG_H__

#include <stdio.h>
#include <string.h>

/* bold */

#define MSG_COLOR_RED(msg)          "\033[0;31m"msg"\033[0m"
#define MSG_COLOR_YELLOW(msg)       "\033[0;33m"msg"\033[0m"
#define MSG_COLOR_CYAN(msg)         "\033[0;36m"msg"\033[0m" 
#define MSG_COLOR_MAG(msg)          "\033[0;35m"msg"\033[0m"

//#define __DEBUG__

#define __FILENAME__    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#ifdef __DEBUG__
#define dlp_note(fmt, args...)   printf("[%s, %s(), %d] %s "fmt"\n", __FILENAME__, __FUNCTION__, __LINE__, MSG_COLOR_CYAN("note:"), ##args)
#define dlp_err(fmt, args...)   printf("[%s, %s(), %d] %s "fmt"\n", __FILENAME__, __FUNCTION__, __LINE__, MSG_COLOR_RED("error:"), ##args)
#define dlp_dbg(fmt, args...)   printf("[%s, %s(), %d] %s "fmt"\n", __FILENAME__, __FUNCTION__, __LINE__, MSG_COLOR_MAG("debug:"), ##args)
#define dlp_warn(fmt, args...)   printf("[%s, %s(), %d] %s "fmt"\n", __FILENAME__, __FUNCTION__, __LINE__, MSG_COLOR_YELLOW("warning:"), ##args)
#define dbp(ptr, size, des) \
        if (des) { \
            printf("[%s]==>", des); \
        } \
        for (int i = 0; i < size; i++) { \
            if ((i % 16) == 0) { \
                printf("\n| "); \
            } \
            printf("%02X ", ptr[i]); \
        } \
        printf("\n");
#else
#define dlp_note(fmt, args...)   printf("[%s, %s(), %d] %s "fmt"\n", __FILENAME__, __FUNCTION__, __LINE__, MSG_COLOR_CYAN("note:"), ##args)
#define dlp_err(fmt, args...)   printf("[%s, %s(), %d] %s "fmt"\n", __FILENAME__, __FUNCTION__, __LINE__, MSG_COLOR_RED("error:"), ##args)
#define dlp_dbg(fmt, args...)
#define dlp_warn(fmt, args...)
#define dbp(ptr, size, des)
#endif
#endif
