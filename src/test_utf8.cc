// utf8 测试：输入中文 -> 存到 direct 结构 -> 读出来输出
#include <windows.h>
#include <stdio.h>
#include <string.h>

// 给定字符串和起始位置，读一个 UTF-8 字符，返回码点和字符字节数
static void utf8_decode(const char *buf, int start, unsigned int *cp, int *blen) {
    unsigned char c = (unsigned char)buf[start];
    int len;
    if (c < 0x80) {
        len = 1;
    } else if ((c & 0xE0) == 0xC0) {
        len = 2;
    } else if ((c & 0xF0) == 0xE0) {
        len = 3;
    } else {
        len = 4;
    }
    *blen = len;
    if (len == 1) {
        *cp = c;
    } else if (len == 2) {
        *cp = (c & 0x1F) << 6;
        *cp |= (buf[start + 1] & 0x3F);
    } else if (len == 3) {
        *cp = (c & 0x0F) << 12;
        *cp |= (buf[start + 1] & 0x3F) << 6;
        *cp |= (buf[start + 2] & 0x3F);
    } else {
        *cp = (c & 0x07) << 18;
        *cp |= (buf[start + 1] & 0x3F) << 12;
        *cp |= (buf[start + 2] & 0x3F) << 6;
        *cp |= (buf[start + 3] & 0x3F);
    }
}

// 模拟 direct 结构
struct direct {
    char d_name[255];
    unsigned int d_ino;
};

int main() {
    direct entry = {0};

    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    strcpy(entry.d_name, "先帝创业未半而中道崩殂，今天下三分，益州疲弊，此诚危急存亡之秋也。然侍卫之臣不懈于内，忠志之士忘身于外者，盖追先帝之殊遇，欲报之于陛下也。诚宜开张圣听，以光先帝遗德，恢弘志士之气，不宜妄自菲薄，引喻失义，以塞忠谏之路也。");
    entry.d_ino = 1;

    printf("hex + unicode:\n");
    int pos = 0;
    while (entry.d_name[pos]) {
        int start = pos;
        unsigned int cp;
        int blen;
        utf8_decode(entry.d_name, start, &cp, &blen);
        // 输出字节
        printf("  bytes: ");
        for (int i = 0; i < blen; i++) {
            printf("%02X ", (unsigned char)entry.d_name[start + i]);
        }
        printf("  U+%04X\n", cp);
        pos += blen;
    }
    printf("\n");

    // 原始字节
    printf("raw bytes: %s\n", entry.d_name);

    return 0;
}
