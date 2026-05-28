#pragma region include::header
#include "file/dir.hh"
#pragma endregion include::header

#pragma region include::project
#include "util/utf8.hh"
#pragma endregion include::project

#pragma region include::standard
#include <cstdio>
#pragma endregion include::standard

//==============================================================================
// 目录操作实现（Win32 API）
//==============================================================================

bool dir_mkdir(const char *path, SECURITY_ATTRIBUTES *sa) {
    return CreateDirectoryA(path, sa) != 0;
}

bool dir_rmdir(const char *path) {
    return RemoveDirectoryA(path) != 0;
}

bool dir_chdir(const char *path) {
    return SetCurrentDirectoryA(path) != 0;
}

char *dir_getcwd(char *buf, DWORD size) {
    DWORD ret = GetCurrentDirectoryA(size, buf);
    if (ret == 0 || ret > size) return NULL;
    return buf;
}

int dir_list(const char *path) {
    // 若未指定 path，取当前目录
    char dir_path[MAX_PATH];
    if (path == NULL) {
        GetCurrentDirectoryA(MAX_PATH, dir_path);
    } else {
        strncpy(dir_path, path, MAX_PATH);
    }

    // 拼通配符路径
    char pattern[MAX_PATH];
    snprintf(pattern, MAX_PATH, "%s\\*", dir_path);

    WIN32_FIND_DATA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return -1;

    int count = 0;
    do {
        // 跳过 "." 和 ".."
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;

        // 打印类型标识
        const char *type = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? "[DIR] " : "[FILE]";
        printf("%-12s %s\n", type, fd.cFileName);
        count++;
    } while (FindNextFileA(h, &fd));

    FindClose(h);
    return count;
}

int dir_foreach(const char *path,
                bool (*callback)(const char *name, const WIN32_FIND_DATA *fd)) {
    char dir_path[MAX_PATH];
    if (path == NULL) {
        GetCurrentDirectoryA(MAX_PATH, dir_path);
    } else {
        strncpy(dir_path, path, MAX_PATH);
    }

    char pattern[MAX_PATH];
    snprintf(pattern, MAX_PATH, "%s\\*", dir_path);

    WIN32_FIND_DATA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return -1;

    int count = 0;
    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;
        if (!callback(fd.cFileName, &fd))
            break;
        count++;
    } while (FindNextFileA(h, &fd));

    FindClose(h);
    return count;
}

bool dir_stat(const char *path, WIN32_FIND_DATA *fd) {
    HANDLE h = FindFirstFileA(path, fd);
    if (h == INVALID_HANDLE_VALUE) return false;
    FindClose(h);
    return true;
}

