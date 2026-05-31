/* iallfre.cc — inode 分配与释放 */

#pragma region include::header
#include "iallfre.hh"
#pragma endregion include::header

#pragma region include::project
#include "filesys.hh"
#pragma endregion include::project

#pragma region include::standard
#include <cstdlib>
// exclude <cstdint>
// exclude <cstdio>
#pragma endregion include::standard

static struct dinode block_buf[BLOCKSIZ / DINODESIZ];

struct inode *ialloc(void) {
    struct inode *temp_inode;
    uint32_t cur_di;
    int32_t i, count, block_end_flag;
    if (filsys.s_pinode == NICINOD) {
        i = 0;
        count = 0;
        block_end_flag = 1;
        filsys.s_pinode = NICINOD - 1;
        cur_di = filsys.s_rinode;
        while ((count < NICINOD) || (count <= filsys.s_ninode)) {
            if (block_end_flag) {
                fseek(fd, DINODESTART + cur_di * DINODESIZ, SEEK_SET);
                fread(block_buf, 1, BLOCKSIZ, fd);
                block_end_flag = 0;
                i = 0;
            }
            while (block_buf[i].di_mode == DIEMPTY) {
                cur_di++;
                i++;
            }
            if (i == NICINOD)
                block_end_flag = 1;
            else {
                filsys.s_inode[filsys.s_pinode--] = cur_di;
                count++;
            }
        }
        filsys.s_rinode = cur_di;
    }
    temp_inode = iget(filsys.s_inode[filsys.s_pinode]);
    fseek(fd, DINODESTART + filsys.s_inode[filsys.s_pinode] * DINODESIZ, SEEK_SET);
    fwrite(&temp_inode->di_number, 1, sizeof(struct dinode), fd);
    filsys.s_pinode++;
    filsys.s_ninode--;
    filsys.s_fmod = SUPDATE;
    return temp_inode;
}

void ifree(uint32_t dinodeid) {
    filsys.s_ninode++;
    if (filsys.s_pinode != NICINOD) {
        filsys.s_inode[filsys.s_pinode] = dinodeid;
        filsys.s_pinode++;
    } else {
        if (dinodeid < filsys.s_rinode) {
            filsys.s_inode[NICINOD] = dinodeid;
            filsys.s_rinode = dinodeid;
        }
    }
}
