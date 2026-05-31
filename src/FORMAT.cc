/* FORMAT.cc — 格式化虚拟磁盘 */

#pragma region include::header
#include "FORMAT.hh"
#pragma endregion include::header

#pragma region include::project
#include "filesys.hh"
#pragma endregion include::project

#pragma region include::standard
#include <cstdlib>
#include <cstring>
// exclude <cstdio>
// exclude <cstdint>
#pragma endregion include::standard

void fs_format(void) {
    struct inode *inode;
    struct direct dir_buf[BLOCKSIZ / (DIRSIZ + 2)];
    struct pwd passwd[BLOCKSIZ / (PWDSIZ + 4)];
    struct filsys fs;
    uint32_t block_buf[BLOCKSIZ / sizeof(uint32_t)];
    char *buf;
    int32_t i, j;

    fd = fopen("filesystem", "w+b");
    buf = (char *)malloc((DINODEBLK + FILEBLK + 2) * BLOCKSIZ * sizeof(char));
    if (buf == NULL) {
        printf("\nfile system file creat failed!\n");
        exit(0);
    }
    fseek(fd, 0, SEEK_SET);
    fwrite(buf, 1, (DINODEBLK + FILEBLK + 2) * BLOCKSIZ * sizeof(char), fd);

    passwd[0].p_uid = 2116;
    passwd[0].p_gid = 03;
    strcpy((char *)passwd[0].password, "dddd");
    passwd[1].p_uid = 2117;
    passwd[1].p_gid = 03;
    strcpy((char *)passwd[1].password, "bbbb");
    passwd[2].p_uid = 2118;
    passwd[2].p_gid = 04;
    strcpy((char *)passwd[2].password, "abcd");
    passwd[3].p_uid = 2119;
    passwd[3].p_gid = 04;
    strcpy((char *)passwd[3].password, "cccc");
    passwd[4].p_uid = 2220;
    passwd[4].p_gid = 05;
    strcpy((char *)passwd[4].password, "eeee");
    for (i = 5; i < PWDNUM; i++) {
        passwd[i].p_uid = 0;
        passwd[i].p_gid = 0;
        memset(passwd[i].password, 0, sizeof(passwd[i].password));
    }

    uint32_t pgd_blk = balloc();
    uint32_t pud_blk = balloc();
    uint32_t pmd_blk = balloc();
    uint32_t pte_blk = balloc();
    uint32_t data_blk = balloc();

    auto write_ptr = [&](uint32_t blk, uint32_t idx, uint64_t ptr) {
        fseek(fd, (int64_t)blk * BLOCKSIZ + idx * (int)sizeof(uint64_t), SEEK_SET);
        fwrite(&ptr, 1, sizeof(uint64_t), fd);
    };

    write_ptr(pgd_blk, 0, (uint64_t)pud_blk);
    write_ptr(pud_blk, 0, (uint64_t)pmd_blk);
    write_ptr(pmd_blk, 0, (uint64_t)pte_blk);
    write_ptr(pte_blk, 0, (uint64_t)data_blk);

    inode = iget(0);
    inode->di_mode = DIEMPTY;
    iput(inode);

    inode = iget(1);
    inode->di_number = 1;
    inode->di_mode = DEFAULTMODE | DIDIR;
    inode->di_size = 3 * (DIRSIZ + 2);
    inode->di_addr = 0ULL;
    strcpy((char *)dir_buf[0].d_name, ".");
    dir_buf[0].d_ino = 1;
    strcpy((char *)dir_buf[1].d_name, "..");
    dir_buf[1].d_ino = 1;
    strcpy((char *)dir_buf[2].d_name, "etc");
    dir_buf[2].d_ino = 2;
    fseek(fd, (int64_t)data_blk * BLOCKSIZ, SEEK_SET);
    fwrite(dir_buf, 1, 3 * (DIRSIZ + 2), fd);
    iput(inode);

    uint32_t pud2 = balloc();
    uint32_t pmd2 = balloc();
    uint32_t pte2 = balloc();
    uint32_t data2 = balloc();

    write_ptr(pgd_blk, 1, (uint64_t)pud2);
    write_ptr(pud2, 0, (uint64_t)pmd2);
    write_ptr(pmd2, 0, (uint64_t)pte2);
    write_ptr(pte2, 0, (uint64_t)data2);

    inode = iget(2);
    inode->di_number = 1;
    inode->di_mode = DEFAULTMODE | DIDIR;
    inode->di_size = 3 * (DIRSIZ + 2);
    inode->di_addr = 1ULL << 39;
    strcpy((char *)dir_buf[0].d_name, ".");
    dir_buf[0].d_ino = 2;
    strcpy((char *)dir_buf[1].d_name, "..");
    dir_buf[1].d_ino = 1;
    strcpy((char *)dir_buf[2].d_name, "password");
    dir_buf[2].d_ino = 3;
    fseek(fd, (int64_t)data2 * BLOCKSIZ, SEEK_SET);
    fwrite(dir_buf, 1, 3 * (DIRSIZ + 2), fd);
    iput(inode);

    uint32_t pud3 = balloc();
    uint32_t pmd3 = balloc();
    uint32_t pte3 = balloc();
    uint32_t data3 = balloc();

    write_ptr(pgd_blk, 2, (uint64_t)pud3);
    write_ptr(pud3, 0, (uint64_t)pmd3);
    write_ptr(pmd3, 0, (uint64_t)pte3);
    write_ptr(pte3, 0, (uint64_t)data3);

    inode = iget(3);
    inode->di_number = 1;
    inode->di_mode = DEFAULTMODE | DIFILE;
    inode->di_size = BLOCKSIZ;
    inode->di_addr = 2ULL << 39;
    fseek(fd, (int64_t)data3 * BLOCKSIZ, SEEK_SET);
    fwrite(passwd, 1, BLOCKSIZ, fd);
    iput(inode);

    fs.s_isize = DINODEBLK;
    fs.s_fsize = FILEBLK;
    fs.s_ninode = DINODEBLK * BLOCKSIZ / DINODESIZ - 4;
    fs.s_nfree = FILEBLK - 3;
    for (i = 0; i < NICINOD; i++)
        fs.s_inode[i] = 4 + i;
    fs.s_pinode = 0;
    fs.s_rinode = NICINOD + 4;
    fs.s_pgd = pgd_blk;
    block_buf[NICFREE - 1] = FILEBLK + 1;
    for (i = 0; i < NICFREE - 1; i++)
        block_buf[NICFREE - 2 - i] = FILEBLK - i;
    fseek(fd, (int64_t)(DATASTART + BLOCKSIZ * (FILEBLK - NICFREE - 1)), SEEK_SET);
    fwrite(block_buf, 1, BLOCKSIZ, fd);
    for (i = FILEBLK - NICFREE - 1; i > 2; i -= NICFREE) {
        for (j = 0; j < NICFREE; j++)
            block_buf[j] = i - j;
        fseek(fd, (int64_t)(DATASTART + BLOCKSIZ * (i - 1)), SEEK_SET);
        fwrite(block_buf, 1, BLOCKSIZ, fd);
    }
    j = 1;
    for (i = i; i > 2; i--)
        fs.s_free[NICFREE + i - j] = i;
    fs.s_pfree = NICFREE - j;
    fs.s_pinode = 0;
    fseek(fd, BLOCKSIZ, SEEK_SET);
    fwrite(&fs, 1, sizeof(struct filsys), fd);
}
