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

static void write_ptr(uint32_t blk, uint32_t idx, uint64_t ptr) {
    fseek(fd, (int64_t)blk * BSIZ + idx * (int)sizeof(uint64_t), SEEK_SET);
    fwrite(&ptr, 1, sizeof(uint64_t), fd);
}

static uint32_t alloc_pt_block(uint32_t pgd_blk, uint64_t *io_di_addr, uint64_t pml4_idx) {
    uint64_t new_pt = balloc();
    write_ptr(pgd_blk, pml4_idx, new_pt);
    *io_di_addr = MAKE_LA(pml4_idx, 0, 0, 0, 0);
    return new_pt;
}

static uint32_t ensure_data_block(uint32_t pgd_blk, uint64_t *io_di_addr, uint64_t pml4_idx, uint64_t pte_idx) {
    uint64_t la = MAKE_LA(pml4_idx, 0, 0, pte_idx, 0);
    uint32_t pte = read_ptr((uint32_t)(*io_di_addr >> 12), (uint32_t)pte_idx);
    if (pte != 0) return pte;
    uint32_t pdpte = read_ptr((uint32_t)(*io_di_addr >> 21), 0);
    uint32_t pde = read_ptr(pdpte, 0);
    uint32_t new_data = balloc();
    write_ptr(pde, (uint32_t)pte_idx, new_data);
    return new_data;
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

uint32_t fs_alloc_block_for_inode(uint32_t pgd_blk, uint64_t *io_di_addr, uint64_t pml4_idx, uint64_t pte_idx) {
    uint64_t la = MAKE_LA(pml4_idx, 0, 0, pte_idx, 0);
    uint32_t pml4e = read_ptr(pgd_blk, pml4_idx);
    if (pml4e == 0) {
        uint32_t pud = balloc();
        write_ptr(pgd_blk, pml4_idx, pud);
        uint32_t pmd = balloc();
        write_ptr(pud, 0, pmd);
        uint32_t pte = balloc();
        write_ptr(pmd, 0, pte);
        uint32_t data = balloc();
        write_ptr(pte, 0, data);
        *io_di_addr = MAKE_LA(pml4_idx, 0, 0, 0, 0);
        return data;
    }
    uint32_t pdpte = read_ptr(pml4e, 0);
    if (pdpte == 0) {
        uint32_t pmd = balloc();
        write_ptr(pml4e, 0, pmd);
        uint32_t pte = balloc();
        write_ptr(pmd, 0, pte);
        uint32_t data = balloc();
        write_ptr(pte, 0, data);
        *io_di_addr = MAKE_LA(pml4_idx, 0, 0, 0, 0);
        return data;
    }
    uint32_t pde = read_ptr(pdpte, 0);
    if (pde == 0) {
        uint32_t pte = balloc();
        write_ptr(pdpte, 0, pte);
        uint32_t data = balloc();
        write_ptr(pte, 0, data);
        *io_di_addr = MAKE_LA(pml4_idx, 0, 0, 0, 0);
        return data;
    }
    uint32_t pte = read_ptr(pde, (uint32_t)pte_idx);
    if (pte != 0) return pte;
    uint32_t data = balloc();
    write_ptr(pde, (uint32_t)pte_idx, data);
    return data;
}
