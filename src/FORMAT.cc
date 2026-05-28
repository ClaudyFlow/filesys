/* 格式化虚拟磁盘程序 — format.c */
#include <stdio.h>
#include <stdint.h>
#include "filesys.h"
#include <stdlib.h>
#include <string.h>

void fs_format(void) {
    struct inode *inode;
    struct direct dir_buf[BLOCKSIZ / (DIRSIZ + 2)];
    struct pwd passwd[BLOCKSIZ / (PWDSIZ + 4)];
    struct filsys filsys;
    unsigned int block_buf[BLOCKSIZ / sizeof(int)];
    char *buf;
    int i, j;

    fd = fopen("filesystem", "r+w+b");
    buf = (char *)malloc((DINODEBLK + FILEBLK + 2) * BLOCKSIZ * sizeof(char));
    if (buf == NULL) {
        printf("\nfile system file creat failed! \n");
        exit(0);
    }
    fseek(fd, 0, SEEK_SET);
    fwrite(buf, 1, (DINODEBLK + FILEBLK + 2) * BLOCKSIZ * sizeof(char), fd);

    /* 0. 初始化密码表 */
    passwd[0].p_uid = 2116;
    passwd[0].p_gid = 03;
    strcpy(passwd[0].password, "dddd");
    passwd[1].p_uid = 2117;
    passwd[1].p_gid = 03;
    strcpy(passwd[1].password, "bbbb");
    passwd[2].p_uid = 2118;
    passwd[2].p_gid = 04;
    strcpy(passwd[2].password, "abcd");
    passwd[3].p_uid = 2119;
    passwd[3].p_gid = 04;
    strcpy(passwd[3].password, "cccc");
    passwd[4].p_uid = 2220;
    passwd[4].p_gid = 05;
    strcpy(passwd[4].password, "eeee");
    for (i = 5; i < PWDNUM; i++) {
        passwd[i].p_uid = 0;
        passwd[i].p_gid = 0;
        memset(passwd[i].password, 0, sizeof(passwd[i].password));
    }

    /* 1. 创建主目录、子目录、password 文件 */
    inode = iget(0);
    inode->di_mode = DIEMPTY;
    iput(inode);

    inode = iget(1);
    inode->di_number = 1;
    inode->di_mode = DEFAULTMODE | DIDIR;
    inode->di_size = 3 * (DIRSIZ + 2);
    inode->di_addr = 0ULL << 12;
    strcpy(dir_buf[0].d_name, "..");
    dir_buf[0].d_ino = 1;
    strcpy(dir_buf[1].d_name, ".");
    dir_buf[1].d_ino = 1;
    strcpy(dir_buf[2].d_name, "etc");
    dir_buf[2].d_ino = 2;
    fseek(fd, DATASTART, SEEK_SET);
    fwrite(dir_buf, 1, 3 * (DIRSIZ + 2), fd);
    iput(inode);

    inode = iget(2);
    inode->di_number = 1;
    inode->di_mode = DEFAULTMODE | DIDIR;
    inode->di_size = 3 * (DIRSIZ + 2);
    inode->di_addr = 0ULL << 12;
    strcpy(dir_buf[0].d_name, "..");
    dir_buf[0].d_ino = 1;
    strcpy(dir_buf[1].d_name, ".");
    dir_buf[1].d_ino = 2;
    strcpy(dir_buf[2].d_name, "password");
    dir_buf[2].d_ino = 3;
    fseek(fd, DATASTART + BLOCKSIZ * 1, SEEK_SET);
    fwrite(dir_buf, 1, 3 * (DIRSIZ + 2), fd);
    iput(inode);

    inode = iget(3);
    inode->di_number = 1;
    inode->di_mode = DEFAULTMODE | DIFILE;
    inode->di_size = BLOCKSIZ;
    inode->di_addr = 2ULL << 12;
    fseek(fd, DATASTART + 2 * BLOCKSIZ, SEEK_SET);
    fwrite(passwd, 1, BLOCKSIZ, fd);
    iput(inode);

    /* 2. 初始化超级块 */
    filsys.s_isize = DINODEBLK;
    filsys.s_fsize = FILEBLK;
    filsys.s_ninode = DINODEBLK * BLOCKSIZ / DINODESIZ - 4;
    filsys.s_nfree = FILEBLK - 3;
    for (i = 0; i < NICINOD; i++)
        filsys.s_inode[i] = 4 + i;
    filsys.s_pinode = 0;
    filsys.s_rinode = NICINOD + 4;
    /* FILEBLK+1 是结束标志 */
    block_buf[NICFREE - 1] = FILEBLK + 1;
    for (i = 0; i < NICFREE - 1; i++)
        block_buf[NICFREE - 2 - i] = FILEBLK - i;
    fseek(fd, DATASTART + BLOCKSIZ * (FILEBLK - NICFREE - 1), SEEK_SET);
    fwrite(block_buf, 1, BLOCKSIZ, fd);
    for (i = FILEBLK - NICFREE - 1; i > 2; i -= NICFREE) {
        for (j = 0; j < NICFREE; j++)
            block_buf[j] = i - j;
        fseek(fd, DATASTART + BLOCKSIZ * (i - 1), SEEK_SET);
        fwrite(block_buf, 1, BLOCKSIZ, fd);
    }
    j = 1;
    for (i = i; i > 2; i--)
        filsys.s_free[NICFREE + i - j] = i;
    filsys.s_pfree = NICFREE - j;
    filsys.s_pinode = 0;
    fseek(fd, BLOCKSIZ, SEEK_SET);
    fwrite(&filsys, 1, sizeof(struct filsys), fd);
}
