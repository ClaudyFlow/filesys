/*dir.c — 目录操作*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "filesys.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#pragma region _dir
void _dir(void) {
    unsigned int di_mode;
    int i,one;
    struct inode * temp_inode;
    uint32_t blk;
    printf("\nCURRENT DIRECTORY ..\n");
    for (i=0; i<dir.size; i++) {
        if (dir.direct[i].d_ino!=DIEMPTY) {
            printf("%14s",dir.direct[i].d_name);
            temp_inode=iget(dir.direct[i].d_ino);
            di_mode=temp_inode->di_mode;
            for (i=0; i<9; i++) {
                one=di_mode % 2;
                di_mode=di_mode /2;
                if (one) printf ("x");
                else printf ("-");
            }
            if (temp_inode->di_mode && DIFILE  ==1) {
                printf ("%lu\n", (unsigned long)temp_inode->di_size);
                printf("block chain:");
                for(i= 0; i<temp_inode->di_size/BLOCKSIZ+ 1; i++) {
                    blk = fs_translate(fd, filsys.s_pgd, temp_inode->di_addr, i);
                    printf("%4d", blk);
                }
                printf("\n");
            } else printf ("<dir>\n");
            iput (temp_inode);
        }
    }
}
#pragma endregion

#pragma region mkdir
void fs_mkdir(char *dirname) {
    int dirid,dirpos;
    struct inode * inode;
    struct direct buf[BLOCKSIZ/(DIRSIZ+ 2)];
    uint32_t block;

    dirid=namei(dirname);
    if (dirid!=NULL) {
        inode=iget (dirid);
        if(inode->di_mode & DIDIR)
            printf("\n%s directory already existed!! \n", dirname);
        else
            printf("\n%s is a file name, can't create a dir the same name", dirname);
        iput (inode);
        return;
    }

    dirpos = iname(dirname);
    inode=ialloc();
    inode->i_ino=dirid;
    dir.direct[dirpos].d_ino=inode->i_ino;
    dir.size++;
    strcpy (buf[0].d_name,".");
    buf[0]. d_ino=dirid;
    strcpy(buf[1].d_name,"..");
    buf[1].d_ino =cur_path_inode->i_ino;
    block=balloc();
    fseek(fd, (long)block * BLOCKSIZ, SEEK_SET);
    fwrite(buf,1,BLOCKSIZ,fd);
    inode->di_size=2 *(DIRSIZ+2);
    inode->di_number= 1;
    inode->di_mode= user[user_id].u_default_mode;
    inode->di_uid=user[user_id].u_uid;
    inode->di_gid= user[user_id].u_gid;
    /* TODO: 建立 inode->di_addr 的页表链并写入 block */
    inode->di_addr = 0; /* 暂时占位，后续建立页表时更新 */
    iput(inode);
    return;
}
#pragma endregion

#pragma region chdir
void fs_chdir(char * dirname) {
    unsigned int dirid;
    struct inode * inode;
    uint32_t block;
    int i,j,low=0,high=0;
    uint32_t blk;

    dirid =namei(dirname);
    if (dirid  ==NULL) {
        printf("\n%s does not existed\n", dirname);
        return;
    }
    inode=iget(dirid);
    if (!file_access(user_id, inode, user[user_id].u_default_mode)) {
        printf("\nhas not access to the directory %s",dirname);
        iput(inode);
        return;
    }
    /* pack the current directory */
    for (i=0; i<dir.size; i++) {
        for (j=0; j<DIRNUM; j++)
            if (dir.direct[j].d_ino==0) break;
        memcpy(&dir.direct[i], &dir. direct[j], DIRSIZ+2);
        dir.direct[j].d_ino=0;
    }

    /* write back the current directory */
    int nblocks = (int)(cur_path_inode->di_size/BLOCKSIZ) + 1;
    for (i=0; i<nblocks; i++) {
        blk = fs_translate(fd, filsys.s_pgd, cur_path_inode->di_addr, i);
        if (blk != 0)
            bfree(blk);
    }

    for (i=0; i<dir.size; i+=BLOCKSIZ/(DIRSIZ+2)) {
        block = balloc();
        /* TODO: 更新 cur_path_inode->di_addr 页表，写入新 block */
        fseek(fd, (long)block*BLOCKSIZ, SEEK_SET);
        fwrite(&dir.direct[j],1,BLOCKSIZ, fd);
    }
    cur_path_inode->di_size=dir.size*(DIRSIZ+2);
    iput(cur_path_inode);
    cur_path_inode=inode;

    /* read the changed dir from disk */
    j=0;
    nblocks = (int)(inode->di_size/BLOCKSIZ) + 1;
    for (i=0; i<nblocks; i++) {
        blk = fs_translate(fd, filsys.s_pgd, inode->di_addr, i);
        fseek(fd, (long)blk * BLOCKSIZ, SEEK_SET);
        fread(&dir.direct[j],1,BLOCKSIZ,fd);
        j+=BLOCKSIZ/(DIRSIZ+2);
    };

    return;
}
#pragma endregion