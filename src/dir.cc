/* dir.cc — 目录操作 */

#pragma region include::header
#include "dir.hh"
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

#pragma region _dir
void _dir(void) {
    uint32_t di_mode;
    int32_t i, j, one;
    struct inode *temp_inode;
    uint32_t blk;
    printf("\nCURRENT DIRECTORY ..\n");
    for (i = 0; i < dir.size; i++) {
        if (dir.direct[i].d_ino != DIEMPTY) {
            printf("%14s", dir.direct[i].d_name);
            temp_inode = iget(dir.direct[i].d_ino);
            di_mode = temp_inode->di_mode;
            for (j = 0; j < 9; j++) {
                one = di_mode % 2;
                di_mode = di_mode / 2;
                if (one) printf("x");
                else printf("-");
            }
            if (temp_inode->di_mode && DIFILE == 1) {
                printf("%I64u\n", (uint64_t)temp_inode->di_size);
                printf("block chain:");
                for (i = 0; i < temp_inode->di_size / BLOCKSIZ + 1; i++) {
                    blk = fs_translate(filsys.s_pgd, temp_inode->di_addr, i);
                    printf("%4d", blk);
                }
                printf("\n");
            } else printf("<dir>\n");
            iput(temp_inode);
        }
    }
}
#pragma endregion

#pragma region mkdir
void fs_mkdir(char *dirname) {
    int32_t dirid, dirpos;
    struct inode *inode;
    struct direct buf[BLOCKSIZ / (DIRSIZ + 2)];
    uint32_t block;

    dirid = namei(dirname);
    if (dirid != 0) {
        inode = iget(dirid);
        if (inode->di_mode & DIDIR)
            printf("\n%s directory already existed!!\n", dirname);
        else
            printf("\n%s is a file name, can't create a dir the same name", dirname);
        iput(inode);
        return;
    }

    dirpos = iname(dirname);
    inode = ialloc();
    inode->i_ino = dirid;
    dir.direct[dirpos].d_ino = inode->i_ino;
    dir.size++;
    strcpy(buf[0].d_name, ".");
    buf[0].d_ino = dirid;
    strcpy(buf[1].d_name, "..");
    buf[1].d_ino = cur_path_inode->i_ino;
    block = balloc();
    fseek(fd, (int64_t)block * BLOCKSIZ, SEEK_SET);
    fwrite(buf, 1, BLOCKSIZ, fd);
    inode->di_size = 2 * (DIRSIZ + 2);
    inode->di_number = 1;
    inode->di_mode = user[user_id].u_default_mode;
    inode->di_uid = user[user_id].u_uid;
    inode->di_gid = user[user_id].u_gid;
    inode->di_addr = 0;
    iput(inode);
    return;
}
#pragma endregion

#pragma region chdir
void fs_chdir(char *dirname) {
    uint32_t dirid;
    struct inode *inode;
    uint32_t block;
    int32_t i, j, low = 0, high = 0;
    uint32_t blk;

    dirid = namei(dirname);
    if (dirid == 0) {
        printf("\n%s does not existed\n", dirname);
        return;
    }
    inode = iget(dirid);
    if (!file_access(user_id, inode, user[user_id].u_default_mode)) {
        printf("\nhas not access to the directory %s", dirname);
        iput(inode);
        return;
    }
    for (i = 0; i < dir.size; i++) {
        for (j = 0; j < DIRNUM; j++)
            if (dir.direct[j].d_ino == 0) break;
        memcpy(&dir.direct[i], &dir.direct[j], DIRSIZ + 2);
        dir.direct[j].d_ino = 0;
    }

    int32_t nblocks = (int32_t)(cur_path_inode->di_size / BLOCKSIZ) + 1;
    for (i = 0; i < nblocks; i++) {
        blk = fs_translate(filsys.s_pgd, cur_path_inode->di_addr, i);
        if (blk != 0)
            bfree(blk);
    }

    for (i = 0; i < dir.size; i += BLOCKSIZ / (DIRSIZ + 2)) {
        block = balloc();
        fseek(fd, (int64_t)block * BLOCKSIZ, SEEK_SET);
        fwrite(&dir.direct[i], 1, BLOCKSIZ, fd);
    }
    cur_path_inode->di_size = dir.size * (DIRSIZ + 2);
    iput(cur_path_inode);
    cur_path_inode = inode;

    j = 0;
    nblocks = (int32_t)(inode->di_size / BLOCKSIZ) + 1;
    for (i = 0; i < nblocks; i++) {
        blk = fs_translate(filsys.s_pgd, inode->di_addr, i);
        fseek(fd, (int64_t)blk * BLOCKSIZ, SEEK_SET);
        fread(&dir.direct[j], 1, BLOCKSIZ, fd);
        j += BLOCKSIZ / (DIRSIZ + 2);
    };

    return;
}
#pragma endregion
