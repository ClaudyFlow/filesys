/* igetput.cc — inode 读写与缓存 */

#pragma region include::header
#include "igetput.hh"
#pragma endregion include::header

#pragma region include::project
#include "filesys.hh"
#pragma endregion include::project

#pragma region include::standard
#include <cstdlib>
// exclude <cstdio>
// exclude <cstdint>
#pragma endregion include::standard

#pragma region iget
struct inode *iget(uint32_t dinodeid) {
    int32_t existed = 0, inodeid;
    int64_t addr;
    struct inode *temp, *newinode;
    inodeid = dinodeid % NHINO;
    if (hinode[inodeid].i_forw == NULL) existed = 0;
    else {
        temp = hinode[inodeid].i_forw;
        while (temp) {
            if (temp->i_ino == dinodeid) {
                existed = 1;
                temp->i_count++;
                return temp;
            } else
                temp = temp->i_forw;
        };
    }
    addr = DINODESTART + dinodeid * DINODESIZ;
    newinode = (struct inode *)malloc(sizeof(struct inode));
    fseek(fd, addr, SEEK_SET);
    fread(&(newinode->di_number), DINODESIZ, 1, fd);
    newinode->i_forw = hinode[inodeid].i_forw;
    newinode->i_back = newinode;
    if (newinode->i_forw != NULL)
        newinode->i_forw->i_back = newinode;
    hinode[inodeid].i_forw = newinode;
    newinode->i_count = 1;
    newinode->i_flag = 0;
    newinode->i_ino = dinodeid;
    return newinode;
}
#pragma endregion

#pragma region iput
void iput(struct inode *pinode) {
    int64_t addr;
    uint32_t block_num;
    int32_t i;
    uint32_t blk;
    if (pinode->i_count > 1) {
        pinode->i_count--;
        return;
    } else {
        if (pinode->di_number != 0) {
            addr = DINODESTART + pinode->i_ino * DINODESIZ;
            fseek(fd, addr, SEEK_SET);
            fwrite(&pinode->di_number, DINODESIZ, 1, fd);
        } else {
            block_num = pinode->di_size / BLOCKSIZ + 1;
            for (i = 0; i < (int32_t)block_num; i++) {
                blk = fs_translate(filsys.s_pgd, pinode->di_addr, i);
                if (blk != 0)
                    bfree(blk);
            }
            ifree(pinode->i_ino);
        }

        if (pinode->i_forw == NULL)
            pinode->i_back->i_forw = NULL;
        else {
            pinode->i_forw->i_back = pinode->i_back;
            pinode->i_back->i_forw = pinode->i_forw;
        };
        free(pinode);
    };
}
#pragma endregion
