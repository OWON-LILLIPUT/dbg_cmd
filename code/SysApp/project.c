
#include "project.h"

/************************************************************************/
/*      Print Message                                                   */
/************************************************************************/
#define MODULE_PRINT_INFO_EN // 屏蔽不定义 关闭此模块打印信息
#ifdef MODULE_PRINT_INFO_EN
//#define PRINT_DETAIL_MODE // module detail print message

#include "myprint.h"  /* 其他地方myprint.h不可出现此之前 */

#ifdef PRINTF_INFO_EN
static int prn_level = 1;/* setting module print level */
#endif

#else

#define PRINT(fmt, ...)             ( (void)0 )
#define PRN_HEXS(a,b)               ( (void)0 )
#define PRN_LEVEL(a,b,fmt, ...)     ( (void)0 )

#endif
/************************************************************************/
struct project_param_s PrjPkg;


/************************************************************************/
/*      Local Prototypes                                                */
/************************************************************************/

/************************************************************************/
/*     dbg_cmd Interface                                                */
/************************************************************************/
#include "dbg_cmd.h" // 屏蔽关闭此模块命令行调试
#ifdef DBG_CMD_EN
static bool dbg_cmd_func()
{
    if (dbg_cmd_exec("help", "", "")) {
        DBG_CMD_PRN(".Prj\r\n");
        return false;
    }
    if (dbg_cmd_exec("exit", "", "")) {
#ifdef MODULE_PRINT_INFO_EN
        prn_level = 0;
#endif
        return false;
    }
    if (dbg_cmd_exec(".prj", "", "Project")) {
        dbg_cmd_print_msg_en();
    }
    if (dbg_cmd_exec("PrjMsg", "", "project msg")) {
#ifdef MODULE_PRINT_INFO_EN
        DBG_CMD_PRN("PrjPrf:%d\r\n", prn_level);
#endif
        DBG_CMD_PRN("PrjState:%d SubState:%d\r\n", PrjPkg.PrjState, PrjPkg.SubState);
        return true;
    }
#ifdef MODULE_PRINT_INFO_EN
    if (dbg_cmd_exec("PrjPrf", "1", "<0~1> project Prn_level")) {
        prn_level = get_param_char(0);
        return true;
    }
#endif
    return false;
}
#endif
/************************************************************************/
/*      Application Interface                                           */
/************************************************************************/
void project_real_time_thread()
{
    if (PrjPkg.user_cfg_updata) {
        PrjPkg.user_cfg_updata = 0;
    }
}

void project_init()
{
#ifdef DBG_CMD_EN
    dbg_cmd_add_list((CMD_FUNC_T)dbg_cmd_func);
#endif
}

