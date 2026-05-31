/* close.cc — 文件关闭函数 */

#pragma region include::header
#include "close.hh"
#pragma endregion include::header

#pragma region include::project
#include "filesys.hh"
#pragma endregion include::project

#pragma region include::standard
// exclude <cstdio>
// exclude <cstdint>
#pragma endregion include::standard

void fs_close(uint32_t user_id, uint16_t cfd) {
    struct inode *inode;
    inode = sys_ofile[user[user_id].u_ofile[cfd]].f_inode;
    iput(inode);
    sys_ofile[user[user_id].u_ofile[cfd]].f_count--;
    user[user_id].u_ofile[cfd] = SYSOPENFILE + 1;
}
