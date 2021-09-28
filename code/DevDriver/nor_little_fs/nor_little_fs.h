#ifndef _NOR_LITTLE_FS_H
#define _NOR_LITTLE_FS_H

#define NOR_LFS_MODULE_EN  (1) // 0不编译模块 1编译模块

#ifdef __cplusplus
extern "C"
{
#endif

#if ( NOR_LFS_MODULE_EN == 1 )

extern void nor_little_fs_init();

#else

#define nor_little_fs_init()

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

