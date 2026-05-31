#ifndef INC_UTIL_NANO
#define INC_UTIL_NANO

#pragma region include::standard
#include <cstdint>
#pragma endregion include::standard

#pragma region defines
#define NANO_MAX_LINE  4096
#define NANO_MAX_LINES 65536
#define NANO_MAX_PATH  260
#pragma endregion defines

#pragma region types
struct nano_buf {
    char     *lines[NANO_MAX_LINES];
    uint32_t  count;
    uint32_t  cursor_row;
    uint32_t  cursor_col;
    uint32_t  view_row;
    uint32_t  dirty;
    char      filepath[NANO_MAX_PATH];
};
#pragma endregion types

#pragma region function
bool nano_edit(const char *filepath, uint16_t uid);
#pragma endregion function

#endif /* INC_UTIL_NANO */
