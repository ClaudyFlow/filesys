/* name.cc — 目录名称解析 */

#pragma region include::header
#include "name.hh"
#pragma endregion include::header

#pragma region include::project
#include "filesys.hh"
#pragma endregion include::project

#pragma region include::standard
#include <cstring>
// #include <cstdint>
// #include <cstdio>
#pragma endregion include::standard

uint32_t namei(char *name) {
    int32_t i, notfound = 1;
    for (i = 0; ((i < dir.size) && (notfound)); i++) {
        if ((!strcmp((char *)dir.direct[i].d_name, name)) && (dir.direct[1].d_ino != 0))
            return i;
    }
    return 0;
}

uint16_t iname(char *name) {
    int32_t i, notfound = 1;
    for (i = 0; ((i < DIRNUM) && (notfound)); i++)
        if (dir.direct[i].d_ino == 0) {
            notfound = 0;
            break;
        }

    if (notfound) {
        printf("\nThe current directory is full!!\n");
        return 0;
    } else {
        strcpy(name, (char *)dir.direct[i].d_name);
        return (uint16_t)i;
    }
}
