/*rdwt.c — x86-64四级页表读写*/
#include <stdio.h>
#include <stdint.h>
#include "filesys.h"

#pragma region read
uint32_t fs_read(unsigned short cfd, unsigned int user_id, char *buf, unsigned int len) {
    unsigned long off;
    int block, block_off, i, j;
    int nblocks;
    struct inode *inode;
    char *temp_buf;
    uint32_t blk;

    inode = sys_ofile[user[user_id].u_ofile[cfd]].f_inode;
    if (!(sys_ofile[user[user_id].u_ofile[cfd]].f_flag & FREAD)) {
        printf("\nthe file is not opened for read\n");
        return 0;
    }
    temp_buf = buf;
    off = sys_ofile[user[user_id].u_ofile[cfd]].f_offset;
    if ((off + len) > inode->di_size) len = inode->di_size - off;
    block_off = (int)(off % BLOCKSIZ);
    block = (int)(off / BLOCKSIZ);
    nblocks = (int)((inode->di_size + BLOCKSIZ - 1) / BLOCKSIZ);

    if (block_off + len < BLOCKSIZ) {
        blk = fs_translate(fd, filsys.s_pgd, inode->di_addr, block);
        fseek(fd, (long)blk * BLOCKSIZ + block_off, SEEK_SET);
        fread(buf, 1, len, fd);
        return len;
    }

    blk = fs_translate(fd, filsys.s_pgd, inode->di_addr, block);
    fseek(fd, (long)blk * BLOCKSIZ + block_off, SEEK_SET);
    fread(temp_buf, 1, BLOCKSIZ - block_off, fd);
    temp_buf += BLOCKSIZ - block_off;
    j = block;
    for (i = 0; i < (len - block_off) / BLOCKSIZ && (j + i) < nblocks; i++) {
        blk = fs_translate(fd, filsys.s_pgd, inode->di_addr, j + i);
        fseek(fd, (long)blk * BLOCKSIZ, SEEK_SET);
        fread(temp_buf, 1, BLOCKSIZ, fd);
        temp_buf += BLOCKSIZ;
    }

    block_off = (len - block_off) % BLOCKSIZ;
    blk = fs_translate(fd, filsys.s_pgd, inode->di_addr, (int)(off / BLOCKSIZ + (len - block_off) / BLOCKSIZ));
    fseek(fd, (long)blk * BLOCKSIZ, SEEK_SET);
    fread(temp_buf, 1, block_off, fd);
    sys_ofile[user[user_id].u_ofile[cfd]].f_offset += len;
    return len;
}
#pragma endregion

#pragma region write
uint32_t fs_write(unsigned short cfd, unsigned int user_id, char *buf, unsigned int len) {
    unsigned long off;
    int block, block_off, i, j;
    int nblocks;
    struct inode *inode;
    char *temp_buf;
    uint32_t blk;

    inode = sys_ofile[user[user_id].u_ofile[cfd]].f_inode;
    if (!(sys_ofile[user[user_id].u_ofile[cfd]].f_flag & FWRITE)) {
        printf("\n the file is not opened for write\n");
        return 0;
    }
    temp_buf = buf;
    off = sys_ofile[user[user_id].u_ofile[cfd]].f_offset;
    block_off = (int)(off % BLOCKSIZ);
    block = (int)(off / BLOCKSIZ);

    if (block_off + len < BLOCKSIZ) {
        blk = fs_translate(fd, filsys.s_pgd, inode->di_addr, block);
        fseek(fd, (long)blk * BLOCKSIZ + block_off, SEEK_SET);
        fwrite(temp_buf, 1, len, fd);
        return len;
    }

    blk = fs_translate(fd, filsys.s_pgd, inode->di_addr, block);
    fseek(fd, (long)blk * BLOCKSIZ + block_off, SEEK_SET);
    fwrite(temp_buf, 1, BLOCKSIZ - block_off, fd);
    temp_buf += BLOCKSIZ - block_off;
    nblocks = block + (int)((len - block_off) / BLOCKSIZ) + 1;

    for (i = 0; i < (len - block_off) / BLOCKSIZ; i++) {
        blk = fs_translate(fd, filsys.s_pgd, inode->di_addr, block + 1 + i);
        if (blk == 0) {
            blk = balloc();
            /* TODO: 写入新分配的块号到页表 */
            fseek(fd, (long)blk * BLOCKSIZ, SEEK_SET);
        } else {
            fseek(fd, (long)blk * BLOCKSIZ, SEEK_SET);
        }
        fwrite(temp_buf, 1, BLOCKSIZ, fd);
        temp_buf += BLOCKSIZ;
    }
    block_off = (len - block_off) % BLOCKSIZ;
    blk = fs_translate(fd, filsys.s_pgd, inode->di_addr, (int)(off / BLOCKSIZ + (len - block_off) / BLOCKSIZ));
    if (blk == 0) {
        blk = balloc();
        fseek(fd, (long)blk * BLOCKSIZ, SEEK_SET);
    } else {
        fseek(fd, (long)blk * BLOCKSIZ, SEEK_SET);
    }
    fwrite(temp_buf, 1, block_off, fd);
    sys_ofile[user[user_id].u_ofile[cfd]].f_offset += len;
    return len;
}
#pragma endregion

