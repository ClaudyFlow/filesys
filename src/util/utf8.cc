// utf8 编码/解码实现
#pragma region include::header
#include "util/utf8.hh"
#pragma endregion include::header

#pragma region include::project
#pragma endregion include::project

#pragma region include::standard
#include <cstring>
// exclude <cstdint>
#pragma endregion include::standard

bool utf8_encode(char *buf, uint32_t cp, int *out_len) {
    if (cp < 0x80) {
        buf[0] = (char)cp;
        *out_len = 1;
    } else if (cp < 0x800) {
        buf[0] = (char)(0xC0 | ((cp >> 6) & 0x1F));
        buf[1] = (char)(0x80 | (cp & 0x3F));
        *out_len = 2;
    } else if (cp < 0x10000) {
        buf[0] = (char)(0xE0 | ((cp >> 12) & 0x0F));
        buf[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        buf[2] = (char)(0x80 | (cp & 0x3F));
        *out_len = 3;
    } else {
        buf[0] = (char)(0xF0 | ((cp >> 18) & 0x07));
        buf[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        buf[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        buf[3] = (char)(0x80 | (cp & 0x3F));
        *out_len = 4;
    }
    return true;
}

bool utf8_decode(const char *buf, int start, uint32_t *cp, int *blen) {
    uint8_t c = (uint8_t)buf[start];
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
    return true;
}
