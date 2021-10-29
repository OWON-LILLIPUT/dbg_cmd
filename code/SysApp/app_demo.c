
/*
 * file encoding utf-8
 *
 * app_demo.c
 *
 * Copyright (C) 2021 Inker.Dong
 *
 * Changelog:
 *
 *   2021-08-03 by Inker.Dong
 *   First create.
 *
 */

#include "app_demo.h"

#if ( APP_DEMO_MODULE_EN == 1 )

/************************************************************************/
/*      Print Message                                                   */
/************************************************************************/
#define MODULE_PRINT_INFO_EN // 屏蔽不定义 关闭此模块打印信息
#ifdef MODULE_PRINT_INFO_EN

#include "myprint.h"  /* 其他地方myprint.h不可出现此之前 */

#ifdef PRINTF_INFO_EN
static int prn_level = 0;/* setting module print level */
#endif

#else

#define PRINT(fmt, ...)             ( (void)0 )
#define PRN_ERR(fmt, ...)           ( (void)0 )
#define PRN_HEXS(a,b)               ( (void)0 )
#define PRN_LEVEL(a,b,fmt, ...)     ( (void)0 )

#endif
/************************************************************************/

struct app_demo_cfg_s {
    unsigned int id;
};
const struct app_demo_cfg_s app_demo_cfg = {
    0X01,
};

/************************************************************************/
/*      static Variables                                                */
/************************************************************************/
struct app_demo_state_s {
    unsigned int time_cnt_10ms;
    unsigned int time_cnt_100ms;
    unsigned int time_cnt_real;
};
struct app_demo_state_s app_demo_sate;

/************************************************************************/
/*      Local Functions                                                 */
/************************************************************************/

/************************************************************************/
/*     dbg_cmd Interface                                                */
/************************************************************************/
#include "dbg_cmd.h" // 屏蔽关闭此模块命令行调试
#ifdef DBG_CMD_EN
static bool dbg_cmd_func()
{
#define CMD_NAME ".AppDemo"
    const struct  app_demo_cfg_s   *p_cfg = &app_demo_cfg;
    struct  app_demo_state_s *p_sta = &app_demo_sate;
    if (dbg_cmd_exec("help", "", "")) {
        DBG_CMD_PRN(CMD_NAME"\r\n");
        return false;
    }
    if (dbg_cmd_exec(".exit", "", "")) {
#ifdef MODULE_PRINT_INFO_EN
        prn_level = 0;
#endif
        return false;
    }
    if (dbg_cmd_exec(CMD_NAME, "", "")) {
        dbg_cmd_print_msg_en();
    }
    if (dbg_cmd_exec("DemoMsg", "", "")) {
#ifdef MODULE_PRINT_INFO_EN
        DBG_CMD_PRN("PrnLevel  :%d\r\n", prn_level);
#endif
        DBG_CMD_PRN("time_cnt_10ms :%d\r\n", p_sta->time_cnt_10ms);
        DBG_CMD_PRN("time_cnt_100ms:%d\r\n", p_sta->time_cnt_100ms);
        DBG_CMD_PRN("time_cnt_real :%d\r\n", p_sta->time_cnt_real);
        DBG_CMD_PRN("cfg id :%d\r\n", p_cfg->id);
        return true;
    }
#ifdef MODULE_PRINT_INFO_EN
    if (dbg_cmd_exec("DemoPrf", "1", "<0~1> Set Prn Level")) {
        prn_level = get_param_char(0);
        return true;
    }
#endif
    if (dbg_cmd_exec("TestStr", "ss", "\"str1\" \"str2\"")) {
        DBG_CMD_PRN("str1:%s str2:%s\r\n", get_param_string(0), get_param_string(1));
        return true;
    }
    return false;
}
#endif
/************************************************************************/
/*      Application Interface                                           */
/************************************************************************/
void app_demo_10ms_thread_isr()
{
    struct app_demo_state_s *p_sta = &app_demo_sate;
    p_sta->time_cnt_10ms++;
}

void app_demo_100ms_thread()
{
    struct app_demo_state_s *p_sta = &app_demo_sate;
    static int Cnt = 0;
    p_sta->time_cnt_100ms++;
    Cnt++;
    if (Cnt >= 10) {
        Cnt = 0;
        PRN_LEVEL(prn_level, 1, "prn 1 msg\r\n");
        PRN_LEVEL(prn_level, 2, "prn 2 msg\r\n");
    }
}

void app_demo_real_time_thread()
{
    struct app_demo_state_s *p_sta = &app_demo_sate;
    p_sta->time_cnt_real++;
}

void app_demo_init()
{
#ifdef DBG_CMD_EN
    dbg_cmd_add_list((CMD_FUNC_T)dbg_cmd_func);
#endif
}
#endif

