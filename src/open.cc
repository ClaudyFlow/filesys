/* open.c 鈥� 鎵撳紑鏂囦欢 */
#include <stdio.h>
#include <stdint.h>
#include "filesys.h"
#include <stdlib.h>

uint16_t aopen(unsigned short uid, char *filename, unsigned short openmode) {
    unsigned int dinodeid;
    struct inode *inode;
    int i, j;
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
        printf("\nuser open file too much!!! \n");
        sys_ofile[i].f_count = 0;
        iput(inode);
        return 0;
    }
    user[uid].u_ofile[j] = 1;
    /* 濡傛灉鏄� APPEND 妯″紡锛屽厛閲婃斁鏂囦欢鍘熸湁鎵�鏈夊潡 */
    if (openmode & FAPPEND) {
        int nblocks = (int)(inode->di_size / BLOCKSIZ) + 1;
        for (i = 0; i < nblocks; i++) {
            blk = fs_translate(fd, filsys.s_pgd, inode->di_addr, i);
            if (blk != 0)
                bfree(blk);
        }
        inode->di_size = 0;
    }
    return j;
}
