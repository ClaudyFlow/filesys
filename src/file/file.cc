
#pragma region include::header
#include <windows.h>
#pragma endregion include::header

#pragma region include::project
#include "utf8.hh"
#pragma endregion include::project


#pragma region include::standard
#include <stdlib.h>
#pragma endregion include::standard

//==============================================================================
// 文件操作实现（Win32 API）
//==============================================================================

bool file_read(HANDLE fd, void *buf, DWORD nbyte, DWORD *out_read) {
    DWORD red = 0;
    BOOL ok = ReadFile(fd, buf, nbyte, &red, NULL);
    if (out_read) *out_read = red;
    return ok && red == nbyte;
}

bool file_write(HANDLE fd, const void *buf, DWORD nbyte, DWORD *out_written) {
    DWORD written = 0;
    BOOL ok = WriteFile(fd, buf, nbyte, &written, NULL);
    if (out_written) *out_written = written;
    return ok && written == nbyte;
}

HANDLE file_open(const char *path, DWORD access, DWORD share_mode,
                 DWORD creation_disposition, DWORD flags) {
    return CreateFileA(path, access, share_mode, NULL,
                       creation_disposition, flags, NULL);
}

bool file_size(HANDLE fd, DWORD64 *out_size) {
    LARGE_INTEGER li;
    if (!GetFileSizeEx(fd, &li)) return false;
    if (out_size) *out_size = li.QuadPart;
    return true;
}

HANDLE file_creat(const char *path, DWORD access, DWORD flags) {
    return CreateFileA(path, access, 0, NULL, CREATE_ALWAYS, flags, NULL);
}

void *file_read_all(const char *path, DWORD64 *out_size) {
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return NULL;

    DWORD64 size = 0;
    GetFileSizeEx(h, (LARGE_INTEGER *)&size);
    if (size == 0) {
        CloseHandle(h);
        if (out_size) *out_size = 0;
        return malloc(1);
    }

    void *buf = malloc((size_t)size + 1);
    if (!buf) {
        CloseHandle(h);
        return NULL;
    }

    DWORD red = 0;
    ReadFile(h, buf, (DWORD)size, &red, NULL);
    ((char *)buf)[red] = '\0';

    if (out_size) *out_size = red;
    CloseHandle(h);
    return buf;
}

DWORD file_write_all(const char *path, const void *data, DWORD size) {
    HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return (DWORD)-1;

    DWORD written = 0;
    WriteFile(h, data, size, &written, NULL);
    CloseHandle(h);
    return written == size ? size : (DWORD)-1;
}

bool file_copy(const char *src, const char *dst, bool fail_if_exists) {
    return CopyFileA(src, dst, fail_if_exists) != 0;
}

bool file_unlink(const char *path) {
    return DeleteFileA(path) != 0;
}

bool file_rename(const char *oldpath, const char *newpath) {
    return MoveFileA(oldpath, newpath) != 0;
}

bool file_lseek(HANDLE fd, LARGE_INTEGER offset, DWORD whence,
                LARGE_INTEGER *out_new_pos) {
    LARGE_INTEGER new_pos;
    if (!SetFilePointerEx(fd, offset, &new_pos, whence)) return false;
    if (out_new_pos) *out_new_pos = new_pos;
    return true;
}

bool file_fsync(HANDLE fd) {
    return FlushFileBuffers(fd) != 0;
}

