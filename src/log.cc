/* log.c — 登录登出 */
#include <stdio.h>
#include <stdint.h>
#include "filesys.h"
#include <stdlib.h>
#include <string.h>

void fs_login(unsigned short uid, char *passwd) {
    int i, j;
    for (i = 0; i < PWDNUM; i++) {
        if ((uid == pwd[i].p_uid) && (strcmp(passwd, pwd[i].password))) {
            for (j = 0; j < USERNUM; j++)
                if (user[j].u_uid == 0)
                    break;
            if (j == USERNUM) {
                printf("\nToo much user in the System, waited to login\n");
                return;
            } else {
                user[j].u_uid = uid;
                user[j].u_gid = pwd[i].p_gid;
                user[j].u_default_mode = DEFAULTMODE;
                return;
            }
        }
    }
    printf("\n incorrect password\n");
}

void fs_logout(unsigned short uid) {
    int i, j, sys_no;
    struct inode *inode;
    for (i = 0; i < USERNUM; i++)
        if (uid == user[i].u_uid)
            break;
    if (i == USERNUM) {
        printf("\nno such a file\n");
        return;
    }
    for (j = 0; j < NOFILE; j++) {
        if (user[i].u_ofile[j] != SYSOPENFILE + 1) {
            sys_no = user[i].u_ofile[j];
            inode = sys_ofile[sys_no].f_inode;
            iput(inode);
            sys_ofile[sys_no].f_count--;
            user[i].u_ofile[j] = SYSOPENFILE + 1;
        }
    }
}
