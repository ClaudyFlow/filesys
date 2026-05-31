/* log.cc — 登录登出 */

#pragma region include::header
#include "log.hh"
#pragma endregion include::header

#pragma region include::project
#include "filesys.hh"
#pragma endregion include::project

#pragma region include::standard
#include <cstdlib>
#include <cstring>
// #include <cstdint>
// #include <cstdio>
#pragma endregion include::standard

void fs_login(uint16_t uid, char *passwd) {
    int32_t i, j;
    for (i = 0; i < PWDNUM; i++) {
        if ((uid == pwd[i].p_uid) && !strcmp(passwd, (char *)pwd[i].password)) {
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

void fs_logout(uint16_t uid) {
    int32_t i, j, sys_no;
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
