/*igetput.c — x86-64四级页表 inode 读写*/
#include <stdio.h>
#include <stdint.h>
#include "filesys.h"
#include <stdlib.h>

#pragma region iget
struct inode * iget(unsigned int dinodeid) {
    int existed=0, inodeid;
    long addr;
    struct inode *temp, * newinode;
    inodeid=dinodeid % NHINO;
    if (hinode[inodeid].i_forw==NULL) existed = 0;
    else {
        temp = hinode[inodeid].i_forw;
        while (temp) {
            if (temp->i_ino==inodeid) {
                existed = 1;
                temp->i_count ++;
                return temp;
            } else
                temp =temp->i_forw;
        };
    }
    addr = DINODESTART + dinodeid * DINODESIZ;
    newinode = (struct inode * ) malloc (sizeof (struct inode));
    fseek(fd, addr, SEEK_SET);
    fread (&(newinode ->di_number), DINODESIZ, 1, fd);
    newinode->i_forw=hinode[inodeid].i_forw;
    newinode->i_back=newinode;
    newinode->i_forw->i_back=newinode;
    hinode[inodeid].i_forw=newinode;
    newinode->i_count=1;
    newinode->i_flag=0;
    newinode->i_ino=dinodeid;
    return newinode;
}
#pragma endregion

#pragma region iput
void iput(struct inode * pinode) {
    long addr;
    unsigned int block_num;
    int i;
    uint32_t blk;
    if (pinode->i_count>1) {
        pinode->i_count--;
        return;
    } else {
        if (pinode->di_number !=0) {
            addr =DINODESTART + pinode->i_ino * DINODESIZ;
            fseek(fd, addr, SEEK_SET);
            fwrite (&pinode->di_number, DINODESIZ, 1,fd);
        } else {
            /* free file data blocks (iterate all logical blocks) */
            block_num=pinode->di_size/BLOCKSIZ + 1;
            for (i=0; i<(int)block_num; i++) {
                blk = fs_translate(fd, filsys.s_pgd, pinode->di_addr, i);
                if (blk != 0)
                    bfree(blk);
            }
            ifree(pinode->i_ino);
        };

        if (pinode->i_forw==NULL)
            pinode->i_back->i_forw= NULL;
        else {
            pinode->i_forw->i_back=pinode->i_back;
            pinode->i_back->i_forw=pinode->i_forw;
        };
        free (pinode);
    };
}
#pragma endregion

