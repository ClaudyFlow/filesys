#pragma region include::project
#include "filesys.hh"
#pragma endregion include::project
#pragma region include::standard
#include <conio.h>
#include <cstdlib>
#include <cstring>
#include <windows.h>
// #include <cstdint>
// #include <cstdio>
#pragma endregion include::standard

#define FS_VERSION "1.0.0"
#define FS_BUILD_DATE __DATE__

struct hinode hinode[NHINO];
struct dir dir;
struct file sys_ofile[SYSOPENFILE];
struct filsys filsys;
struct pwd pwd[PWDNUM];
struct user user[USERNUM];
FILE *fd;
struct inode *cur_path_inode;
int user_id = -1;
static int logged_in_user_idx = -1;

#pragma region boot_animation
void print_banner(void) {
    printf("\n");
    printf("  ==================================================\n");
    printf("  ||                                              ||\n");
    printf("  ||      N A C H O S   F I L E S Y S T E M      ||\n");
    printf("  ||                                              ||\n");
    printf("  ||        Version %s                    ||\n", FS_VERSION);
    printf("  ||        Build: %s        ||\n", FS_BUILD_DATE);
    printf("  ||                                              ||\n");
    printf("  ==================================================\n");
    printf("\n");
}

void show_help_document(void) {
    printf("\n");
    printf("  ==================================================\n");
    printf("  ||           NACHOS HELP DOCUMENT              ||\n");
    printf("  ==================================================\n");
    printf("\n");
    printf("  [System Info]\n");
    printf("  Version : %s\n", FS_VERSION);
    printf("  Build   : %s\n", FS_BUILD_DATE);
    printf("  Arch    : x86-64 Page Table (4-level)\n");
    printf("  Block   : %d bytes\n", BLOCKSIZ);
    printf("  Disk    : %I64u blocks\n\n", (uint64_t)filsys.s_fsize);

    printf("  [Test Users]\n");
    printf("  ----------------------------------------\n");
    printf("  UID  | GID | Password\n");
    printf("  2116 | 03  | dddd\n");
    printf("  2117 | 03  | bbbb\n");
    printf("  2118 | 04  | abcd\n");
    printf("  2119 | 04  | cccc\n");
    printf("  2220 | 05  | eeee\n");
    printf("  ----------------------------------------\n\n");

    printf("  [Disk Layout]\n");
    printf("  Block 0       : Boot (reserved)\n");
    printf("  Block 1       : SuperBlock\n");
    printf("  Block 2~33    : inode area (32 blocks)\n");
    printf("  Block 34~     : Data area (512 blocks)\n\n");

    printf("  [Quick Start]\n");
    printf("  1. login <uid> <password>\n");
    printf("  2. mkdir <name>\n");
    printf("  3. create <name>\n");
    printf("  4. open <name> <mode>\n");
    printf("  5. write <fd> <text>\n");
    printf("  6. read <fd>\n\n");

    printf("  [Mode Values]\n");
    printf("  1 = READ, 2 = WRITE, 4 = APPEND\n\n");

    printf("  ==================================================\n");
    printf("  Press ENTER to continue...\n");
    getchar();
}

void loading_bar(int width) {
    int i;
    printf("  [");
    for (i = 0; i < width; i++) {
        printf("=");
    }
    printf("]");
}

void boot_animation(void) {
    int i, j;
    const char *stages[] = {
        "Initializing filesystem...",
        "Loading superblock...",
        "Building inode hash table...",
        "Initializing file descriptor table...",
        "Loading user tables...",
        "Mounting root directory...",
        "Loading password database...",
        "Setting up page tables...",
        "System ready!"
    };
    int num_stages = sizeof(stages) / sizeof(stages[0]);
    int bar_width = 30;

    print_banner();
    printf("  Booting system, please wait...\n\n");

    for (i = 0; i < num_stages; i++) {
        printf("  %-40s ", stages[i]);
        fflush(stdout);

        loading_bar(0);
        for (j = 0; j <= bar_width; j++) {
            printf("\b\b");
            loading_bar(j);
            fflush(stdout);
            Sleep(30);
        }
        printf(" [OK]\n");
    }

    printf("\n");
    printf("  ==================================================\n");
    printf("  ||         SYSTEM BOOT COMPLETE                 ||\n");
    printf("  ==================================================\n");
    printf("\n");
    printf("  Type 'help' for available commands.\n");
    printf("  Press 'H' to view full help document.\n\n");
}
#pragma endregion

#pragma region command_parser
int parse_command(char *input, char *cmd, char *arg1, char *arg2) {
    char *token;
    int argc = 0;
    cmd[0] = '\0';
    arg1[0] = '\0';
    arg2[0] = '\0';

    token = strtok(input, " \t\n\r");
    if (token == NULL) return 0;
    strcpy(cmd, token);
    argc++;

    token = strtok(NULL, " \t\n\r");
    if (token != NULL) {
        strcpy(arg1, token);
        argc++;
    }

    token = strtok(NULL, " \t\n\r");
    if (token != NULL) {
        strcpy(arg2, token);
        argc++;
    }

    return argc;
}
#pragma endregion

#pragma region command_handlers
void cmd_help(void) {
    printf("\n  Available commands:\n");
    printf("  ------------------------------------------------\n");
    printf("  login <uid> <password>   Login to system\n");
    printf("  logout                   Logout from system\n");
    printf("  dir                      List current directory\n");
    printf("  mkdir <name>             Create directory\n");
    printf("  chdir <name>             Change directory\n");
    printf("  create <name> [mode]     Create file (default 01777)\n");
    printf("  open <name> <mode>       Open file (1=read,2=write,4=append)\n");
    printf("  read <fd>                Read file by descriptor\n");
    printf("  write <fd> <text>        Write to file by descriptor\n");
    printf("  close <fd>               Close file by descriptor\n");
    printf("  delete <name>            Delete file\n");
    printf("  format                   Format disk (DANGER!)\n");
    printf("  user                     Show current user info\n");
    printf("  pwd                      Show current path\n");
    printf("  help                     Show this help\n");
    printf("  exit                     Shutdown and exit\n");
    printf("  ------------------------------------------------\n");
    printf("\n  Press 'H' for full help document.\n\n");
}

void cmd_user(void) {
    if (logged_in_user_idx < 0 || user[logged_in_user_idx].u_uid == 0) {
        printf("\n  [INFO] No user logged in.\n\n");
        return;
    }
    printf("\n  Current User:\n");
    printf("  ------------------------------------------------\n");
    printf("  UID    : %u\n", user[logged_in_user_idx].u_uid);
    printf("  GID    : %u\n", user[logged_in_user_idx].u_gid);
    printf("  Mode   : %04o\n", user[logged_in_user_idx].u_default_mode);
    printf("  ------------------------------------------------\n\n");
}

void cmd_format(void) {
    char confirm;
    printf("\n  [WARNING] This will ERASE all data on the disk!\n");
    printf("  Are you sure? (y/N): ");
    confirm = getch();
    printf("%c\n", confirm);
    if (confirm == 'y' || confirm == 'Y') {
        printf("\n  Formatting disk, please wait...\n");
        fs_format();
        printf("  [OK] Disk formatted successfully.\n\n");
    } else {
        printf("  [CANCELLED] Format aborted.\n\n");
    }
}

void cmd_login(uint16_t uid, char *passwd) {
    if (logged_in_user_idx >= 0 && user[logged_in_user_idx].u_uid != 0) {
        printf("\n  [INFO] User %u already logged in. Logout first.\n\n",
               user[logged_in_user_idx].u_uid);
        return;
    }
    fs_login(uid, passwd);
    int i;
    for (i = 0; i < USERNUM; i++) {
        if (user[i].u_uid == uid) {
            logged_in_user_idx = i;
            user_id = i;
            printf("\n  [OK] User %u logged in successfully.\n\n", uid);
            return;
        }
    }
}

void cmd_logout(void) {
    if (logged_in_user_idx < 0 || user[logged_in_user_idx].u_uid == 0) {
        printf("\n  [INFO] No user logged in.\n\n");
        return;
    }
    uint16_t uid = user[logged_in_user_idx].u_uid;
    fs_logout(uid);
    user[logged_in_user_idx].u_uid = 0;
    user[logged_in_user_idx].u_gid = 0;
    logged_in_user_idx = -1;
    user_id = -1;
    printf("\n  [OK] User logged out successfully.\n\n");
}

void cmd_mkdir(char *dirname) {
    if (logged_in_user_idx < 0) {
        printf("\n  [ERROR] Please login first.\n\n");
        return;
    }
    if (strlen(dirname) == 0) {
        printf("\n  [ERROR] Directory name required.\n\n");
        return;
    }
    fs_mkdir(dirname);
    printf("\n  [OK] Directory '%s' created.\n\n", dirname);
}

void cmd_chdir(char *dirname) {
    if (logged_in_user_idx < 0) {
        printf("\n  [ERROR] Please login first.\n\n");
        return;
    }
    if (strlen(dirname) == 0) {
        printf("\n  [ERROR] Directory name required.\n\n");
        return;
    }
    fs_chdir(dirname);
}

void cmd_create(char *filename, uint16_t mode) {
    if (logged_in_user_idx < 0) {
        printf("\n  [ERROR] Please login first.\n\n");
        return;
    }
    if (strlen(filename) == 0) {
        printf("\n  [ERROR] File name required.\n\n");
        return;
    }
    if (mode == 0) mode = DEFAULTMODE;
    fs_creat(logged_in_user_idx, filename, mode);
    printf("\n  [OK] File '%s' created with mode %04o.\n\n", filename, mode);
}

void cmd_open(char *filename, uint16_t openmode) {
    if (logged_in_user_idx < 0) {
        printf("\n  [ERROR] Please login first.\n\n");
        return;
    }
    if (strlen(filename) == 0) {
        printf("\n  [ERROR] File name required.\n\n");
        return;
    }
    uint16_t cfd = aopen(logged_in_user_idx, filename, openmode);
    if (cfd != 0) {
        printf("\n  [OK] File '%s' opened, fd=%u\n\n", filename, cfd);
    }
}

void cmd_read(uint16_t cfd) {
    if (logged_in_user_idx < 0) {
        printf("\n  [ERROR] Please login first.\n\n");
        return;
    }
    char buf[BLOCKSIZ + 1];
    uint32_t len = fs_read(cfd, logged_in_user_idx, buf, BLOCKSIZ);
    if (len > 0) {
        buf[len] = '\0';
        printf("\n  --- Content (%u bytes) ---\n", len);
        printf("%s", buf);
        printf("\n  --- End ---\n\n");
    } else {
        printf("\n  [ERROR] Read failed or empty file.\n\n");
    }
}

void cmd_write(uint16_t cfd, char *text) {
    if (logged_in_user_idx < 0) {
        printf("\n  [ERROR] Please login first.\n\n");
        return;
    }
    uint32_t len = strlen(text);
    uint32_t written = fs_write(cfd, logged_in_user_idx, text, len);
    printf("\n  [OK] Wrote %u bytes.\n\n", written);
}

void cmd_close(uint16_t cfd) {
    if (logged_in_user_idx < 0) {
        printf("\n  [ERROR] Please login first.\n\n");
        return;
    }
    fs_close(logged_in_user_idx, cfd);
    printf("\n  [OK] File closed (fd=%u).\n\n", cfd);
}

void cmd_delete(char *filename) {
    if (logged_in_user_idx < 0) {
        printf("\n  [ERROR] Please login first.\n\n");
        return;
    }
    if (strlen(filename) == 0) {
        printf("\n  [ERROR] File name required.\n\n");
        return;
    }
    fs_delete(filename);
    printf("\n  [OK] File '%s' deleted.\n\n", filename);
}

void cmd_pwd(void) {
    printf("\n  Current path: /\n\n");
}
#pragma endregion

#pragma region main_loop
void main_loop(void) {
    char input[256];
    char cmd[64], arg1[128], arg2[128];

    while (1) {
        if (logged_in_user_idx >= 0 && user[logged_in_user_idx].u_uid != 0) {
            printf("NACHOS:%u$ ", user[logged_in_user_idx].u_uid);
        } else {
            printf("NACHOS:guest$ ");
        }
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        int argc = parse_command(input, cmd, arg1, arg2);
        if (argc == 0) continue;

        if (strcmp(cmd, "help") == 0) {
            cmd_help();
        } else if (strcmp(cmd, "H") == 0) {
            show_help_document();
        } else if (strcmp(cmd, "user") == 0) {
            cmd_user();
        } else if (strcmp(cmd, "pwd") == 0) {
            cmd_pwd();
        } else if (strcmp(cmd, "format") == 0) {
            cmd_format();
        } else if (strcmp(cmd, "login") == 0) {
            if (argc < 3) {
                printf("\n  [ERROR] Usage: login <uid> <password>\n\n");
            } else {
                uint16_t uid = (uint16_t)atoi(arg1);
                cmd_login(uid, arg2);
            }
        } else if (strcmp(cmd, "logout") == 0) {
            cmd_logout();
        } else if (strcmp(cmd, "dir") == 0) {
            if (logged_in_user_idx < 0) {
                printf("\n  [ERROR] Please login first.\n\n");
            } else {
                _dir();
            }
        } else if (strcmp(cmd, "mkdir") == 0) {
            cmd_mkdir(arg1);
        } else if (strcmp(cmd, "chdir") == 0) {
            cmd_chdir(arg1);
        } else if (strcmp(cmd, "create") == 0) {
            uint16_t mode = (argc >= 3) ? (uint16_t)strtol(arg2, NULL, 8) : DEFAULTMODE;
            cmd_create(arg1, mode);
        } else if (strcmp(cmd, "open") == 0) {
            uint16_t openmode = (argc >= 3) ? (uint16_t)atoi(arg2) : READ;
            cmd_open(arg1, openmode);
        } else if (strcmp(cmd, "read") == 0) {
            if (argc < 2) {
                printf("\n  [ERROR] Usage: read <fd>\n\n");
            } else {
                cmd_read((uint16_t)atoi(arg1));
            }
        } else if (strcmp(cmd, "write") == 0) {
            if (argc < 3) {
                printf("\n  [ERROR] Usage: write <fd> <text>\n\n");
            } else {
                cmd_write((uint16_t)atoi(arg1), arg2);
            }
        } else if (strcmp(cmd, "close") == 0) {
            if (argc < 2) {
                printf("\n  [ERROR] Usage: close <fd>\n\n");
            } else {
                cmd_close((uint16_t)atoi(arg1));
            }
        } else if (strcmp(cmd, "delete") == 0) {
            cmd_delete(arg1);
        } else if (strcmp(cmd, "exit") == 0) {
            if (logged_in_user_idx >= 0) {
                cmd_logout();
            }
            printf("\n  Good Bye! See You Next Time.\n\n");
            break;
        } else {
            printf("\n  [ERROR] Unknown command: '%s'. Type 'help' for available commands.\n\n", cmd);
        }
    }
}
#pragma endregion

#pragma region main
int main(void) {
    char choice;

    print_banner();

    if (fopen("filesystem", "rb") == NULL) {
        printf("  [INFO] No filesystem found, creating new one...\n");
        fs_format();
        printf("  [OK] Filesystem created.\n\n");
    } else {
        printf("  Do you want to [f]ormat the disk or [l]oad existing? (f/l): ");
        choice = getch();
        printf("%c\n", choice);

        if (choice == 'f' || choice == 'F') {
            printf("\n  [WARNING] Format will erase ALL data!\n");
            printf("  Are you sure? (y/N): ");
            choice = getch();
            printf("%c\n", choice);
            if (choice == 'y' || choice == 'Y') {
                fs_format();
            } else {
                printf("\n  [CANCELLED] Format aborted.\n");
            }
        }
    }

    printf("\n  Loading filesystem...\n");
    fs_install();
    printf("  [OK] Filesystem loaded.\n\n");

    boot_animation();

    main_loop();

    fs_halt();
    return 0;
}
#pragma endregion
