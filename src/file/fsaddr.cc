// fsaddr.cc — x86-64四级页表地址翻译
#pragma region include::header
#include "file/fsaddr.hh"
#pragma endregion include::header

#pragma region include::project
#include "filesys.hh"
#pragma endregion include::project

#pragma region include::standard
// exclude <cstdint>
// exclude <cstdio>
#pragma endregion include::standard

// 读指定块的指定条目（每条8字节）
static uint32_t read_ptr(uint32_t blk, uint32_t idx) {
    uint64_t buf[PTR_PER_BLK];
    fseek(fd, (int64_t)blk * BSIZ, SEEK_SET);
    if (fread(buf, BSIZ, 1, fd) != 1) return 0;
    return (uint32_t)buf[idx];
}

uint32_t fs_translate(uint32_t pgd_blk, uint64_t di_addr, uint64_t blk_num) {
    uint64_t la = di_addr + blk_num * BSIZ;

    uint32_t pml4e = read_ptr(pgd_blk, PML4_IDX(la));
    if (pml4e == 0) return 0;

    uint32_t pdpte = read_ptr(pml4e, PDPT_IDX(la));
    if (pdpte == 0) return 0;

    uint32_t pde = read_ptr(pdpte, PD_IDX(la));
    if (pde == 0) return 0;

    uint32_t pte = read_ptr(pde, PT_IDX(la));
    if (pte == 0) return 0;

    return pte + (uint32_t)BYT_OFF(la);
}
