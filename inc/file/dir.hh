#ifndef INC_FILE_DIR
#define INC_FILE_DIR

#define WIN32_LEAN_AND_MEAN

#pragma region include::standard
#include <windows.h>
#pragma endregion include::standard

//==============================================================================
// 目录操作接口声明（Win32 API: FindFirstFile, FindNextFile, CreateDirectory 等）
//==============================================================================

/**
 * @brief 创建目录（Win32: CreateDirectory）
 * @param path 目录路径（相对或绝对）
 * @param sa 安全属性（传 NULL 使用默认）
 * @return 成功返回 true，失败返回 false
 */
bool dir_mkdir(const char *path, SECURITY_ATTRIBUTES *sa = NULL);

/**
 * @brief 删除空目录（Win32: RemoveDirectory）
 * @param path 目录路径
 * @return 成功返回 true，失败返回 false
 */
bool dir_rmdir(const char *path);

/**
 * @brief 切换当前工作目录（Win32: SetCurrentDirectory）
 * @param path 目标目录路径
 * @return 成功返回 true，失败返回 false
 */
bool dir_chdir(const char *path);

/**
 * @brief 获取当前工作目录路径（Win32: GetCurrentDirectory）
 * @param buf 存储路径的缓冲区
 * @param size 缓冲区大小（字符数）
 * @return 成功返回 buf，失败返回 NULL
 */
char *dir_getcwd(char *buf, DWORD size);

/**
 * @brief 列出目录内容（Win32: FindFirstFile + FindNextFile）
 * @param path 目录路径（NULL 表示当前目录）
 * @return 成功返回条目数，失败返回 -1
 */
int dir_list(const char *path = NULL);

/**
 * @brief 遍历目录条目（Win32: FindFirstFile + FindNextFile）
 * @param path 目录路径
 * @param callback 每条目回调: (const char *name, const WIN32_FIND_DATA *fd)->bool
 *                 返回 false 停止遍历
 * @return 遍历的条目数，失败返回 -1
 */
int dir_foreach(const char *path,
                bool (*callback)(const char *name, const WIN32_FIND_DATA *fd));

/**
 * @brief 判断路径是否为目录（Win32: GetFileAttributes）
 */
inline bool dir_is_dir(const char *path) {
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

/**
 * @brief 判断路径是否为常规文件
 */
inline bool dir_is_file(const char *path) {
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

/**
 * @brief 获取文件/目录元信息（Win32: FindFirstFile）
 * @param path 路径
 * @param fd 输出缓冲区（WIN32_FIND_DATA）
 * @return 成功返回 true，失败返回 false
 */
bool dir_stat(const char *path, WIN32_FIND_DATA *fd);

#endif /* INC_FILE_DIR */
