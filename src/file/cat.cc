#pragma region include::header
#include "file/cat.hh"
#pragma endregion include::header

#pragma region include::project
#include "file/dir.hh"
#include "file/file.hh"
#include "util/utf8.hh"
#pragma endregion include::project

#pragma region include::standard
#include <cstdio>
// exclude <cstdint>
// exclude <windows.h>
#pragma endregion include::standard

//==============================================================================
// cat 实现（读取并输出文件内容）
//==============================================================================

bool cat_files(HANDLE fd_out, const char **paths) {
    const char *p = *paths;
    while (p) {
        HANDLE h = CreateFileA(p, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h == INVALID_HANDLE_VALUE) {
            fprintf(stderr, "cat: cannot open '%s'\n", p);
            return false;
        }

        char buf[8192];
        DWORD red;
        while (ReadFile(h, buf, sizeof(buf), &red, NULL) && red > 0) {
            DWORD written = 0;
            WriteFile(fd_out, buf, red, &written, NULL);
            if (written < red) break;  // 输出截断
        }

        CloseHandle(h);
        p = *(++paths);
    }
    return true;
}
