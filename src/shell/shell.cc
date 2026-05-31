/* shell.cc — 虚拟文件系统命令行工具 */
#pragma region include::header
#include "shell/shell.hh"
#pragma endregion include::header
#pragma region include::project
#include "filesys.hh"
#include "util/nano.hh"
#pragma endregion include::project
#pragma region include::standard
#include <cctype>
#include <cstdlib>
#include <cstring>
// exclude <cstdint>
// exclude <cstdio>
#pragma endregion include::standard

/* ======================= 全局声明 ======================= */
struct hinode hinode[NHINO];
struct dir    dir;
struct file   sys_ofile[SYSOPENFILE];
struct filsys filsys;
struct pwd    pwd[PWDNUM];
struct user   user[USERNUM];
FILE         *fd;
struct inode *cur_path_inode;
int           user_id;

/* ======================= 内部状态 ======================= */
static char line_buf[256];
static char token_buf[16][128];
static int  tok_cnt;

/* ======================= 工具函数 ======================= */
static int split(char *line) {
    char *p = line;
    tok_cnt = 0;
    while (*p && tok_cnt < 16) {
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        if (*p == '"') {
            p++;
            char *out = token_buf[tok_cnt];
            while (*p && *p != '"') *out++ = *p++;
            *out = '\0';
            if (*p == '"') p++;
        } else {
            char *out = token_buf[tok_cnt];
            while (*p && *p != ' ' && *p != '\t' && *p != '\n')
                *out++ = *p++;
            *out = '\0';
        }
        if (token_buf[tok_cnt][0]) tok_cnt++;
    }
    return tok_cnt;
}

static void prompt(void) {
    if (user_id < 0)
        printf("guest@fs:/root> ");
    else
        printf("%u@fs:/root> ", user[user_id].u_uid);
    fflush(stdout);
}

static uint16_t cur_uid(void) {
    if (user_id < 0) return 0;
    return user[user_id].u_uid;
}

/* ======================= 命令实现 ======================= */

static void cmd_login(void) {
    if (tok_cnt < 3) {
        printf("usage: login <uid> <password>\n");
        return;
    }
    unsigned short uid = (uint16_t)atoi(token_buf[1]);
    fs_login(uid, token_buf[2]);
    int i;
    for (i = 0; i < USERNUM; i++)
        if (user[i].u_uid == uid) {
            user_id = i;
            break;
        }
}

static void cmd_logout(void) {
    if (user_id < 0) {
        printf("not logged in\n");
        return;
    }
    fs_logout(user[user_id].u_uid);
    user_id = -1;
}

static void cmd_mkdir(void) {
    if (tok_cnt < 2) {
        printf("usage: mkdir <dir>\n");
        return;
    }
    fs_mkdir(token_buf[1]);
}

static void cmd_ls(void) {
    _dir();
}

static void cmd_cd(void) {
    if (tok_cnt < 2) {
        printf("usage: cd <dir>\n");
        return;
    }
    fs_chdir(token_buf[1]);
}

static void cmd_create(void) {
    if (tok_cnt < 2) {
        printf("usage: create <file> [mode]\n");
        return;
    }
    unsigned short mode = (tok_cnt >= 3)
                          ? (uint16_t)strtol(token_buf[2], NULL, 8)
                          : DEFAULTMODE;
    fs_creat(cur_uid(), token_buf[1], mode);
}

static void cmd_open(void) {
    if (tok_cnt < 2) {
        printf("usage: open <file> [r|w|a]\n");
        return;
    }
    unsigned short omode = FREAD;
    if (tok_cnt >= 3) {
        if (token_buf[2][0] == 'w') omode = FWRITE;
        else if (token_buf[2][0] == 'a') omode = FAPPEND;
    }
    uint16_t cfd = aopen(cur_uid(), token_buf[1], omode);
    if (cfd) printf("fd=%u\n", cfd);
}

static void cmd_read(void) {
    if (tok_cnt < 3) {
        printf("usage: read <fd> <n>\n");
        return;
    }
    unsigned short cfd = (uint16_t)atoi(token_buf[1]);
    uint32_t len = (uint32_t)atoi(token_buf[2]);
    char *buf = (char *)malloc(len + 1);
    unsigned n = fs_read(cfd, user_id, buf, len);
    buf[n] = '\0';
    printf("%s", buf);
    free(buf);
}

static void cmd_write(void) {
    if (tok_cnt < 3) {
        printf("usage: write <fd> <text>\n");
        return;
    }
    unsigned short cfd = (uint16_t)atoi(token_buf[1]);
    fs_write(cfd, user_id, token_buf[2], strlen(token_buf[2]));
}

static void cmd_close(void) {
    if (tok_cnt < 2) {
        printf("usage: close <fd>\n");
        return;
    }
    unsigned short cfd = (uint16_t)atoi(token_buf[1]);
    fs_close(user_id, cfd);
}

static void cmd_delete(void) {
    if (tok_cnt < 2) {
        printf("usage: delete <file>\n");
        return;
    }
    fs_delete(token_buf[1]);
}

static void cmd_cat(void) {
    if (tok_cnt < 2) {
        printf("usage: cat <file>\n");
        return;
    }
    uint16_t cfd = aopen(cur_uid(), token_buf[1], FREAD);
    if (!cfd) return;
    char buf[256];
    unsigned n;
    while ((n = fs_read(cfd, user_id, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        printf("%s", buf);
    }
    fs_close(user_id, cfd);
}

static void cmd_nano(void) {
    if (tok_cnt < 2) {
        printf("usage: nano <file>\n");
        return;
    }
    nano_edit(token_buf[1], cur_uid());
}

/* ======================= 主循环 ======================= */
int main(void) {
    user_id = -1;

    printf("\n=== Virtual File System Shell ===\n");
    printf("Commands: login logout mkdir ls cd create open read write close delete cat nano help\n\n");

    printf("Format the disk first? (y/n): ");
    if (getchar() == 'y') {
        while (getchar() != '\n');
        printf("This will ERASE all data! Sure? (y/n): ");
        if (getchar() == 'y') {
            while (getchar() != '\n');
            fs_format();
            printf("Format done.\n");
        }
    }
    while (getchar() != '\n');

    fs_install();
    printf("\nfilesystem ready.\n\n");

    while (1) {
        prompt();
        if (!fgets(line_buf, sizeof(line_buf), stdin))
            break;
        line_buf[strcspn(line_buf, "\r\n")] = '\0';
        if (!line_buf[0]) continue;

        split(line_buf);
        if (tok_cnt == 0) continue;

        if (strcmp(token_buf[0], "login")   == 0) {
            cmd_login();
        } else if (strcmp(token_buf[0], "logout")  == 0) {
            cmd_logout();
        } else if (strcmp(token_buf[0], "mkdir")   == 0) {
            cmd_mkdir();
        } else if (strcmp(token_buf[0], "ls")      == 0) {
            cmd_ls();
        } else if (strcmp(token_buf[0], "cd")      == 0) {
            cmd_cd();
        } else if (strcmp(token_buf[0], "create")  == 0) {
            cmd_create();
        } else if (strcmp(token_buf[0], "open")    == 0) {
            cmd_open();
        } else if (strcmp(token_buf[0], "read")    == 0) {
            cmd_read();
        } else if (strcmp(token_buf[0], "write")   == 0) {
            cmd_write();
        } else if (strcmp(token_buf[0], "close")   == 0) {
            cmd_close();
        } else if (strcmp(token_buf[0], "delete")  == 0) {
            cmd_delete();
        } else if (strcmp(token_buf[0], "cat")     == 0) {
            cmd_cat();
        } else if (strcmp(token_buf[0], "nano")    == 0) {
            cmd_nano();
        } else if (strcmp(token_buf[0], "help")    == 0) {
            printf("login <uid> <pass>   login\n"
                   "logout               logout\n"
                   "mkdir <dir>          make directory\n"
                   "ls                   list directory\n"
                   "cd <dir>             change directory\n"
                   "create <file> [mode] create file\n"
                   "open <file> [r|w|a]  open file\n"
                   "read <fd> <n>        read n bytes\n"
                   "write <fd> <text>    write text\n"
                   "close <fd>           close fd\n"
                   "delete <file>        delete file\n"
                   "cat <file>           show file\n"
                   "nano <file>          edit file\n"
                   "help                 this help\n");
        } else {
            printf("unknown: %s (try help)\n", token_buf[0]);
        }
    }
    return 0;
}
