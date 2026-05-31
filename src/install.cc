/* install.cc — 加载文件系统 */

#pragma region include::header
#include "install.hh"
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

void fs_install(void) {
    int32_t i, j;
    uint32_t blk;

    fd = fopen("filesystem", "r+b");
    if (fd == NULL) {
        printf("\nfilesys can not be loaded\n");
        exit(0);
    }
    fseek(fd, BLOCKSIZ, SEEK_SET);
    fread(&filsys, 1, sizeof(struct filsys), fd);

    for (i = 0; i < NHINO; i++)
        hinode[i].i_forw = NULL;

    for (i = 0; i < SYSOPENFILE; i++) {
        sys_ofile[i].f_count = 0;
        sys_ofile[i].f_inode = NULL;
    }

    for (i = 0; i < USERNUM; i++) {
        user[i].u_uid = 0;
        user[i].u_gid = 0;
        for (j = 0; j < NOFILE; j++)
            user[i].u_ofile[j] = SYSOPENFILE + 1;
    }

    cur_path_inode = iget(1);
    dir.size = cur_path_inode->di_size / (DIRSIZ + 2);
    for (i = 0; i < DIRNUM; i++) {
        strcpy((char *)dir.direct[i].d_name, "                 ");
        dir.direct[i].d_ino = 0;
    }
    int32_t nfull = dir.size / (BLOCKSIZ / (DIRSIZ + 2));
    int32_t remainder = dir.size % (BLOCKSIZ / (DIRSIZ + 2));
    for (i = 0; i < nfull; i++) {
        blk = fs_translate(filsys.s_pgd, cur_path_inode->di_addr, i);
        fseek(fd, (int64_t)blk * BLOCKSIZ, SEEK_SET);
        fread(&dir.direct[(BLOCKSIZ / (DIRSIZ + 2)) * i], 1, BLOCKSIZ, fd);
    }
    if (remainder != 0) {
        blk = fs_translate(filsys.s_pgd, cur_path_inode->di_addr, nfull);
        fseek(fd, (int64_t)blk * BLOCKSIZ, SEEK_SET);
        fread(&dir.direct[nfull * (BLOCKSIZ / (DIRSIZ + 2))], 1, remainder * (DIRSIZ + 2), fd);
    }
}
