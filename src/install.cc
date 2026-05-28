/*install.c — 加载文件系统*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <cstdlib>
#include "filesys.h"

void fs_install(void) {
    int i,j;
    uint32_t blk;

    fd = fopen("filesystem", "w+r+b");
    if(fd==NULL) {
        printf("\nfilesys can not be loaded\n");
        exit(0);
    }
    /* read filsys from superblock */
    fseek(fd, BLOCKSIZ, SEEK_SET);
    fread(&filsys, 1, sizeof(struct filsys), fd);

    /* init mode hash chain */
    for(i=0; i<NHINO; i++)
        hinode[i].i_forw=NULL;

    /* init sys-ofile */
    for(i=0; i<SYSOPENFILE; i++) {
        sys_ofile[i].f_count=0;
        sys_ofile[i].f_inode=NULL;
    }

    /* init user */
    for(i=0; i<USERNUM; i++) {
        user[i].u_uid=0;
        user[i].u_gid=0;
        for(j=0; j<NOFILE; j++)
            user[i].u_ofile[j]=SYSOPENFILE+ 1;
    }

    /* read main directory to init dir */
    cur_path_inode=iget(1);
    dir.size=cur_path_inode->di_size/(DIRSIZ+2);
    for(i=0; i<DIRNUM; i++) {
        strcpy(dir.direct[i].d_name,"                 ");
        dir.direct[i].d_ino=0;
    }
    for(i=0; i<dir.size/(BLOCKSIZ/(DIRSIZ+2)); i++) {
        blk = fs_translate(fd, filsys.s_pgd, cur_path_inode->di_addr, i);
        fseek(fd, (long)blk * BLOCKSIZ, SEEK_SET);
        fread(&dir.direct[(BLOCKSIZ/(DIRSIZ+2)) * i], 1, BLOCKSIZ, fd);
    }
    blk = fs_translate(fd, filsys.s_pgd, cur_path_inode->di_addr, i);
    fseek(fd, (long)blk * BLOCKSIZ, SEEK_SET);
    fread(&dir.direct[(BLOCKSIZ)/(DIRSIZ+2) * i], 1, cur_path_inode->di_size % BLOCKSIZ, fd);
}
