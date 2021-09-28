#ifndef __PROJECT_H
#define __PROJECT_H

enum prj_state {
    PRJ_IDLE,
};

struct project_param_s {
    unsigned char   PrjPrf;
    unsigned char   PrjState;
    unsigned char   SubState;
    unsigned char   user_cfg_updata;
};

extern struct project_param_s PrjPkg;

extern void project_init();
extern void project_real_time_thread();

#endif
