/*璁块棶鎺у埗鍑芥暟access()(鏂囦欢鍚峚ccess.c)*/

#include <stdio.h>
#include "filesys.h"

uint32_t file_access(unsigned int user_id, struct inode *inode, uint16_t mode) {
    switch (mode) {
    case READ:
        if (inode->di_mode & ODIREAD) return 1;
        if ((inode->di_mode & ODIREAD) &&(user[user_id].u_gid==inode->di_gid)) return 1;
        if ((inode->di_mode & UDIREAD) &&(user[user_id].u_uid==inode->di_uid)) return 1;
        return 0;

    case WRITE:
        if (inode->di_mode & ODIWRITE) return 1;
        if ((inode->di_mode & GDIWRITE) &&(user[user_id].u_gid==inode->di_gid)) return 1;
        if ((inode->di_mode & UDIWRITE) &&
                (user[user_id].u_uid ==inode->di_uid)) return 1;
        return 0;

    case EXECUTE:

        if (inode->di_mode & ODIEXECUTE) return 1;
        if ((inode->di_mode & GDIEXECUTE) &&
                (user[user_id].u_gid== inode ->di_gid)) return 1;
        if ((inode->di_mode & ODIEXECUTE) &&
                (user[user_id].u_uid==inode ->di_uid)) return 1;
        return 0;
    default:
        return 0;
    }
}