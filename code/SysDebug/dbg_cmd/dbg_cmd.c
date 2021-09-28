/*
 *
 * file encoding utf-8
 *   ___                                          _
 *  / __|  ___   _ __    _ __    __ _   _ _    __| |  ___   _ _
 * | (__  / _ \ | '  \  | '  \  / _` | | ' \  / _` | / -_) | '_|
 *  \___| \___/ |_|_|_| |_|_|_| \__,_| |_||_| \__,_| \___| |_|
 *
 *                            Release V1.1.1 Powered by Inker.Dong
 *
 * dgb_cmd.c
 *
 * Copyright (C) 2012 Inker.Dong
 *
 *
 * Change Activity:
 *
 *   2018-07-05 by Inker.Dong
 *   First create.
 *   2021-07-05 by Inker.Dong
 *   Add  _8051_MCU Config
 *   2021-07-14 23:13:57
 *       DbgParams(char *cmd, char param_table[])
 *       DbgParams(char *cmd, char param_table[], char help_msg[])
 *   2021-08-07 19:37:07 Inker.Dong
 *      Add param string num config
 *   2021-09-11 08:54:47 Inker.Dong
 *      Add float 4byte string param #if (n >= 1) #endif
 *      Add DBG_CMD_MODULE_EN on/off
 *      Del bool param type
 *      Del LogoName
 *
 */

#include "dbg_cmd.h"

#ifdef DBG_CMD_EN

struct dbg_cmd_s {
    char             param_char_buf[PARAM_1BYTE_NUM];
    short            param_short_buf[PARAM_2BYTE_NUM];
#if ( PARAM_4BYTE_NUM >= 1 )
    long             param_long_buf[PARAM_4BYTE_NUM];
#endif
#if ( PARAM_FOLAT_NUM >= 1 )
    float            param_float_buf[PARAM_FOLAT_NUM];
#endif
#if ( PARAM_STRING_NUM >= 1 )
    char             param_str_buf[PARAM_STRING_NUM][PARAM_STR_LEN];
#endif
    unsigned char    param_char_cnt;
    unsigned char    param_shor_cnt;
    unsigned char    param_long_cnt;
    unsigned char    param_float_cnt;
    unsigned char    param_str_cnt;
    unsigned char    cmd_en;
    unsigned char    cmd_param_state;
    unsigned char    rxd_cmd_cnt;
    char             rxd_cmd_buf[DBG_CMD_BUF_LEN];// \n \0
    unsigned char    cmd_str_pos;
    unsigned char    hours;
    unsigned char    minutes;
    unsigned char    seconds;
    unsigned char    prn_cmd_msg_en;
    unsigned char    dbg_cmd_list_cnt;
    int              dab_cmd_list_buf[DBG_CMD_LIST_NUM];
};

struct dbg_cmd_s dbg_cmd;

/*将字符串命令与参数之间第一个出现的字符fcahr替换为bchar */
static bool search_replace_char( char *str, char fchar, char bchar, char len)
{
    char i = 0;
    while (i < DBG_CMD_BUF_LEN) {
        if (*str == fchar) {
            *str = bchar;
            return true;
        }
        if (*str == '\r') { //遇到回车 结束搜索
            return false;
        }
        str++;
        i++;
    }
    return false;
}
/*大写字母转小写字母*/
static void upper_to_lower(char *str)
{
    while (*str != '\0') {
        if (*str >= 'A' && *str <= 'Z') {
            *str = *str + 32;
        }
        str++;
    }
}

static unsigned char search_char_pos(char *str, char asiic) /*搜索字符并返回字符所在地址*/
{
    unsigned char i = 0;
    while (i < DBG_CMD_BUF_LEN) {
        if (*str == asiic) { /*搜索到返回地址*/
            return i;
        }
        str++;
        i++;
    }
    return 0XFF;
}

static bool cmd_end_is_enter(char *str)
{
    if (*str == '\r') {
        return true;
    } else {
        return false;
    }
}

static bool param_type_1byte(char *str, char *val) /*参数为8位类型*/
{
    bool result = true;
    char i = 0, temp8 = 0;
    if (*str == 'H' || *str == 'h') { //十六进制格式
        str++;
        dbg_cmd.cmd_str_pos++;
        while (i < 2) {
            if (*str >= '0' && *str <= '9') {
                temp8 <<= 4;
                temp8 += (*str) & 0X0F;
                str++;
                i++;
                dbg_cmd.cmd_str_pos++;
            } else if (*str >= 'a' && *str <= 'f') {
                temp8 <<= 4;
                temp8 += (*str) & 0X0F;
                temp8 += 9;
                str++;
                i++;
                dbg_cmd.cmd_str_pos++;
            } else {
                break;
            }
        }
        if (i == 0) { /*参数之间不是一个空格错误*/
            result = false;
        }
        if (i == 2) { /*数据范围错误*/
            if (*str >= 'a' && *str <= 'f') {
                result = false;
            }
            if (*str >= '0' && *str <= '9') {
                result = false;
            }
        }
    } else {
        while (i < 3) {
            if (*str >= '0' && *str <= '9') {
                temp8 *= 10;
                temp8 += (*str) & 0X0F;
                str++;
                i++;
                dbg_cmd.cmd_str_pos++;
            } else {
                break;
            }
        }
        if (i == 0) { /*参数之间不是一个空格错误*/
            result = false;
        }
        if (i == 3) { /*数据范围错误*/
            if (*str >= '0' && *str <= '9') {
                result = false;
            }
        }
    }
    *val = temp8;
    dbg_cmd.cmd_str_pos++;
    return result;
}

static bool param_type_2byte(char *str, short *val) /*参数为16位类型*/
{
    bool result = true;
    char i = 0;
    short temp16 = 0;
    if (*str == 'H' || *str == 'h') { //十六进制格式
        str++;
        dbg_cmd.cmd_str_pos++;
        while (i < 4) {
            if (*str >= '0' && *str <= '9') {
                temp16 <<= 4;
                temp16 += (*str) & 0X0F;
                str++;
                i++;
                dbg_cmd.cmd_str_pos++;
            } else if (*str >= 'a' && *str <= 'f') {
                temp16 <<= 4;
                temp16 += (*str) & 0X0F;
                temp16 += 9;
                str++;
                i++;
                dbg_cmd.cmd_str_pos++;
            } else {
                break;
            }
        }
        if (i == 0) { /*参数之间不是一个空格错误*/
            result = false;
        }
        if (i == 4) { /*数据范围错误*/
            if (*str >= 'a' && *str <= 'f') {
                result = false;
            }
            if (*str >= '0' && *str <= '9') {
                result = false;
            }
        }
    } else {
        while (i < 6) {
            if (*str >= '0' && *str <= '9') {
                temp16 *= 10;
                temp16 += (*str) & 0X0F;
                str++;
                i++;
                dbg_cmd.cmd_str_pos++;
            } else {
                break;
            }
        }
        if (i == 0) { /*参数之间不是一个空格错误*/
            result = false;
        }
        if (i == 6) { /*数据范围错误*/
            if (*str >= '0' && *str <= '9') {
                result = false;
            }
        }
    }
    *val = temp16;
    dbg_cmd.cmd_str_pos++;
    return result;
}

#if ( PARAM_4BYTE_NUM >= 1 )
static bool param_type_4byte(char *str, long *val) /*参数为16位类型*/
{
    bool result = true;
    char  i = 0;
    long tempu32 = 0;
    if (*str == 'H' || *str == 'h') { //十六进制格式
        str++;
        dbg_cmd.cmd_str_pos++;
        while (i < 8) {
            if (*str >= '0' && *str <= '9') {
                tempu32 <<= 4;
                tempu32 += (*str) & 0X0F;
                str++;
                i++;
                dbg_cmd.cmd_str_pos++;
            } else if (*str >= 'a' && *str <= 'f') {
                tempu32 <<= 4;
                tempu32 += (*str) & 0X0F;
                tempu32 += 9;
                str++;
                i++;
                dbg_cmd.cmd_str_pos++;
            } else {
                break;
            }
        }
        if (i == 0) { /*参数之间不是一个空格错误*/
            result = false;
        }
        if (i == 8) { /*数据范围错误*/
            if (*str >= 'a' && *str <= 'f') {
                result = false;
            }
            if (*str >= '0' && *str <= '9') {
                result = false;
            }
        }
    } else {
        while (i < 10) {
            if (*str >= '0' && *str <= '9') {
                tempu32 *= 10;
                tempu32 += (*str) & 0X0F;
                str++;
                i++;
                dbg_cmd.cmd_str_pos++;
            } else {
                break;
            }
        }
        if (i == 0) { /*参数之间不是一个空格错误*/
            result = false;
        }
        if (i == 6) { /*数据范围错误*/
            if (*str >= '0' && *str <= '9') {
                result = false;
            }
        }
    }
    *val = tempu32;
    dbg_cmd.cmd_str_pos++;
    return result;
}
#endif

#if ( PARAM_FOLAT_NUM >= 1 )
static bool param_type_float(char *str, float *val) /*参数为浮点类型*/
{
    bool result = true;
    char i = 0, flag = 0, sign = 0;
    float tempf1 = 0, tempf2 = 0, tempf3 = 0;
    if (*str == '-') { //负号
        str++;
        dbg_cmd.cmd_str_pos++;
        sign = 1;
    }
    while (i < 8) {
        if (*str >= '0' && *str <= '9') {
            if (flag) {
                tempf3 = (*str) & 0X0F;
                if (flag == 1) {
                    tempf2 += tempf3 / 10;
                }
                if (flag == 2) {
                    tempf2 += tempf3 / 100;
                }
                if (flag == 3) {
                    tempf2 += tempf3 / 1000;
                }
                if (flag == 4) {
                    tempf2 += tempf3 / 10000;
                }
                if (flag == 5) {
                    tempf2 += tempf3 / 100000;
                }
                if (flag == 6) {
                    tempf2 += tempf3 / 1000000;
                }
                flag++;
            } else {
                tempf1 *= 10;
                tempf1 += (*str) & 0X0F;
            }
            str++;
            i++;
            dbg_cmd.cmd_str_pos++;
        } else if (*str == '.' && flag == 0) {
            flag = 1;
            str++;
            i++;
            dbg_cmd.cmd_str_pos++;
        } else {
            break;
        }
    }
    if (i == 0) { /*参数之间不是一个空格错误*/
        result = false;
    }
    if (i == 8) { /*数据范围错误*/
        if (*str >= '0' && *str <= '9') {
            result = false;
        }
    }
    tempf3 = tempf1 + tempf2;
    if (sign) {
        *val = -tempf3;
    } else {
        *val = tempf3;
    }
    dbg_cmd.cmd_str_pos++;
    return result;
}
#endif

#if ( PARAM_STRING_NUM >= 1 )
static bool param_type_str(char *str, char Array[]) /*参数为8位类型*/
{
    bool result = true;
    char i = 0;
    if (*str != '"') {
        return false;
    }
    str++;
    dbg_cmd.cmd_str_pos++;
    while (i < PARAM_STR_LEN - 3) {
        if (*str == '"') {
            dbg_cmd.cmd_str_pos++;
            Array[i] = '\0';
            break;
        } else {
            Array[i] = *str;
            str++;
            i++;
            dbg_cmd.cmd_str_pos++;
        }
    }
    if (i == 0) { /*参数之间不是一个空格错误*/
        result = false;
    }
    dbg_cmd.cmd_str_pos++;
    return result;
}
#endif

// Param Analy
static bool param_type_analy(char param_type)
{
    switch (param_type) {
        case '1':
            if (dbg_cmd.param_char_cnt >= PARAM_1BYTE_NUM) {
                return false;
            }
            if (param_type_1byte(dbg_cmd.rxd_cmd_buf + dbg_cmd.cmd_str_pos, dbg_cmd.param_char_buf + dbg_cmd.param_char_cnt) == false) {
                return false;
            }
            dbg_cmd.param_char_cnt++;
            break;
        case '2':
            if (dbg_cmd.param_shor_cnt >= PARAM_2BYTE_NUM) {
                return false;
            }
            if (param_type_2byte(dbg_cmd.rxd_cmd_buf + dbg_cmd.cmd_str_pos, dbg_cmd.param_short_buf + dbg_cmd.param_shor_cnt) == false) {
                return false;
            }
            dbg_cmd.param_shor_cnt++;
            break;
#if ( PARAM_4BYTE_NUM >= 1 )
        case '4':
            if (dbg_cmd.param_long_cnt >= PARAM_4BYTE_NUM) {
                return false;
            }
            if (param_type_4byte(dbg_cmd.rxd_cmd_buf + dbg_cmd.cmd_str_pos, dbg_cmd.param_long_buf + dbg_cmd.param_long_cnt) == false) {
                return false;
            }
            dbg_cmd.param_long_cnt++;
            break;
#endif
#if ( PARAM_FOLAT_NUM >= 1 )
        case 'f':
            if (dbg_cmd.param_float_cnt >= PARAM_FOLAT_NUM) {
                return false;
            }
            if (param_type_float(dbg_cmd.rxd_cmd_buf + dbg_cmd.cmd_str_pos, dbg_cmd.param_float_buf + dbg_cmd.param_float_cnt) == false) {
                return false;
            }
            dbg_cmd.param_float_cnt++;
            break;
#endif
#if ( PARAM_STRING_NUM >= 1 )
        case 's':
            if (dbg_cmd.param_str_cnt >= PARAM_STRING_NUM) {
                return false;
            }
            if (param_type_str(dbg_cmd.rxd_cmd_buf + dbg_cmd.cmd_str_pos, dbg_cmd.param_str_buf[dbg_cmd.param_str_cnt]) == false) {
                return false;
            }
            dbg_cmd.param_str_cnt++;
            break;
#endif
        default:
            return false;
            //break;
    }
    return true;
}

typedef bool (*ComDbgFunc)(void);
static bool dbg_cmd_analy(void)
{
    //static volatile char CntTask = 0;//使用临时变量i会莫名被清零??? 2021-07-05 23:43:21
    char res, cmd_len, CntTask;
    ComDbgFunc Func;
    /* 命令字符串与参数隔离 并且命令字符串不区分大小写字母 而参数区分大小写 */
    upper_to_lower(dbg_cmd.rxd_cmd_buf);              /* 将大写字母转成小写 从而不区分大小写*/
    cmd_len = strlen(dbg_cmd.rxd_cmd_buf);
    if (search_replace_char(dbg_cmd.rxd_cmd_buf, ' ', '\0', cmd_len)) {
        dbg_cmd.cmd_param_state = 1;
    } else {
        dbg_cmd.cmd_param_state = 0;
        search_replace_char(dbg_cmd.rxd_cmd_buf, '\r', '\0', cmd_len);
    }
    if (dbg_cmd.cmd_en == 0) {
        if (strcmp((const char *)dbg_cmd.rxd_cmd_buf, "inker") == NULL) {
            dbg_cmd.cmd_en = 1;
            DBG_CMD_PRN("Welcom Inker System Debug Mode!\r\n");
            memcpy(dbg_cmd.rxd_cmd_buf, "help\r\n", 7); /* 登录默认命令为打印帮助信息 */
            goto success;
        }
        goto fail;
    }
    for (CntTask = 0; CntTask < dbg_cmd.dbg_cmd_list_cnt; CntTask++) {
        Func = (ComDbgFunc)dbg_cmd.dab_cmd_list_buf[CntTask];
        res = Func();
        if (dbg_cmd.prn_cmd_msg_en) {
            dbg_cmd.prn_cmd_msg_en = 0;
            goto success;
        }
        //DBG_CMD_PRN("CntTask:%d\r\n",CntTask);
        if (res == true) {
            //if(pFunc() == true) //直接使用会出函数返回值错误问题??? 2021-07-04 01:23:22
            goto success;
        }
    }
    if (strcmp((const char *)dbg_cmd.rxd_cmd_buf, "help") == NULL) {
        goto success;
    }
    if (strcmp((const char *)dbg_cmd.rxd_cmd_buf, "exit") == NULL) {
        if (dbg_cmd.cmd_en) {
            dbg_cmd.cmd_en = 0;
            DBG_CMD_PRN("Exit Inker Debug System!\r\n");
            goto fail;
        }
    }
fail:
    if (dbg_cmd.cmd_param_state) {
        search_replace_char(dbg_cmd.rxd_cmd_buf, '\0', ' ', cmd_len);
    } else {
        search_replace_char(dbg_cmd.rxd_cmd_buf, '\0', '\r', cmd_len);
    }
    return false;
success:
    if (dbg_cmd.cmd_param_state) {
        search_replace_char(dbg_cmd.rxd_cmd_buf, '\0', ' ', cmd_len);
    } else {
        search_replace_char(dbg_cmd.rxd_cmd_buf, '\0', '\r', cmd_len);
    }
    return true;
}
/************************************************************************/
/*     dbg_cmd Interface                                                */
/************************************************************************/
#ifdef DBG_CMD_MODULE_EN
static bool dbg_cmd_func()
{
    if (dbg_cmd_exec("help", "", "")) {
        DBG_CMD_PRN(".Dbg\r\n");
        return false;
    }
    if (dbg_cmd_exec(".dbg", "", "")) {
        dbg_cmd_print_msg_en();
    }
    if (dbg_cmd_exec("dbgmsg", "", "")) {
        DBG_CMD_PRN("cmd_list:%d(%d)\r\n", dbg_cmd.dbg_cmd_list_cnt, DBG_CMD_LIST_NUM);
        DBG_CMD_PRN("ParamS:%d(Len:%d)\r\n",PARAM_STRING_NUM, PARAM_STR_LEN);
        DBG_CMD_PRN("Param1:%d\r\n", PARAM_1BYTE_NUM);
        DBG_CMD_PRN("Param2:%d\r\n", PARAM_2BYTE_NUM);
        DBG_CMD_PRN("Param4:%d\r\n", PARAM_4BYTE_NUM);
        DBG_CMD_PRN("ParamF:%d\r\n", PARAM_FOLAT_NUM);
        DBG_CMD_PRN("InPut 0X3A => h3a or H3a\r\n");
        return true;
    }
    if (dbg_cmd_exec("SetTime", "111", "<H> <M> <S>")) {
        if (get_param_char(1) < 60 && get_param_char(2) < 59) {
            dbg_cmd.hours   = get_param_char(0);
            dbg_cmd.minutes = get_param_char(1);
            dbg_cmd.seconds = get_param_char(2);
            return true;
        }
    }
    return false;
}
#endif

char get_param_char(char index)
{
    return dbg_cmd.param_char_buf[index];
}

short get_param_short(char index)
{
    return dbg_cmd.param_short_buf[index];
}

#if ( PARAM_4BYTE_NUM >= 1 )
long get_param_long(char index)
{
    return dbg_cmd.param_long_buf[index];
}
#endif

#if ( PARAM_FOLAT_NUM >= 1 )
float get_param_float(char index)
{
    return dbg_cmd.param_float_buf[index];
}
#endif

#if ( PARAM_STRING_NUM >= 1 )
char *get_param_string(char index)
{
    return (char *)dbg_cmd.param_str_buf[index];
}
#endif

bool dbg_cmd_exec(char *cmd, char param_table[], char help_msg[])
{
    char i, str_buf[DBG_CMD_BUF_LEN];
    if (dbg_cmd.prn_cmd_msg_en) {
        DBG_CMD_PRN("%-10s", cmd);
        if (strlen(param_table)) {
            DBG_CMD_PRN(" | %-6s", param_table);
        } else {
            DBG_CMD_PRN(" |       ");
        }
        if (strlen(help_msg)) {
            DBG_CMD_PRN(" | %-20s", help_msg);
        } else {
            DBG_CMD_PRN(" |");
        }
        DBG_CMD_PRN("\r\n");
        return false;
    }
    memcpy(str_buf, cmd, strlen(cmd)); /* 启动默认命令为打印帮助信息 */
    str_buf[strlen(cmd)] = '\0';
    upper_to_lower(str_buf);            /* 将大写字母转成小写 从而不区分大小写*/
    if (strcmp((const char *)dbg_cmd.rxd_cmd_buf, str_buf) != NULL) {
        return false;
    }
    if (dbg_cmd.cmd_param_state == 0) {
        if (strlen(param_table)) {
            goto fail;
        }
        return true;
    }
    dbg_cmd.param_char_cnt  = 0;
    dbg_cmd.param_shor_cnt  = 0;
    dbg_cmd.param_long_cnt  = 0;
    dbg_cmd.param_float_cnt = 0;
    dbg_cmd.param_str_cnt   = 0;
    dbg_cmd.cmd_str_pos = search_char_pos(dbg_cmd.rxd_cmd_buf, '\0') + 1;
    for (i = 0; i < strlen(param_table); i++) {
        if (param_type_analy(param_table[i]) == false) {
            goto fail;
        }
    }
    if (cmd_end_is_enter(dbg_cmd.rxd_cmd_buf + dbg_cmd.cmd_str_pos - 1) == false) {
        goto fail;
    }
    return true;
fail:
    DBG_CMD_PRN("%s %s %s\r\n", cmd, param_table, help_msg);
    return false;
}

void dbg_cmd_add_list(int func_addr)
{
    if (dbg_cmd.dbg_cmd_list_cnt < DBG_CMD_LIST_NUM) {
        dbg_cmd.dab_cmd_list_buf[dbg_cmd.dbg_cmd_list_cnt] = func_addr;
        dbg_cmd.dbg_cmd_list_cnt++;
    } else {
        DBG_CMD_PRN("Error----dbg_cmd_list_cnt > Max\r\n");
    }
}

void dbg_cmd_print_msg_en(void)
{
    dbg_cmd.prn_cmd_msg_en = 1;
}

void dbg_cmd_rxd(char rxd)
{
    if ((rxd == 0X0D) || (rxd == 0X0A)) {
        if (dbg_cmd.rxd_cmd_cnt) { /*接收新命令*/
            if (dbg_cmd.cmd_en) {
                DBG_CMD_PRN("\r\n");
            }
            dbg_cmd.rxd_cmd_buf[dbg_cmd.rxd_cmd_cnt++] = '\r';
            dbg_cmd.rxd_cmd_buf[dbg_cmd.rxd_cmd_cnt++] = '\n';
            dbg_cmd.rxd_cmd_buf[dbg_cmd.rxd_cmd_cnt++] = '\0';
            dbg_cmd.rxd_cmd_cnt = 0;
        } else {
            if (dbg_cmd.cmd_en) {
                DBG_CMD_PRN("%s", dbg_cmd.rxd_cmd_buf);
            }
        }
        if (dbg_cmd_analy() == false) {
            if (dbg_cmd.cmd_en) {
                DBG_CMD_PRN("Unknown Cmd\r\n");
            }
        }
        if (dbg_cmd.cmd_en) {
            DBG_CMD_PRN("%002u:%002u:%002u>", dbg_cmd.hours, dbg_cmd.minutes, dbg_cmd.seconds);
        }
    } else if (rxd == 0X08) { // backspace
        if (dbg_cmd.rxd_cmd_cnt) {
            dbg_cmd.rxd_cmd_cnt--;
            if (dbg_cmd.cmd_en) {
                DBG_CMD_PRN("%c", rxd);
            }
        }
    } else {
        dbg_cmd.rxd_cmd_buf[dbg_cmd.rxd_cmd_cnt] = rxd;
        dbg_cmd.rxd_cmd_cnt++;
        if (dbg_cmd.rxd_cmd_cnt >= DBG_CMD_BUF_LEN - 3) {
            dbg_cmd.rxd_cmd_cnt = 0;
            if (dbg_cmd.cmd_en) {
                DBG_CMD_PRN("\r\n%002u:%002u:%002u>", dbg_cmd.hours, dbg_cmd.minutes, dbg_cmd.seconds);
            }
        } else if (dbg_cmd.cmd_en) {
            DBG_CMD_PRN("%c", rxd);
        }
    }
}

void dbg_cmd_1s_thread(void)
{
    dbg_cmd.seconds ++;
    if (dbg_cmd.seconds > 59) {
        dbg_cmd.seconds = 0;
        dbg_cmd.minutes++;
    }
    if (dbg_cmd.minutes > 59) {
        dbg_cmd.minutes = 0;
        dbg_cmd.hours++;
    }
}

void dbg_cmd_init(char dbg_cmd_en)
{
    dbg_cmd.rxd_cmd_cnt = 0;
    dbg_cmd.cmd_en      = dbg_cmd_en;
    if (dbg_cmd.cmd_en) {
        memcpy(dbg_cmd.rxd_cmd_buf, "help\r\n", 7); /* 启动默认命令为打印帮助信息 */
        DBG_CMD_PRN("%s>", "InkerSys");
    }
#ifdef DBG_CMD_MODULE_EN
    dbg_cmd_add_list((int)dbg_cmd_func);
#endif
}
#endif

