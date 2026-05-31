/* delete.cc — 删除文件 */

#pragma region include::header
#include "delete.hh"
#pragma endregion include::header

#pragma region include::project
#include "filesys.hh"
#pragma endregion include::project

#pragma region include::standard
// exclude <cstdint>
// exclude <cstdio>
#pragma endregion include::standard

void fs_delete(char *filename) {
    uint32_t dinodeid;
    struct inode *inode;
    dinodeid = namei(filename);
    if (dinodeid != 0)
        inode = iget(dinodeid);
    inode->di_number--;
    iput(inode);
}
