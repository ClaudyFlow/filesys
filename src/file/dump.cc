// dump.c — 十六进制查看虚拟磁盘镜像
// 用法: dump <块号> [块数]
//   dump 0     查单个块
//   dump 1 5   从块1开始查5个块
//   dump all   查整个镜像
#pragma region include::standard
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#pragma endregion include::standard

#define BLOCKSIZ 512

// 字节 → 可打印字符，否则 '.'
static char printable(uint8_t c) {
    return (c >= 0x20 && c <= 0x7E) ? c : '.';
}

// 打印一行的 16 字节 hex + ascii
static void hex_line(const uint8_t *buf, int offset, int len) {
    printf("%08X  ", offset);
    for (int i = 0; i < 16; i++) {
        if (i < len)
            printf("%02X ", buf[i]);
        else
            printf("   ");
        if (i == 7) putchar(' ');
    }
    putchar(' |');
    for (int i = 0; i < len; i++)
        putchar(printable(buf[i]));
    putchar('|');
    putchar('\n');
}

// dump 若干块
static void dump_blocks(FILE *fp, int start, int count) {
    uint8_t buf[BLOCKSIZ];
    int blk;
    for (blk = start; blk < start + count; blk++) {
        fseek(fp, (long)blk * BLOCKSIZ, SEEK_SET);
        size_t n = fread(buf, 1, BLOCKSIZ, fp);
        if (n == 0) {
            printf("[Block %d] 读取失败或已到文件末尾\n", blk);
            break;
        }
        printf("\n========== Block %d (0x%X) ==========\n", blk, blk * BLOCKSIZ);
        printf("  偏移到文件头: 0x%X 字节\n\n", blk * BLOCKSIZ);
        int line;
        for (line = 0; line < BLOCKSIZ; line += 16) {
            int remain = (int)n - line;
            if (remain <= 0) break;
            if (remain > 16) remain = 16;
            hex_line(buf + line, line, remain);
        }
        printf("\n  [共 %zu / %d 字节]\n", n, BLOCKSIZ);
    }
}

// 查整个镜像的概要（每块只显示前 16 字节）
static void dump_summary(FILE *fp) {
    uint8_t buf[16];
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    int total_blocks = (int)(fsize / BLOCKSIZ);
    printf("镜像大小: %ld 字节, 总块数: %d\n\n", fsize, total_blocks);
    for (int blk = 0; blk < total_blocks; blk++) {
        fseek(fp, (long)blk * BLOCKSIZ, SEEK_SET);
        size_t n = fread(buf, 1, 16, fp);
        printf("Block %3d [0x%05X]: ", blk, blk * BLOCKSIZ);
        for (size_t i = 0; i < n; i++)
            printf("%02X ", buf[i]);
        for (size_t i = n; i < 16; i++)
            printf("   ");
        printf(" |");
        for (size_t i = 0; i < n; i++)
            putchar(printable(buf[i]));
        printf("|\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("用法:\n");
        printf("  dump <块号> [块数]   查指定块\n");
        printf("  dump all            概要（每块前16字节）\n");
        return 0;
    }
    FILE *fp = fopen("filesystem", "rb");
    if (!fp) {
        fp = fopen("../filesystem", "rb");
    }
    if (!fp) {
        fp = fopen("D:/Code/Project/filesys/filesystem", "rb");
    }
    if (!fp) {
        perror("无法打开 filesystem");
        return 1;
    }
    if (strcmp(argv[1], "all") == 0) {
        dump_summary(fp);
    } else {
        int blk = atoi(argv[1]);
        int count = 1;
        if (argc >= 3)
            count = atoi(argv[2]);
        if (blk < 0 || count <= 0) {
            printf("参数错误\n");
            fclose(fp);
            return 1;
        }
        dump_blocks(fp, blk, count);
    }
    fclose(fp);
    return 0;
}
