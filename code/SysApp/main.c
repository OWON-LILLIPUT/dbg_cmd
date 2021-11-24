
/*
 * file encoding utf-8
 *
 */

#define MODULE_PRINT_INFO_EN // 屏蔽不定义 关闭此模块打印信息

#include "bsp.h"
#include "main.h"
#include "app_demo.h"
#include "nor_little_fs.h"
#include "project.h"
#include "myprint.h"

#include <stdbool.h>

#ifdef PRINTF_INFO_EN
static int prn_level = 0; /* 信息打印等级 */
#endif

struct sys_param_s sys_param;

/*
 * uart queue
 */
#include "queue.h"
#define TXD_RXD_BUF_LEN  128
struct queue_s rxd1_queue, txd1_queue;
static void queue_init()
{
    static char txd1_buf[TXD_RXD_BUF_LEN];
    static char rxd1_buf[TXD_RXD_BUF_LEN];
    rxd1_queue.queue_buf         = rxd1_buf;
    rxd1_queue.queue_max_element = TXD_RXD_BUF_LEN;
    init_queue(&rxd1_queue);
    txd1_queue.queue_buf         = txd1_buf;
    txd1_queue.queue_max_element = TXD_RXD_BUF_LEN;
    init_queue(&txd1_queue);
}

void uart_rxd1_thread_isr(char rxd)
{
    in_queue(&rxd1_queue, rxd);
}

void uart_txd1_thread_isr()
{
    char txd;
    if (out_queue(&txd1_queue, &txd) != QUEUE_EMPTY) {
        uart1_send(txd);
    } else {
        uart1_txd_int_en(0);
    }
}

void put_txd1_queue(char txd)
{
    // 串口发送完成中断 当空闲状态 需要这里发送第一个数据 才能触发发送完成中断

    // 串口发送空闲中断
    while (in_queue(&txd1_queue, txd) == QUEUE_FULL);
    uart1_txd_int_en(1);

    // 串口查询发送空闲状态
    // uart1_send_wait(txd);
}

/*
 * dbg_cmd
 */
#include "dbg_cmd.h"
#ifdef DBG_CMD_EN
static bool dbg_cmd_func()
{
    if (dbg_cmd_exec("help", "", "")) {
        DBG_CMD_PRN(".System\r\n");
        return false;
    }
    if (dbg_cmd_exec("exit", "", "")) {
#ifdef MODULE_PRINT_INFO_EN
        prn_level = 0;
#endif
        DBG_CMD_PRN("System Exit\r\n");
        return false;
    }
    if (dbg_cmd_exec(".system", "", "")) {
        dbg_cmd_print_msg_en();
    }
    if (dbg_cmd_exec("sysmsg", "", "")) {
#ifdef MODULE_PRINT_INFO_EN
        DBG_CMD_PRN("SysPrint   :%d\r\n",      prn_level);
#endif
        return true;
    }
#ifdef MODULE_PRINT_INFO_EN
    if (dbg_cmd_exec("SysPrint", "1", "<0~1>")) {
        prn_level = get_param_char(0);
        return true;
    }
#endif
    return false;
}
#endif

/*
 * System Timer Tick
 */
volatile char tim_cnt_10ms = 0;	/*Unit: mS*/
static volatile int delay_1ms_cnt = 0;
void delay_1ms(int ms_value)
{
    delay_1ms_cnt = ms_value;
    while (delay_1ms_cnt);
}

void admin_1ms_thread_isr()
{
    static char tim_cnt_1ms = 0;
    if (delay_1ms_cnt) {
        delay_1ms_cnt--;
    }
    tim_cnt_1ms++;
    switch (tim_cnt_1ms) {
        case 1:
            tim_cnt_10ms++;
            break;
        case 2:
            app_demo_10ms_thread_isr();
            break;
        case 10:
            tim_cnt_1ms = 0;
            break;
    }
}

static void admin_1s_thread()
{
    static int cnt_1s = 0;

    cnt_1s++;
    PRN_LEVEL(prn_level, 1, "system print 1s cnt:%d\r\n",cnt_1s);
#ifdef DBG_CMD_EN
    dbg_cmd_1s_thread();
#endif
}

static void admin_100ms_thread()  //后台100mS进程
{
    static char tim_cnt_100ms = 0;
    tim_cnt_100ms++;
    switch (tim_cnt_100ms) {
        case 10:
            tim_cnt_100ms = 0;
            admin_1s_thread();
            break;
    }
    app_demo_100ms_thread();
}

int main()
{
    char rxd;
    // 初始化串口队列
    queue_init();
    my_print_init((PUT_FUNC_T)put_txd1_queue);
    mcu_bsp_init();
#ifdef DBG_CMD_EN
    dbg_cmd_init(1);// 上电默认启动命令行
    dbg_cmd_add_list((CMD_FUNC_T)dbg_cmd_func);
#endif
    app_demo_init();
    nor_little_fs_init();
    project_init();
    while (1) {
        project_real_time_thread();
        app_demo_real_time_thread();
        if (tim_cnt_10ms >= 10) {
            tim_cnt_10ms -= 10;
            admin_100ms_thread();
        }
        while (out_queue(&rxd1_queue, &rxd) != QUEUE_EMPTY) {
#ifdef DBG_CMD_EN
            dbg_cmd_rxd(rxd);
#endif
        }
    }
    //return 0;
}

