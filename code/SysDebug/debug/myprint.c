/*
 * file encoding utf-8
 *
 * myprint.c
 *
 * Copyright (C) 2021 Inker.Dong
 *
 * Changelog:
 *
 *   2021-07-25 by Inker.Dong
 *   First create.
 */

#include "myprint.h"

#ifdef PRINTF_INFO_EN

typedef bool (*print_func)(char);

struct myprint_cfg_s {
    char  print_buf[PRINT_BUF_SIZE];       //格式打印缓存地址
    print_func print_out;
};
struct myprint_cfg_s myprint_cfg;

void print_init(PUT_FUNC_T print_out_func)
{
    myprint_cfg.print_out = (print_func)print_out_func;
}

int myprint_func(const char *format, ...)
{
    int length = 0, i = 0;
    va_list arg_ptr;
    va_start (arg_ptr, format);           /* format string */
    vsnprintf (myprint_cfg.print_buf, PRINT_BUF_SIZE, format, arg_ptr);
    va_end (arg_ptr);
    length = strlen(myprint_cfg.print_buf);
    for (i = 0; i < length; i++) {
        myprint_cfg.print_out(myprint_cfg.print_buf[i]);
    }
    return length;
}
void print_hex_array(char hex[], int len)
{
    int i;
    myprint_func("\r\n");
    myprint_func("     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
    for (i = 0; i < len; i ++) {
        if ( i % 16 == 0 ) {
            myprint_func("\r\n0x%0.2X:", i);
        }
        myprint_func("%0.2X ", hex[i]);
    }
    myprint_func("\r\n");
}
void print_level(int set_print_level, const int print_level, const char *format, ...)
{
    if (print_level <= set_print_level) {
        int length, i;
        va_list arg_ptr;
        va_start (arg_ptr, format);
        vsnprintf (myprint_cfg.print_buf, PRINT_BUF_SIZE, format, arg_ptr);
        length = strlen(myprint_cfg.print_buf);
        for (i = 0; i < length; i++) {
            myprint_cfg.print_out(myprint_cfg.print_buf[i]);
        }
        va_end (arg_ptr);
    }
}

#endif
