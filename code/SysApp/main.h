#ifndef __MAIN_H
#define __MAIN_H

//#define _80C51_MCU

struct sys_param_s {
    unsigned char   sys_prn_level;    /* */
    unsigned char   dbg_cmd_en;       /*调试系统    */
    unsigned char   dbg_cmd_pow_on;   /*上电调试状态*/
    unsigned short  product_id;       /*设备编号    */
    unsigned long   eeprom_erase_cnt; /*Flash 擦除累计*/
    unsigned char   eeprom_updata;    /* */
    unsigned char   language;
};

extern struct sys_param_s sys_param;

extern void delay_1ms(int ms_value);
extern void admin_1ms_thread_isr();
extern void uart_rxd1_thread_isr(char rxd);
extern void uart_txd1_thread_isr();

#endif
