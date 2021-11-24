#ifndef __DBG_CMD_H
#define __DBG_CMD_H

#include "myprint.h"

#ifdef PRINTF_INFO_EN // 打印输出定义有效 命令行才可用
#define DBG_CMD_EN    // 屏蔽关闭所以模块命令行功能
#endif


#ifdef DBG_CMD_EN

// mcu function address type
typedef	unsigned int CMD_FUNC_T;

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#define DBG_CMD_PRN(format, ...) my_print_format(format, ##__VA_ARGS__)
//#define DBG_CMD_PRN(format, ...) printf(format, ##__VA_ARGS__)

#define DBG_CMD_MODULE_EN // 屏蔽时关闭自身模块命令功能

// cmd config list
#define DBG_CMD_LIST_NUM       32 // 最大注册模块个数
#define DBG_CMD_BUF_LEN        32 // 接收最大命令长度
#define PARAM_STR_LEN          16 // 接收字符串参数类型 字符长度
#define PARAM_FLT_LEN          10 // 接收浮点型参数类型 字符长度
#define PARAM_1BYTE_NUM        3  // "1" 1byte
#define PARAM_2BYTE_NUM        3  // "2" 2byte
#define PARAM_4BYTE_NUM        1  // "4" 4byte  0:disable build
#define PARAM_FOLAT_NUM        1  // "f" float  0:disable build
#define PARAM_STRING_NUM       2  // "s" string 0:disable build

extern char  get_param_char(char index);
extern short get_param_short(char index);
#if ( PARAM_4BYTE_NUM >= 1 )
extern long  get_param_long(char index);
#endif
#if ( PARAM_FOLAT_NUM >= 1 )
extern float get_param_float(char index);
#endif
#if ( PARAM_STRING_NUM >= 1 )
extern char *get_param_string(char index);
#endif

/*
 * types: "" "1" "2" "4" "f" "s"
 * eg: func(char,float,short,char) cmd:setcmd
 *     if(dbg_cmd_exec("setcmd","1f21", "held message")) {
 *        func(get_param_char(0),
 *             get_param_float(0),
 *             get_param_short(0),
 *             get_param_char(1))
 *     }
 */
extern bool dbg_cmd_exec(char *cmd, char param_table[], char help_msg[]);

/*
 * #include "dbg_cmd.h" // on/off dbg_cmd
 * #ifdef DBG_CMD_EN
 * static bool dbg_cmd_func()
 * {
 *    dbg_cmd_exec("help", "", ""){
 *        DPRINTF((".xxxx\r\n"));
 *        return false;
 *    }
 *
 * }
 * #endif
 *
 * xxx_init()
 * {
 * #ifdef DBG_CMD_EN
 *     dbg_cmd_add_list((int)dbg_cmd_func);
 * #endif
 * }
 */
extern void dbg_cmd_add_list(CMD_FUNC_T func_addr);
extern void dbg_cmd_print_msg_en(void);
extern void dbg_cmd_1s_thread(void);
extern void dbg_cmd_rxd(char rxd);
extern void dbg_cmd_init(char dbg_cmd_en);
#endif

#endif

