/* open.cc — 打开文件 */

#pragma region include::header
#include "open.hh"
#pragma endregion include::header

#pragma region include::project
#include "filesys.hh"
#pragma endregion include::project

#pragma region include::standard
#include <cstdlib>
// #include <cstdint>
// #include <cstdio>
#pragma endregion include::standard

uint16_t aopen(uint16_t uid, char *filename, uint16_t openmode) {
    uint32_t dinodeid;
    struct inode *inode;
    int32_t i, j;
    uint32_t blk;

    dinodeid = namei(filename);
    if (dinodeid == 0) {
        printf("\nfile does not existed!!\n");
        return 0;
    }
    inode = iget(dinodeid);
    if (!file_access(uid, inode, openmode)) {
        printf("\nfile open has not access!!!");
        iput(inode);
        return 0;
    }
    for (i = 1; i < SYSOPENFILE; i++)
        if (sys_ofile[i].f_count == 0)
            break;
    if (i == SYSOPENFILE) {
        printf("\nsystem open file too much\n");
        iput(inode);
        return 0;
    }
    sys_ofile[i].f_inode = inode;
    sys_ofile[i].f_flag = openmode;
    sys_ofile[i].f_count = 1;
    if (openmode & FAPPEND)
        sys_ofile[i].f_offset = inode->di_size;
    else
        sys_ofile[i].f_offset = 0;
    for (j = 0; j < NOFILE; j++)
        if (user[uid].u_ofile[j] == 0)
            break;
    if (j == NOFILE) {
        printf("\nuser open file too much!!!\n");
        sys_ofile[i].f_count = 0;
        iput(inode);
        return 0;
    }
    user[uid].u_ofile[j] = 1;
    if (openmode & FAPPEND) {
        int32_t nblocks = (int32_t)(inode->di_size / BLOCKSIZ) + 1;
        for (i = 0; i < nblocks; i++) {
            blk = fs_translate(filsys.s_pgd, inode->di_addr, i);
            if (blk != 0)
                bfree(blk);
        }
        inode->di_size = 0;
    }
    return (uint16_t)j;
}
