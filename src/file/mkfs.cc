// mkfs.c — 创建虚拟磁盘镜像（x86-64四级页表结构，块大小512字节）
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BLOCKSIZ      512
#define DINODEBLK     32
#define FILEBLK       478
#define NICFREE       50
#define NICINOD       50
#define DINODESIZ     32
#define DIRSIZ        14
#define DIRNUM        128
#define PWDSIZ        12
#define PWDNUM        32
#define BSIZ          512
#define BSIZ_LG       9           // log2(512)
#define PTR_PER_BLK   (BSIZ / sizeof(uint64_t))  // = 64

// x86-64逻辑地址各级索引提取
#define PML4_IDX(la)  (((la) >> 39) & 0x1FF)
#define PDPT_IDX(la)  (((la) >> 30) & 0x1FF)
#define PD_IDX(la)    (((la) >> 21) & 0x1FF)
#define PT_IDX(la)    (((la) >> 12) & 0x1FF)
#define BLK_OFF(la)   (((la) >> BSIZ_LG) & 0x7)   // 页内块号(0~7)
#define BYT_OFF(la)   ((la) & (BSIZ - 1))          // 块内字节偏移

// 构造64位逻辑地址
#define MAKE_LA(pml4, pdpt, pd, pte, off) (          \
    ((uint64_t)((pml4) & 0x1FFULL) << 39) |          \
    ((uint64_t)((pdpt)  & 0x1FFULL) << 30) |          \
    ((uint64_t)((pd)    & 0x1FFULL) << 21) |          \
    ((uint64_t)((pte)   & 0x1FFULL) << 12) |          \
    ((uint64_t)((off)   & 0xFFFULL)))

//==============================================================================
// 结构体定义（与 FILESYS.H 完全一致）
//==============================================================================

// inode（内存版）
struct inode {
    struct inode *i_forw;
    struct inode *i_back;
    int8_t  i_flag;
    uint32_t i_ino;
    uint32_t i_count;
    uint16_t di_number;
    uint16_t di_mode;
    uint16_t di_uid;
    uint16_t di_gid;
    uint16_t di_size;
    uint64_t di_addr;  // x86-64标准布局：pml4(9)+pdpt(9)+pd(9)+pte(9)+off(12)
};

// dinode（磁盘版）
struct dinode {
    uint16_t di_number;   // 2
    uint16_t di_mode;     // 2
    uint16_t di_uid;      // 2
    uint16_t di_gid;      // 2
    uint64_t di_size;     // 8
    uint64_t di_addr;     // 8  x86-64逻辑地址（pml4/pdpt/pd/pte/off）
    uint32_t di_ctime;    // 4
    uint32_t di_reserved; // 4
    // 合计 32 字节
};

// 目录项
struct direct {
    int8_t d_name[14];    // UTF-8 自分割，无长度字段
    uint32_t d_ino;       // inode 号（0=空）
};

// 目录块
struct dirblk {
    struct direct entry[DIRNUM];
};

// SuperBlock
struct filsys {
    uint16_t s_isize;
    uint64_t s_fsize;
    uint32_t s_nfree;
    uint16_t s_pfree;
    uint32_t s_free[NICFREE];
    uint32_t s_ninode;
    uint16_t s_pinode;
    uint32_t s_inode[NICINOD];
    uint32_t s_pgd;       // PGD 基址块号（x86-64页表根）
    uint32_t s_rinode;
    int8_t  s_fmod;
};

// 密码项
struct pwd {
    uint16_t p_uid;
    uint16_t p_gid;
    int8_t password[PWDSIZ];
};

//==============================================================================
// 工具函数
//==============================================================================

static int min(int a, int b) {
    return a < b ? a : b;
}

static char pchar(unsigned char c) {
    return (c >= 0x20 && c <= 0x7E) ? c : '.';
}

static void hex_line(const uint8_t *buf, int offset, int len) {
    printf("%08X  ", offset);
    for (int i = 0; i < 16; i++) {
        if (i < len) printf("%02X ", buf[i]);
        else         printf("   ");
        if (i == 7) putchar(' ');
    }
    putchar('|');
    for (int i = 0; i < len; i++) putchar(pchar(buf[i]));
    printf("|\n");
}

static void dump_block(FILE *fp, int blk) {
    uint8_t buf[BLOCKSIZ];
    fseek(fp, (long)blk * BLOCKSIZ, SEEK_SET);
    size_t n = fread(buf, 1, BLOCKSIZ, fp);
    printf("\n=== Block %d (0x%X) ===\n", blk, blk * BLOCKSIZ);
    for (int i = 0; i < BLOCKSIZ; i += 16) {
        int remain = min((int)n - i, 16);
        if (remain <= 0) break;
        hex_line(buf + i, i, remain);
    }
    printf("[%zu / %d bytes]\n", n, BLOCKSIZ);
}

//==============================================================================
// 页表分配
//==============================================================================

static int DATA_START_BLK(void) {
    return 2 + DINODEBLK;    // 数据区区起始块号
}

// 当前分配的块号（全局计数器，在 alloc_blk 中使用）
static int cur_blk = 0;
static int alloc_blk(void) {
    return cur_blk++;
}

// 在指定位置写入64位指针
static void write_ptr(FILE *fp, int blk, int idx, uint64_t ptr) {
    fseek(fp, (long)blk * BSIZ + idx * (int)sizeof(uint64_t), SEEK_SET);
    fwrite(&ptr, 1, sizeof(uint64_t), fp);
}

// 读取64位指针
static uint64_t read_ptr(FILE *fp, int blk, int idx) {
    uint64_t ptr;
    fseek(fp, (long)blk * BSIZ + idx * (int)sizeof(uint64_t), SEEK_SET);
    fread(&ptr, 1, sizeof(uint64_t), fp);
    return ptr;
}

//==============================================================================
// 主程序
//==============================================================================

int main(void) {
    const char *fname = "filesystem";
    FILE *fp;

    fp = fopen(fname, "rb");
    if (fp) {
        fclose(fp);
        remove(fname);
    }

    fp = fopen(fname, "w+b");
    if (!fp) {
        perror("创建镜像失败");
        return 1;
    }

    int total_blocks = 2 + DINODEBLK + FILEBLK;
    printf("镜像总块数: %d, 大小: %d 字节\n", total_blocks, total_blocks * BSIZ);

    // 全零镜像
    uint8_t *zero = (uint8_t*)calloc(1, total_blocks * BSIZ);
    fwrite(zero, 1, total_blocks * BSIZ, fp);
    free(zero);

    //==========================================================================
    // Block 1: SuperBlock（先填充，s_pgd 等 pgd_blk 分配完再填）
    //==========================================================================
    cur_blk = DATA_START_BLK();  // 从数据区开始分配块号

    struct filsys sb;
    memset(&sb, 0, sizeof(sb));
    sb.s_isize  = DINODEBLK;
    sb.s_fsize  = FILEBLK;
    sb.s_ninode = DINODEBLK * BSIZ / DINODESIZ - 4;
    sb.s_nfree  = FILEBLK - 3;
    sb.s_pinode = 0;

    for (int i = 0; i < NICINOD; i++)
        sb.s_inode[i] = 4 + i;
    sb.s_rinode = NICINOD + 4;

    sb.s_free[NICFREE - 1] = FILEBLK + 1;
    for (int i = 0; i < NICFREE - 1; i++)
        sb.s_free[NICFREE - 2 - i] = FILEBLK - i;

    //==========================================================================
    // 为 inode 1（根目录）建立四级页表链
    //==========================================================================
    int pgd_blk  = alloc_blk();  // PML4 基址块（存入 SuperBlock s_pgd）
    int pud_blk  = alloc_blk();   // PDPT
    int pmd_blk  = alloc_blk();   // PD
    int pte_blk  = alloc_blk();   // PT
    int data_blk = alloc_blk();   // 根目录数据块

    // 建立页表链
    write_ptr(fp, pgd_blk, 0, (uint64_t)pud_blk);   // PML4[0] → PDPT
    write_ptr(fp, pud_blk, 0, (uint64_t)pmd_blk);    // PDPT[0] → PD
    write_ptr(fp, pmd_blk, 0, (uint64_t)pte_blk);    // PD[0]   → PT
    write_ptr(fp, pte_blk, 0, (uint64_t)data_blk);   // PT[0]   → 数据块

    printf("PML4=blk%d, PDPT=blk%d, PD=blk%d, PT=blk%d, data=blk%d\n",
           pgd_blk, pud_blk, pmd_blk, pte_blk, data_blk);

    // SuperBlock 的 s_pgd 现在有值了
    sb.s_pgd = pgd_blk;
    fseek(fp, 1 * BSIZ, SEEK_SET);
    fwrite(&sb, 1, sizeof(sb), fp);
    printf("SuperBlock 已写入 Block 1 (s_pgd=blk%d)\n", pgd_blk);

    //==========================================================================
    // inode 区（Block 2~33）
    //==========================================================================
    struct dinode di;

    // inode 0: 保留空
    memset(&di, 0, sizeof(di));
    fseek(fp, 2 * BSIZ + 0 * sizeof(di), SEEK_SET);
    fwrite(&di, 1, sizeof(di), fp);

    // inode 1: 根目录 "/"
    memset(&di, 0, sizeof(di));
    di.di_number = 1;
    di.di_mode   = 0040777 | 0040000; // 0777 | IFDIR
    di.di_uid    = 0;
    di.di_gid    = 0;
    di.di_size   = 3 * (DIRSIZ + 4);
    di.di_addr   = MAKE_LA(0, 0, 0, 0, 0);  // 逻辑地址 0
    fseek(fp, 2 * BSIZ + 1 * sizeof(di), SEEK_SET);
    fwrite(&di, 1, sizeof(di), fp);
    printf("inode 1 (root): PGD=blk%d, 逻辑地址=%016llX\n", pgd_blk, (unsigned long long)di.di_addr);

    //==========================================================================
    // inode 2: /etc 目录（接入共用 PGD PML4[1]）
    //==========================================================================
    int pud2  = alloc_blk();
    int pmd2  = alloc_blk();
    int pte2  = alloc_blk();
    int data2 = alloc_blk();

    write_ptr(fp, pgd_blk, 1, (uint64_t)pud2);   // PML4[1] → PDPT2
    write_ptr(fp, pud2, 0,   (uint64_t)pmd2);
    write_ptr(fp, pmd2, 0,  (uint64_t)pte2);
    write_ptr(fp, pte2, 0,   (uint64_t)data2);

    memset(&di, 0, sizeof(di));
    di.di_number = 1;
    di.di_mode   = 0040777 | 0040000;
    di.di_uid    = 0;
    di.di_gid    = 0;
    di.di_size   = 3 * (DIRSIZ + 4);
    di.di_addr   = MAKE_LA(1, 0, 0, 0, 0);  // pml4=1 → PML4[1]
    fseek(fp, 2 * BSIZ + 2 * sizeof(di), SEEK_SET);
    fwrite(&di, 1, sizeof(di), fp);
    printf("inode 2 (/etc): PML4[1] → PDPT blk%d\n", pud2);

    //==========================================================================
    // inode 3: /etc/password 文件（接入共用 PGD PML4[2]）
    //==========================================================================
    int pud3  = alloc_blk();
    int pmd3  = alloc_blk();
    int pte3  = alloc_blk();
    int data3 = alloc_blk();

    write_ptr(fp, pgd_blk, 2, (uint64_t)pud3);   // PML4[2] → PDPT3
    write_ptr(fp, pud3, 0,   (uint64_t)pmd3);
    write_ptr(fp, pmd3, 0,  (uint64_t)pte3);
    write_ptr(fp, pte3, 0,   (uint64_t)data3);

    memset(&di, 0, sizeof(di));
    di.di_number = 1;
    di.di_mode   = 0100644;
    di.di_uid    = 0;
    di.di_gid    = 0;
    di.di_size   = BSIZ;
    di.di_addr   = MAKE_LA(2, 0, 0, 0, 0);  // pml4=2 → PML4[2]
    fseek(fp, 2 * BSIZ + 3 * sizeof(di), SEEK_SET);
    fwrite(&di, 1, sizeof(di), fp);
    printf("inode 3 (password): PML4[2] → PDPT blk%d\n", pud3);

    printf("inode 区示例数据已写入 Block 2（inode 0~3）\n");

    //==========================================================================
    // 数据区
    //==========================================================================
    // 数据块: inode1 根目录内容
    struct direct root_dir[3];
    memset(root_dir, 0, sizeof(root_dir));
    strcpy((char*)root_dir[0].d_name, ".");
    root_dir[0].d_ino = 1;
    strcpy((char*)root_dir[1].d_name, "..");
    root_dir[1].d_ino = 1;
    strcpy((char*)root_dir[2].d_name, "etc");
    root_dir[2].d_ino = 2;
    fseek(fp, DATA_START_BLK() * BSIZ + (data_blk - DATA_START_BLK()) * BSIZ, SEEK_SET);
    fwrite(root_dir, 1, sizeof(root_dir), fp);
    printf("数据块 %d: 根目录内容已写入\n", data_blk);

    // 数据块: inode2 /etc 目录内容
    struct direct etc_dir[3];
    memset(etc_dir, 0, sizeof(etc_dir));
    strcpy((char*)etc_dir[0].d_name, ".");
    etc_dir[0].d_ino = 2;
    strcpy((char*)etc_dir[1].d_name, "..");
    etc_dir[1].d_ino = 1;
    strcpy((char*)etc_dir[2].d_name, "password");
    etc_dir[2].d_ino = 3;
    fseek(fp, DATA_START_BLK() * BSIZ + (data2 - DATA_START_BLK()) * BSIZ, SEEK_SET);
    fwrite(etc_dir, 1, sizeof(etc_dir), fp);
    printf("数据块 %d: /etc 目录内容已写入\n", data2);

    // 数据块: inode3 password 文件内容
    struct pwd pwdblk[5];
    memset(pwdblk, 0, sizeof(pwdblk));
    pwdblk[0].p_uid = 2116;
    pwdblk[0].p_gid = 3;
    strcpy((char*)pwdblk[0].password, "dddd");
    pwdblk[1].p_uid = 2117;
    pwdblk[1].p_gid = 3;
    strcpy((char*)pwdblk[1].password, "bbbb");
    pwdblk[2].p_uid = 2118;
    pwdblk[2].p_gid = 4;
    strcpy((char*)pwdblk[2].password, "abcd");
    pwdblk[3].p_uid = 2119;
    pwdblk[3].p_gid = 4;
    strcpy((char*)pwdblk[3].password, "cccc");
    pwdblk[4].p_uid = 2220;
    pwdblk[4].p_gid = 5;
    strcpy((char*)pwdblk[4].password, "eeee");
    fseek(fp, DATA_START_BLK() * BSIZ + (data3 - DATA_START_BLK()) * BSIZ, SEEK_SET);
    fwrite(pwdblk, 1, sizeof(pwdblk), fp);
    printf("数据块 %d: password 文件内容已写入\n", data3);

    fclose(fp);
    printf("\n镜像 '%s' 创建完成!\n", fname);
    printf("总块数: %d, 总大小: %d 字节\n", total_blocks, total_blocks * BSIZ);

    //==========================================================================
    // dump 关键块
    //==========================================================================
    fp = fopen(fname, "rb");
    printf("\n========== 镜像概要 ==========\n");
    printf("Block  0 [Boot     ]: 全零\n");
    dump_block(fp, 1);        // SuperBlock
    dump_block(fp, 2);        // inode 区开始
    dump_block(fp, pgd_blk);  // PML4
    dump_block(fp, pud_blk);  // PDPT
    dump_block(fp, pmd_blk);  // PD
    dump_block(fp, pte_blk);  // PT
    dump_block(fp, data_blk); // 数据块
    fclose(fp);

    return 0;
}

