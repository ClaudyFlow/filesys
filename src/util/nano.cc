#pragma region include::header
#include "util/nano.hh"
#pragma endregion include::header

#pragma region include::project
#include "filesys.hh"
#include "util/utf8.hh"
#pragma endregion include::project

#pragma region include::standard
#include <cstdlib>
#include <cstring>
#include <ctype.h>
// exclude <cstdio>
// exclude <cstdint>
#pragma endregion include::standard

#pragma region nano_buffer

static struct nano_buf *nano_create(void) {
    struct nano_buf *nb = (struct nano_buf *)calloc(1, sizeof(struct nano_buf));
    nb->lines[0] = (char *)calloc(1, 1);
    nb->count = 1;
    nb->cursor_row = 0;
    nb->cursor_col = 0;
    nb->view_row = 0;
    nb->dirty = 0;
    return nb;
}

static void nano_free(struct nano_buf *nb) {
    for (int i = 0; i < nb->count; i++)
        free(nb->lines[i]);
    free(nb);
}

static void nano_insert_char(struct nano_buf *nb, int row, int col, char ch) {
    if (row < 0 || row >= nb->count) return;
    int len = (int)strlen(nb->lines[row]);
    if (col < 0) col = 0;
    if (col > len) col = len;

    char *new_line = (char *)malloc(len + 2);
    memcpy(new_line, nb->lines[row], col);
    new_line[col] = ch;
    memcpy(new_line + col + 1, nb->lines[row] + col, len - col + 1);
    free(nb->lines[row]);
    nb->lines[row] = new_line;
    nb->dirty = 1;
}

static void nano_delete_char(struct nano_buf *nb, int row, int col) {
    if (row < 0 || row >= nb->count) return;
    int len = (int)strlen(nb->lines[row]);
    if (col < 0 || col >= len) return;

    char *new_line = (char *)malloc(len);
    memcpy(new_line, nb->lines[row], col);
    memcpy(new_line + col, nb->lines[row] + col + 1, len - col);
    free(nb->lines[row]);
    nb->lines[row] = new_line;
    nb->dirty = 1;
}

static void nano_insert_line(struct nano_buf *nb, int row) {
    if (nb->count >= NANO_MAX_LINES - 1) return;
    for (int i = nb->count; i > row; i--)
        nb->lines[i] = nb->lines[i - 1];
    nb->lines[row + 1] = (char *)calloc(1, 1);
    nb->count++;
    nb->dirty = 1;
}

static void nano_delete_line(struct nano_buf *nb, int row) {
    if (row < 0 || row >= nb->count || nb->count <= 1) return;
    free(nb->lines[row]);
    for (int i = row; i < nb->count - 1; i++)
        nb->lines[i] = nb->lines[i + 1];
    nb->count--;
    nb->dirty = 1;
}

static void nano_merge_line(struct nano_buf *nb, int row) {
    if (row < 0 || row >= nb->count - 1) return;
    int len1 = (int)strlen(nb->lines[row]);
    int len2 = (int)strlen(nb->lines[row + 1]);
    char *new_line = (char *)malloc(len1 + len2 + 1);
    memcpy(new_line, nb->lines[row], len1);
    memcpy(new_line + len1, nb->lines[row + 1], len2 + 1);
    free(nb->lines[row]);
    free(nb->lines[row + 1]);
    for (int i = row; i < nb->count - 2; i++)
        nb->lines[i] = nb->lines[i + 1];
    nb->lines[row] = new_line;
    nb->count--;
    nb->dirty = 1;
}

static int nano_load_file(struct nano_buf *nb, const char *filepath, uint16_t uid) {
    uint16_t cfd = aopen(uid, (char *)filepath, FREAD);
    if (!cfd) return -1;

    int total = (int)strlen(filepath);
    if (total >= NANO_MAX_PATH - 5) total = NANO_MAX_PATH - 5;
    memcpy(nb->filepath, filepath, total);
    nb->filepath[total] = '\0';

    char *buf = (char *)malloc(65536);
    uint32_t n = fs_read(cfd, user_id, buf, 65535);
    buf[n] = '\0';
    fs_close(uid, cfd);

    for (int i = 0; i < nb->count; i++) free(nb->lines[i]);
    nb->count = 0;

    char *p = buf;
    while (*p) {
        char *eol = strchr(p, '\n');
        if (!eol) eol = p + strlen(p);
        int len = (int)(eol - p);
        if (len > 0 && *(eol - 1) == '\r') len--;
        char *line = (char *)malloc(len + 1);
        memcpy(line, p, len);
        line[len] = '\0';
        if (nb->count < NANO_MAX_LINES)
            nb->lines[nb->count++] = line;
        else
            free(line);
        p = (*eol) ? eol + 1 : eol;
    }
    free(buf);
    if (nb->count == 0) {
        nb->lines[0] = (char *)calloc(1, 1);
        nb->count = 1;
    }
    nb->cursor_row = 0;
    nb->cursor_col = 0;
    nb->view_row = 0;
    nb->dirty = 0;
    return 0;
}

static int nano_save_file(struct nano_buf *nb, uint16_t uid) {
    if (!nb->filepath[0]) return -1;

    fs_delete(nb->filepath);
    fs_creat(uid, nb->filepath, DEFAULTMODE);
    uint16_t cfd = aopen(uid, nb->filepath, FWRITE);
    if (!cfd) return -1;

    for (int i = 0; i < nb->count; i++) {
        char tmp[8192];
        int len = (int)strlen(nb->lines[i]);
        if (len >= (int)sizeof(tmp) - 2) len = sizeof(tmp) - 3;
        memcpy(tmp, nb->lines[i], len);
        tmp[len] = '\n';
        fs_write(cfd, uid, tmp, len + 1);
    }
    fs_close(uid, cfd);
    nb->dirty = 0;
    return 0;
}

#pragma endregion

#pragma region nano_render

enum { CTRL_C = 3, CTRL_S = 19, CTRL_X = 24, CTRL_G = 7 };

static void nano_draw(struct nano_buf *nb) {
    printf("\x1b[2J\x1b[H");
    int h = 20;
    for (int i = 0; i < h; i++) {
        int r = nb->view_row + i;
        if (r >= nb->count) {
            printf("~\n");
            continue;
        }
        char *line = nb->lines[r];
        int len = (int)strlen(line);
        for (int j = 0; j < len; j++) {
            unsigned int cp;
            int blen;
            if (utf8_decode(line, j, &cp, &blen)) {
                if (cp < 0x80) printf("%c", (char)cp);
                else {
                    char tmp[5] = {0};
                    utf8_encode(tmp, cp, &blen);
                    printf("%s", tmp);
                }
                j += blen - 1;
            } else {
                printf("%c", line[j]);
            }
        }
        printf("\n");
    }
    int col = nb->cursor_col < 70 ? nb->cursor_col : 70;
    printf("\x1b[%d;%dH", nb->cursor_row - nb->view_row + 1, col + 1);
    fflush(stdout);
}

#pragma endregion

#pragma region nano_input

static void nano_cursor_left(struct nano_buf *nb) {
    if (nb->cursor_col > 0) {
        nb->cursor_col--;
    } else if (nb->cursor_row > 0) {
        nb->cursor_row--;
        nb->cursor_col = (int)strlen(nb->lines[nb->cursor_row]);
    }
}

static void nano_cursor_right(struct nano_buf *nb) {
    int len = (int)strlen(nb->lines[nb->cursor_row]);
    if (nb->cursor_col < len) {
        nb->cursor_col++;
    } else if (nb->cursor_row < nb->count - 1) {
        nb->cursor_row++;
        nb->cursor_col = 0;
    }
}

static void nano_cursor_up(struct nano_buf *nb) {
    if (nb->cursor_row > 0) {
        nb->cursor_row--;
        int len = (int)strlen(nb->lines[nb->cursor_row]);
        if (nb->cursor_col > len) nb->cursor_col = len;
        if (nb->cursor_row < nb->view_row) nb->view_row = nb->cursor_row;
    }
}

static void nano_cursor_down(struct nano_buf *nb) {
    if (nb->cursor_row < nb->count - 1) {
        nb->cursor_row++;
        int len = (int)strlen(nb->lines[nb->cursor_row]);
        if (nb->cursor_col > len) nb->cursor_col = len;
        if (nb->cursor_row >= nb->view_row + 20) nb->view_row = nb->cursor_row - 19;
    }
}

static void nano_backspace(struct nano_buf *nb) {
    if (nb->cursor_col > 0) {
        nano_delete_char(nb, nb->cursor_row, nb->cursor_col - 1);
        nb->cursor_col--;
    } else if (nb->cursor_row > 0) {
        nb->cursor_col = (int)strlen(nb->lines[nb->cursor_row - 1]);
        nano_merge_line(nb, nb->cursor_row - 1);
        nb->cursor_row--;
    }
}

static void nano_enter(struct nano_buf *nb) {
    nano_insert_line(nb, nb->cursor_row);
    nb->cursor_row++;
    nb->cursor_col = 0;
    if (nb->cursor_row >= nb->view_row + 20) nb->view_row++;
}

static void nano_home(struct nano_buf *nb) {
    nb->cursor_col = 0;
}

static void nano_end(struct nano_buf *nb) {
    nb->cursor_col = (int)strlen(nb->lines[nb->cursor_row]);
}

static void nano_page_up(struct nano_buf *nb) {
    nb->cursor_row -= 20;
    if (nb->cursor_row < 0) nb->cursor_row = 0;
    nb->view_row = nb->cursor_row;
    nb->cursor_col = 0;
}

static void nano_page_down(struct nano_buf *nb) {
    nb->cursor_row += 20;
    if (nb->cursor_row >= nb->count) nb->cursor_row = nb->count - 1;
    nb->view_row = nb->cursor_row;
    nb->cursor_col = 0;
}

static int nano_read_key(void) {
    int ch = getchar();
    if (ch == 0x1b) {
        int ch2 = getchar();
        if (ch2 == 0x5b) {
            int ch3 = getchar();
            switch (ch3) {
            case 0x41:
                return 0x141;
            case 0x42:
                return 0x142;
            case 0x43:
                return 0x143;
            case 0x44:
                return 0x144;
            case 0x48:
                return 0x148;
            case 0x46:
                return 0x146;
            case 0x35:
                return 0x135;
            case 0x36:
                return 0x136;
            default:
                return 0x100;
            }
        }
        return 0x1b;
    }
    return ch;
}

#pragma endregion

#pragma region nano_edit

static void nano_status(struct nano_buf *nb, const char *msg) {
    printf("\x1b[21;1H\x1b[K%s", msg);
    fflush(stdout);
}

bool nano_edit(const char *filepath, uint16_t uid) {
    struct nano_buf *nb = nano_create();

    if (filepath && *filepath) {
        if (nano_load_file(nb, filepath, uid) != 0) {
            nano_free(nb);
            return false;
        }
    }

    nano_draw(nb);
    nano_status(nb, "nano: Ctrl+S save, Ctrl+X exit, Ctrl+G help");

    while (1) {
        int key = nano_read_key();

        if (key == CTRL_C) {
            if (nb->dirty) {
                nano_status(nb, "nano: Modified. Ctrl+X to exit without saving?");
                int c = nano_read_key();
                if (c == CTRL_X) {
                    nano_free(nb);
                    return false;
                }
                nano_draw(nb);
            }
            continue;
        }

        if (key == CTRL_S) {
            if (nano_save_file(nb, uid) == 0)
                nano_status(nb, "nano: Saved.");
            else
                nano_status(nb, "nano: Save failed.");
            nano_draw(nb);
            continue;
        }

        if (key == CTRL_G) {
            nano_status(nb, "nano: ^G help  ^S save  ^X exit  ^C abort");
            nano_read_key();
            nano_draw(nb);
            continue;
        }

        if (key == CTRL_X) {
            if (nb->dirty) {
                nano_status(nb, "nano: Save before exit? (Y/n)");
                int c = getchar();
                if (c == 'Y' || c == 'y' || c == '\n')
                    nano_save_file(nb, uid);
            }
            break;
        }

        if (key == 0x141) {
            nano_cursor_up(nb);
            nano_draw(nb);
            continue;
        }
        if (key == 0x142) {
            nano_cursor_down(nb);
            nano_draw(nb);
            continue;
        }
        if (key == 0x143) {
            nano_cursor_right(nb);
            nano_draw(nb);
            continue;
        }
        if (key == 0x144) {
            nano_cursor_left(nb);
            nano_draw(nb);
            continue;
        }
        if (key == 0x148) {
            nano_home(nb);
            nano_draw(nb);
            continue;
        }
        if (key == 0x146) {
            nano_end(nb);
            nano_draw(nb);
            continue;
        }
        if (key == 0x135) {
            nano_page_up(nb);
            nano_draw(nb);
            continue;
        }
        if (key == 0x136) {
            nano_page_down(nb);
            nano_draw(nb);
            continue;
        }

        if (key == '\t') {
            nano_insert_char(nb, nb->cursor_row, nb->cursor_col, ' ');
            nb->cursor_col++;
            if (nb->cursor_col % 4 == 0) {
                nano_insert_char(nb, nb->cursor_row, nb->cursor_col, ' ');
                nb->cursor_col++;
            }
            nano_draw(nb);
            continue;
        }

        if (key == 0x7f || key == '\b') {
            nano_backspace(nb);
            nano_draw(nb);
            continue;
        }

        if (key == '\n' || key == '\r') {
            nano_enter(nb);
            nano_draw(nb);
            continue;
        }

        if (key >= 0x20 && key < 0x100) {
            nano_insert_char(nb, nb->cursor_row, nb->cursor_col, (char)key);
            nb->cursor_col++;
            nano_draw(nb);
            continue;
        }
    }

    nano_free(nb);
    printf("\x1b[2J\x1b[H");
    printf("nano: done.\n");
    return true;
}

#pragma endregion
