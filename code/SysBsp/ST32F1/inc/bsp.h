#ifndef _BSP_H
#define _BSP_H

/* RTC */
extern void SetStm32Rtc(unsigned long val);
extern unsigned long  GetStm32Rtc(void);

/* Uart */
extern void uart1_txd_int_en(char en);
extern void uart1_send(char send_data);
extern void uart1_send_wait(char ch);

/* InPut */
extern int  GetKey1(void);
extern int  GetKey2(void);

/* Out Port*/
extern void SetBeepIO(int val);
extern void SetLampIO(int val);

extern void mcu_bsp_init(void);

#endif

