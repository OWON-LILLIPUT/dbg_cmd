
/*
 * file encoding utf-8
 *
 * nor_little_fs.c
 *
 * Copyright (C) 2021 Inker.Dong
 *
 * Release V1.1.1 Powered by Inker.Dong
 *
 * Changelog:
 *
 *   2021-08-06 by Inker.Dong
 *   First create.
 *
 */

#include "nor_little_fs.h"

#if ( NOR_LFS_MODULE_EN == 1 )
/************************************************************************/
/*      Print Message                                                   */
/************************************************************************/
#define MODULE_PRINT_INFO_EN // 屏蔽不定义 关闭此模块打印信息
#ifdef MODULE_PRINT_INFO_EN

#include "myprint.h"  /* 其他地方myprint.h不可出现此之前 */
#ifdef PRINTF_INFO_EN
static int prn_level = 0; /* 信息打印等级 */
#endif

#else

#define PRINT(fmt, ...)             ( (void)0 )
#define PRN_ERR(fmt, ...)           ( (void)0 )
#define PRN_HEXS(a,b)               ( (void)0 )
#define PRN_LEVEL(a,b,fmt, ...)     ( (void)0 )

#endif
/************************************************************************/
#include "lfs.h"
#include "lfs_util.h"

#include <stdio.h>
#include <stdlib.h>


lfs_t lfs;
lfs_file_t file;

int user_provided_block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int user_provided_block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int user_provided_block_device_erase(const struct lfs_config *c, lfs_block_t block);
int user_provided_block_device_sync(const struct lfs_config *c);

#define VIRTUAL_SPI_FLASH

// configuration of the filesystem is provided by this struct
const struct lfs_config cfg = {
    // block device operations
    .read  = user_provided_block_device_read,
    .prog  = user_provided_block_device_prog,
    .erase = user_provided_block_device_erase,
    .sync  = user_provided_block_device_sync,

    //github
    //.read_size      = 16,
    //.prog_size      = 16,
    //.block_size     = 4096,
    //.block_count    = 128,
    //.cache_size     = 16,
    //.lookahead_size = 16,
    //.block_cycles   = 500,
#ifdef VIRTUAL_SPI_FLASH
    .read_size      = 0X40,
    .prog_size      = 0X40,
    .cache_size     = 0X40,
    .lookahead_size = 0X40,
    .block_size     = 0X100,
    .block_count    = 8,
    .block_cycles   = 500,
#else
    .read_size      = 256,
    .prog_size      = 256,
    .cache_size     = 256,
    .lookahead_size = 256,
    .block_size     = 4096,
    .block_count    = 4, // (1*1024*1024)/4096
    .block_cycles   = 500,
#endif
};


#ifdef VIRTUAL_SPI_FLASH
char virtual_lfs_flash[0X800];
#endif
int user_provided_block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
#ifdef VIRTUAL_SPI_FLASH
    char *buf;
    buf = buffer;
    for (int i = 0; i < size; i++) {
        *(buf + i) = virtual_lfs_flash[block * cfg.block_size + off + i];
    }
    PRN_LEVEL(prn_level, 3, "read block:%lx off:%X size:%X\r\n", block, off, size);
#else
    LFS_SPI_Flash_Read((char *) buffer, (block * (c->block_size) + off), size);
#endif
    return 0;
}
int user_provided_block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
#ifdef VIRTUAL_SPI_FLASH
    const char *buf = buffer;
    for (int i = 0; i < size; i++) {
        virtual_lfs_flash[block * cfg.block_size + off + i] = *(buf + i);
    }
    PRN_LEVEL(prn_level, 3, "save block:%lx off:%X size:%X\r\n", block, off, size);
#else
    LFS_SPI_Flash_Write_NoCheck((char *)buffer, ((block) * (c->block_size) + off), size);
#endif
    return 0;
}
int user_provided_block_device_erase(const struct lfs_config *c, lfs_block_t block)
{
#ifdef VIRTUAL_SPI_FLASH
    memset(virtual_lfs_flash + block * cfg.block_size, 0XFF, cfg.block_size);
#else
    LFS_SPI_Flash_Erase_Sector(block * (c->block_size));
#endif
    PRN_LEVEL(prn_level, 3, "erase block:%lx\r\n", block);
    return 0;
}
int user_provided_block_device_sync(const struct lfs_config *c)
{
    return 0;
}

/************************************************************************/
/*      static Variables                                                */
/************************************************************************/

struct nor_lfs_state_s {
    unsigned int  fs_mount_state;
    unsigned long total_size;
    unsigned long used_size;
};
struct nor_lfs_state_s nor_lfs_sate;

/************************************************************************/
/*      Local Functions                                                 */
/************************************************************************/
int nor_little_fs_format(void)
{
    int err = lfs_format(&lfs, &cfg);
    PRN_LEVEL(prn_level, 1, "format res:%d\r\n", err);
    return err;
}

int nor_little_fs_mount(void)
{
    int err = lfs_mount(&lfs, &cfg);
    nor_lfs_sate.fs_mount_state = 0;
    if (err == LFS_ERR_OK) {
        nor_lfs_sate.fs_mount_state = 1;
    }
    PRN_LEVEL(prn_level, 1, "mount res:%d\r\n", err);
    return err;
}

void nor_little_fs_unmount(void)
{
    int err = lfs_unmount(&lfs);
    nor_lfs_sate.fs_mount_state = 0;
    PRN_LEVEL(prn_level, 1, "Unmount res:%d\r\n", err);
}

static void nor_little_fs_path_list(char path[])
{
    struct lfs_info info;
    lfs_dir_t dir;
    int res = 0;
    res = lfs_dir_open(&lfs, &dir, path);
    if (res < 0) {
        PRN_LEVEL(prn_level, 1, "lfs_dir_open err%d\n", res);
        return ;
    }
    PRINT("Type      Name  size\r\n");
    while (lfs_dir_read(&lfs, &dir, &info) != 0) {
        if (info.type == 2) { // 2 Dir
            PRINT("Dir  %10s\r\n", info.name);
        } else { // 1 File
            PRINT("File %10s %d\r\n", info.name, info.size);
        }
    }
    res = lfs_dir_close(&lfs, &dir);
}

void nor_little_fs_write(char filename[], char content[])
{
    int res = 0;
    PRN_LEVEL(prn_level, 1, "FileName:%s Content:%s Len:%d\r\n", filename, content, strlen(content));
    //打开文件，如果文件不存在，自动创建
    res = lfs_file_open(&lfs, &file, filename, LFS_O_RDWR | LFS_O_CREAT);
    if (res < 0) {
        PRN_LEVEL(prn_level, 1, "lfs_file_open err%d\n", res);
        return ;
    }
    //写文件
    res = lfs_file_write(&lfs, &file, content, strlen(content));
    if (res < 0) {
        PRN_LEVEL(prn_level, 1, "lfs_file_write err%d\n", res);
        return ;
    }
    lfs_file_close(&lfs, &file);
}

void nor_little_fs_read(char filename[], char mode)
{
    int filelen, res = 0;
    char ReadBuf[64];
    //打开文件，如果文件不存在，自动创建
    res = lfs_file_open(&lfs, &file, filename, LFS_O_RDONLY);
    if (res < 0) {
        PRN_LEVEL(prn_level, 1, "lfs_file_open err%d\n", res);
        return ;
    }
    // 获取文件长度
    filelen = lfs_file_size(&lfs, &file);
    PRN_LEVEL(prn_level, 1, "lfs_file_size:%d\n", filelen);
    if (filelen > 64) {
        filelen = 64;
    }
    res = lfs_file_read(&lfs, &file, ReadBuf, filelen);
    if (res < 0) {
        PRN_LEVEL(prn_level, 1, "lfs_file_read err%d\n", res);
        return ;
    }
    // 关闭文件
    lfs_file_close(&lfs, &file);
    if (mode) {
        PRINT("%s\n", ReadBuf);
    } else {
        PRN_HEXS(ReadBuf, filelen);
    }
}

void nor_little_fs_remove_file(char filename[])
{
    int res = 0;
    res = lfs_remove(&lfs, filename);
    if (res < 0) {
        PRN_LEVEL(prn_level, 1, "lfs_remove err%d\n", res);
    } else {
        PRN_LEVEL(prn_level, 1, "lfs_remove File %s OK!\n", filename);
    }
}

void nor_little_fs_mkdir(char dir[])
{
    int res = 0;
    res = lfs_mkdir(&lfs, dir);
    if (res < 0) {
        PRN_LEVEL(prn_level, 1, "lfs_mkdir err%d\n", res);
        return ;
    }
}

static void nor_little_fs_remove_dir(char path[])
{
    struct lfs_info info;
    lfs_dir_t dir;
    int res = 0;
    char StrBuf[64];
    res = lfs_dir_open(&lfs, &dir, path);
    if (res < 0) {
        PRN_LEVEL(prn_level, 1, "lfs_dir_open err%d\n", res);
        return ;
    }
    while (lfs_dir_read(&lfs, &dir, &info) != 0) {
        if (info.type == 2) { // 2 Dir
            PRN_LEVEL(prn_level, 1, "Dir  %10s\r\n", info.name);
        } else { // 1 File
            PRN_LEVEL(prn_level, 1, "File %10s %d\r\n", info.name, info.size);
            sprintf(StrBuf, "%s/%s", path, info.name);
            res = lfs_remove(&lfs, StrBuf);
            if (res < 0) {
                PRN_LEVEL(prn_level, 1, "lfs_remove err%d\n", res);
            } else {
                PRN_LEVEL(prn_level, 1, "Remove File %s OK!\n", StrBuf);
            }
        }
    }
    lfs_dir_close(&lfs, &dir);
    res = lfs_remove(&lfs, path);
    if (res < 0) {
        PRN_LEVEL(prn_level, 1, "lfs_remove err%d\n", res);
    } else {
        PRN_LEVEL(prn_level, 1, "Remove path %s OK!\n", path);
    }
}

/************************************************************************/
/*     dbg_cmd Interface                                                */
/************************************************************************/
#include "dbg_cmd.h" // 屏蔽关闭此模块命令行调试
#ifdef DBG_CMD_EN
static bool dbg_cmd_func()
{
    struct  nor_lfs_state_s *p_sta = &nor_lfs_sate;
    if (dbg_cmd_exec("help", "", "")) {
        DBG_CMD_PRN(".NorLfs\r\n");
        return false;
    }
    if (dbg_cmd_exec(".exit", "", "")) {
#ifdef MODULE_PRINT_INFO_EN
        prn_level = 0;
#endif
        return false;
    }
    if (dbg_cmd_exec(".NorLfs", "", "")) {
        dbg_cmd_print_msg_en();
    }
    if (dbg_cmd_exec("NorLfsMsg", "", "")) {
#ifdef MODULE_PRINT_INFO_EN
        DBG_CMD_PRN("NorLfsPrf: %d\r\n", prn_level);
#endif
        DBG_CMD_PRN("fs_mount_state:%d\r\n", p_sta->fs_mount_state);
        if (p_sta->fs_mount_state) {
            int total = cfg.block_count;
            p_sta->used_size = lfs_fs_size(&lfs);
            DBG_CMD_PRN("Block total:%d Used:%d\r\n", total, p_sta->used_size);
            DBG_CMD_PRN("total:%dB Used:%dB\r\n", total * cfg.block_size, p_sta->used_size * cfg.block_size);
        }
        return true;
    }
#ifdef MODULE_PRINT_INFO_EN
    if (dbg_cmd_exec("NorLfsPrf", "1", "<0~1> Set Prn Level")) {
        prn_level = get_param_char(0);
        return true;
    }
#endif
    if (dbg_cmd_exec("NorLfsF", "", "Format")) {
        nor_little_fs_format();
        return true;
    }
    if (dbg_cmd_exec("NorLfsM", "", "mount")) {
        nor_little_fs_mount();
        return true;
    }
    if (dbg_cmd_exec("NorLfsUM", "", "unmount")) {
        nor_little_fs_unmount();
        return true;
    }
    if (dbg_cmd_exec("NorLfsW", "ss", "<file name> <content>")) {
        nor_little_fs_write(get_param_string(0), get_param_string(1));
        return true;
    }
    if (dbg_cmd_exec("NorLfsR", "s", "<file name> print str")) {
        nor_little_fs_read(get_param_string(0), 1);
        return true;
    }
    if (dbg_cmd_exec("NorLfsRH", "s", "<file name> print bin")) {
        nor_little_fs_read(get_param_string(0), 0);
        return true;
    }
    if (dbg_cmd_exec("NorLfsLS", "", "")) {
        nor_little_fs_path_list("/");
        return true;
    }
    if (dbg_cmd_exec("NorLfsLSP", "s", "<\"/ or /DirName\">")) {
        nor_little_fs_path_list(get_param_string(0));
        return true;
    }
    if (dbg_cmd_exec("NorLfsRM", "s", "<file name>")) {
        nor_little_fs_remove_file(get_param_string(0));
        return true;
    }
    if (dbg_cmd_exec("NorLfsMK", "s", "<dir name>")) {
        nor_little_fs_mkdir(get_param_string(0));
        return true;
    }
    if (dbg_cmd_exec("NorLfsRMD", "s", "<\"Dir or FileName\"> remove dir")) {
        nor_little_fs_remove_dir(get_param_string(0));
        return true;
    }
    return false;
}
#endif
/************************************************************************/
/*      Application Interface                                           */
/************************************************************************/
void nor_little_fs_init()
{
    if (nor_little_fs_mount() != LFS_ERR_OK) {
        if (nor_little_fs_format() == LFS_ERR_OK) {
            nor_little_fs_mount();
        }
    }
#ifdef DBG_CMD_EN
    dbg_cmd_add_list((CMD_FUNC_T)dbg_cmd_func);
#endif
}
#endif

