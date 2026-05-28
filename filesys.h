#ifndef FILESYS_H
#define FILESYS_H

/* Windows 兼容层 */
#ifdef _WIN32
    #include <io.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #define close _close
    #define read  _read
    #define write _write
    #define unlink _unlink
#else
    #include <unistd.h>
#endif

/* 项目头文件 */
#include "inc/FILESYS.hh"

#endif /* FILESYS_H */
