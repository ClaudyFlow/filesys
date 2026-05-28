#ifndef INC_FILE_FILE
#define INC_FILE_FILE

#define WIN32_LEAN_AND_MEAN

#pragma region include::standard
#include <windows.h>
#pragma endregion include::standard

//==============================================================================
// 文件操作接口声明（Win32 API: CreateFile, ReadFile, WriteFile 等）
//==============================================================================

/**
 * @brief 读取文件内容到缓冲区（Win32: ReadFile）
 * @param fd 文件句柄（HANDLE）
 * @param buf 输出缓冲区
 * @param nbyte 期望读取的字节数
 * @param out_read 实际读取的字节数输出（可传 NULL）
 * @return 成功返回 true，失败返回 false
 */
bool file_read(HANDLE fd, void *buf, DWORD nbyte, DWORD *out_read = NULL);

/**
 * @brief 将缓冲区内容写入文件（Win32: WriteFile）
 * @param fd 文件句柄
 * @param buf 数据缓冲区
 * @param nbyte 要写入的字节数
 * @param out_written 实际写入的字节数输出（可传 NULL）
 * @return 成功返回 true，失败返回 false
 */
bool file_write(HANDLE fd, const void *buf, DWORD nbyte, DWORD *out_written = NULL);

/**
 * @brief 打开文件（Win32: CreateFile）
 * @param path 文件路径
 * @param access GENERIC_READ | GENERIC_WRITE
 * @param share_mode FILE_SHARE_READ | FILE_SHARE_WRITE 等
 * @param creation_disposition OPEN_EXISTING | OPEN_ALWAYS | CREATE_ALWAYS 等
 * @param flags FILE_ATTRIBUTE_NORMAL 等
 * @return 成功返回文件句柄，失败返回 INVALID_HANDLE_VALUE
 */
HANDLE file_open(const char *path, DWORD access, DWORD share_mode = 0,
                 DWORD creation_disposition = OPEN_EXISTING,
                 DWORD flags = FILE_ATTRIBUTE_NORMAL);

/**
 * @brief 获取文件大小（Win32: GetFileSizeEx）
 * @param fd 文件句柄
 * @param out_size 输出大小（DWORD64）
 * @return 成功返回 true，失败返回 false
 */
bool file_size(HANDLE fd, DWORD64 *out_size);

/**
 * @brief 创建并打开文件（Win32: CreateFile + CREATE_ALWAYS）
 */
HANDLE file_creat(const char *path, DWORD access = GENERIC_WRITE,
                  DWORD flags = FILE_ATTRIBUTE_NORMAL);

/**
 * @brief 读取整个文件到新分配的内存（Win32: CreateFile + GetFileSize + ReadFile）
 * @param path 文件路径
 * @param out_size 输出参数，返回文件大小（可传 NULL）
 * @return 成功返回 malloc 的指针，失败返回 NULL
 *         注意：调用者需用 free() 释放
 */
void *file_read_all(const char *path, DWORD64 *out_size = NULL);

/**
 * @brief 将数据写入文件（Win32: CreateFile + WriteFile + CloseHandle）
 * @param path 文件路径
 * @param data 数据缓冲区
 * @param size 数据大小
 * @return 成功返回写入的字节数，失败返回 -1
 */
DWORD file_write_all(const char *path, const void *data, DWORD size);

/**
 * @brief 复制文件内容（Win32: CopyFile）
 * @param src 源文件路径
 * @param dst 目标文件路径
 * @param fail_if_exists true=目标存在则失败，false=覆盖
 * @return 成功返回 true，失败返回 false
 */
bool file_copy(const char *src, const char *dst, bool fail_if_exists = true);

/**
 * @brief 删除文件（Win32: DeleteFile）
 * @param path 文件路径
 * @return 成功返回 true，失败返回 false
 */
bool file_unlink(const char *path);

/**
 * @brief 重命名/移动文件（Win32: MoveFile）
 * @param oldpath 原路径
 * @param newpath 新路径
 * @return 成功返回 true，失败返回 false
 */
bool file_rename(const char *oldpath, const char *newpath);

/**
 * @brief 文件指针定位（Win32: SetFilePointerEx）
 * @param fd 文件句柄
 * @param offset 偏移量（LARGE_INTEGER）
 * @param whence FILE_BEGIN | FILE_CURRENT | FILE_END
 * @return 成功返回 true，失败返回 false
 */
bool file_lseek(HANDLE fd, LARGE_INTEGER offset, DWORD whence = FILE_BEGIN,
                LARGE_INTEGER *out_new_pos = NULL);

/**
 * @brief 同步文件内容到磁盘（Win32: FlushFileBuffers）
 * @param fd 文件句柄
 * @return 成功返回 true，失败返回 false
 */
bool file_fsync(HANDLE fd);

#endif /* INC_FILE_FILE */
