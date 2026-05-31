#ifndef INC_DEF
#define INC_DEF

#pragma region include::standard
#include <cstdint>
#pragma endregion include::standard

// 文件系统预留空间大小（可按需调整）
constexpr uint32_t FS_BLOCK_SIZE = 4096; // 块大小 4KB
constexpr uint32_t FS_MAX_NAME  = 255; // 文件名最大长度

// 通用错误码
enum FileError {
    OK        = 0,
    ERR_OPEN  = -1,
    ERR_READ  = -2,
    ERR_WRITE = -3,
    ERR_STAT  = -4,
    ERR_PERM  = -5,
};

#endif /* INC_DEF */
