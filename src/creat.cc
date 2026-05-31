/* creat.cc — 创建文件 */

#pragma region include::header
#include "creat.hh"
#pragma endregion include::header

#pragma region include::project
#include "filesys.hh"
#pragma endregion include::project

#pragma region include::standard
#include <cstdlib>
#include <cstring>
// exclude <cstdint>
// exclude <cstdio>
#pragma endregion include::standard

void fs_creat(uint32_t user_id, char *filename, uint16_t mode) {
    uint32_t di_ith, di_ino;
    struct inode *inode;
    int32_t i, j;
    uint32_t blk;

    di_ino = namei(filename);
    if (di_ino != 0) {
        inode = iget(di_ino);
        if (file_access(user_id, inode, mode) == 0) {
            iput(inode);
            printf("\rcreat access not allowed \n");
            return;
        }
        int32_t nblocks = (int32_t)(inode->di_size / BLOCKSIZ) + 1;
        for (i = 0; i < nblocks; i++) {
            blk = fs_translate(filsys.s_pgd, inode->di_addr, i);
            if (blk != 0)
                bfree(blk);
        }
        for (i = 0; i < SYSOPENFILE; i++)
            if (sys_ofile[i].f_inode == inode)
                sys_ofile[i].f_offset = 0;
        for (i = 0; i < NOFILE; i++)
            if (user[user_id].u_ofile[i] == SYSOPENFILE + 1) {
                user[user_id].u_uid = inode->di_uid;
                user[user_id].u_gid = inode->di_gid;
                for (j = 0; j < SYSOPENFILE; j++)
                    if (sys_ofile[j].f_count == 0) {
                        user[user_id].u_ofile[i] = j;
                        sys_ofile[j].f_flag = mode;
                    }
                return;
            }
    } else {
        inode = ialloc();
        di_ith = iname(filename);
        dir.size++;

        dir.direct[di_ith].d_ino = inode->i_ino;

        inode->di_mode = user[user_id].u_default_mode;
        inode->di_uid = user[user_id].u_uid;
        inode->di_gid = user[user_id].u_gid;
        inode->di_size = 0;
        inode->di_number = 0;
        inode->di_addr = 0;

        for (i = 0; i < SYSOPENFILE; i++)
            if (sys_ofile[i].f_count == 0)
                break;
        for (j = 0; j < NOFILE; j++)
            if (user[user_id].u_ofile[j] == SYSOPENFILE + 1)
                break;
        user[user_id].u_ofile[j] = i;
        sys_ofile[i].f_flag = mode;
        sys_ofile[i].f_count = 0;
        sys_ofile[i].f_offset = 0;
        sys_ofile[i].f_inode = inode;
        return;
    }
}
