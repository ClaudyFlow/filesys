// fsaddr.cc — x86-64四级页表地址翻译
#include "../inc/file/fsaddr.hh"

// 读指定块的指定条目（每条8字节）
static uint32_t read_ptr(FILE *fp, uint32_t blk, uint32_t idx) {
    uint64_t buf[PTR_PER_BLK];
    fseek(fp, (long)blk * BSIZ, SEEK_SET);
    if (fread(buf, BSIZ, 1, fp) != 1) return 0;
    return (uint32_t)buf[idx];  // 块号从0开始编号
}

uint32_t fs_translate(FILE *fp, uint32_t pgd_blk,
                      uint64_t di_addr, uint64_t blk_num) {
    // 逻辑块号 → 页号 + 页内块索引 + 字节偏移
    uint64_t la = di_addr + blk_num * BSIZ;

    uint32_t pml4e = read_ptr(fp, pgd_blk, PML4_IDX(la));  // PGD基址=PML4基址
    if (pml4e == 0) return 0;

    uint32_t pdpte = read_ptr(fp, pml4e, PDPT_IDX(la));
    if (pdpte == 0) return 0;

    uint32_t pde = read_ptr(fp, pdpte, PD_IDX(la));
    if (pde == 0) return 0;

    uint32_t pte = read_ptr(fp, pde, PT_IDX(la));
    if (pte == 0) return 0;

    return pte + (uint32_t)BYT_OFF(la);  // 数据块号 + 块内偏移
}
