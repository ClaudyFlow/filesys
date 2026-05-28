#ifndef INC_FILE_CAT
#define INC_FILE_CAT

#define WIN32_LEAN_AND_MEAN

#pragma region include::standard
#include <windows.h>
#pragma endregion include::standard

/**
 * @brief 连接打印文件内容到指定句柄（类似 Unix cat）
 * @param fd_out 输出文件句柄（GetStdHandle(STD_OUTPUT_HANDLE) 为标准输出）
 * @param paths 文件路径列表（NULL 结尾）
 * @return 成功返回 true，任何文件失败返回 false
 */
bool cat_files(HANDLE fd_out, const char **paths);

/**
 * @brief 单文件打印到标准输出
 */
inline bool cat(const char *path) {
    const char *p = path;
    return cat_files(GetStdHandle(STD_OUTPUT_HANDLE), &p);
}

#endif /* INC_FILE_CAT */
