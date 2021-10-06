#ifndef __APP_DEMO_H
#define __APP_DEMO_H

//#define _80C51_MCU
#define APP_DEMO_MODULE_EN  (1) // 0不编译模块 1编译模块

#ifdef __cplusplus
extern "C"
{
#endif


#if ( APP_DEMO_MODULE_EN == 1 )

extern void app_demo_10ms_thread_isr();
extern void app_demo_100ms_thread();
extern void app_demo_real_time_thread();
extern void app_demo_init();

#else

#define app_demo_10ms_thread_isr()
#define app_demo_100ms_thread()
#define app_demo_real_time_thread()
#define app_demo_init()

#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

