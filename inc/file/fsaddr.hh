#ifndef INC_FILE_FSADDR
#define INC_FILE_FSADDR

#pragma region include::standard
#include <cstdint>
#pragma endregion include::standard

//==============================================================================
// x86-64风格四级页表地址翻译（块大小 512 字节）
//
// di_addr 布局（单个 uint64_t，纯 x86-64 逻辑地址）：
//   [63:48] 符号扩展（必须为全0或全1）
//   [47:39] pml4  — PML4  表索引（9位 = 512项）
//   [38:30] pdpt  — PDPT  表索引（9位 = 512项）
//   [29:21] pd    — PD    表索引（9位 = 512项）
//   [20:12] pte   — PTE   表索引（9位 = 512项）
//   [11:0]  off   — 块内字节偏移（12位 = 4096字节页 = 8块×512字节）
//==============================================================================

constexpr uint32_t BSIZ         = 512;       // 块大小（字节）
constexpr uint32_t BSIZ_LG      = 9;         // log2(512) = 块内字节偏移位数
constexpr uint32_t PG_SIZ       = 4096;      // 页大小（字节）
constexpr uint32_t PG_SIZ_LG    = 12;        // log2(4096)
constexpr uint32_t PTR_PER_BLK  = BSIZ / sizeof(uint64_t);  // 每块指针数 = 64

//==============================================================================
// 索引提取宏（从64位逻辑地址提取各级页表索引）
//==============================================================================
constexpr uint64_t PML4_IDX(uint64_t la) {
    return (la >> 39) & 0x1FF;
}
constexpr uint64_t PDPT_IDX(uint64_t la) {
    return (la >> 30) & 0x1FF;
}
constexpr uint64_t PD_IDX(uint64_t la)   {
    return (la >> 21) & 0x1FF;
}
constexpr uint64_t PT_IDX(uint64_t la)   {
    return (la >> 12) & 0x1FF;
}
constexpr uint64_t BLK_OFF_IDX(uint64_t la) {
    return (la >> BSIZ_LG) & 0x7;    // 页内块号(0~7)
}
constexpr uint64_t BYT_OFF(uint64_t la)  {
    return la & (BSIZ - 1);    // 块内字节偏移
}

//==============================================================================
// 构造逻辑地址
//==============================================================================
constexpr uint64_t MAKE_LA(uint64_t pml4, uint64_t pdpt, uint64_t pd, uint64_t pte, uint64_t off) {
    return ((pml4 & 0x1FFULL) << 39) |
           ((pdpt  & 0x1FFULL) << 30) |
           ((pd    & 0x1FFULL) << 21) |
           ((pte   & 0x1FFULL) << 12) |
           (off    & 0xFFFULL);
}

//==============================================================================
// 将 inode di_addr 翻译成物理块号
// @param pgd_blk   PGD 基址块号（来自超级块 s_pgd）
// @param di_addr   inode 的逻辑地址（x86-64 纯逻辑地址）
// @param blk_num   要读取的逻辑块号（从0开始）
// @return 物理块号，失败返回0
//==============================================================================
uint32_t fs_translate(uint32_t pgd_blk, uint64_t di_addr, uint64_t blk_num);
uint32_t fs_alloc_block_for_inode(uint32_t pgd_blk, uint64_t *io_di_addr, uint64_t pml4_idx, uint64_t pte_idx);


#endif /* INC_FILE_FSADDR */
