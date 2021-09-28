#ifndef __DEMO_H
#define __DEMO_H

//#define _80C51_MCU
#define DEMO_MODULE_EN  (1) // 0不编译模块 1编译模块

#ifdef __cplusplus
extern "C"
{
#endif


#if ( DEMO_MODULE_EN == 1 )

extern void demo_10ms_thread_isr();
extern void demo_100ms_thread();
extern void demo_real_time_thread();
extern void demo_init();

#else

#define demo_10ms_thread_isr()
#define demo_100ms_thread()
#define demo_real_time_thread()
#define demo_init()

#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

