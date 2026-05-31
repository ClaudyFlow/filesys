/* ballfre.cc — 磁盘块分配与释放函数 */

#pragma region include::header
#include "ballfre.hh"
#pragma endregion include::header

#pragma region include::project
#include "filesys.hh"
#pragma endregion include::project

#pragma region include::standard
#include <cstdlib>
// exclude <cstdint>
// exclude <cstdio>
#pragma endregion include::standard

static uint32_t block_buf[BLOCKSIZ];

uint32_t balloc(void) {
    uint32_t free_block, free_block_num;
    uint32_t i;
    if (filsys.s_nfree == 0) {
        printf("\nDisk Full!!!\n");
        return DISKFULL;
    }
    free_block = filsys.s_free[filsys.s_pfree];
    if (filsys.s_pfree >= NICFREE - 1) {
        fseek(fd, (int64_t)free_block * BLOCKSIZ, SEEK_SET);
        fread(block_buf, 1, BLOCKSIZ, fd);
        free_block_num = block_buf[NICFREE];
        for (i = 0; i < free_block_num; i++) {
            filsys.s_free[NICFREE - 1 - i] = block_buf[i];
        }
        filsys.s_pfree = NICFREE - free_block_num;
    } else {
        filsys.s_pfree++;
    }
    filsys.s_nfree--;
    filsys.s_fmod = SUPDATE;
    return free_block;
}

uint32_t bfree(uint32_t block_num) {
    int32_t i;
    if (filsys.s_pfree >= NICFREE - 1) {
        block_buf[0] = block_num;
        for (i = 1; i < NICFREE; i++)
            block_buf[i] = filsys.s_free[NICFREE - i];
        fseek(fd, (int64_t)(DATASTART + BLOCKSIZ * (block_num - 1)), SEEK_SET);
        fwrite(block_buf, 1, BLOCKSIZ, fd);
        filsys.s_pfree = 1;
    } else {
        filsys.s_free[filsys.s_pfree] = block_num;
        filsys.s_pfree++;
    }
    filsys.s_nfree++;
    filsys.s_fmod = SUPDATE;
    return 0;
}
