/* halt.cc — 关机 */

#pragma region include::header
#include "halt.hh"
#pragma endregion include::header

#pragma region include::project
#include "filesys.hh"
#pragma endregion include::project

#pragma region include::standard
#include <cstdlib>
#pragma endregion include::standard

void fs_halt(void) {
    int32_t i, j;
    (void)i;
    (void)j;
    fs_chdir((char *)"..");
    iput(cur_path_inode);
    for (i = 0; i < USERNUM; i++) {
        if (user[i].u_uid != 0) {
            for (j = 0; j < NOFILE; j++) {
                if (user[i].u_ofile[j] != SYSOPENFILE + 1) {
                    fs_close(user[i].u_uid, user[i].u_ofile[j]);
                    user[i].u_ofile[j] = SYSOPENFILE + 1;
                }
            }
        }
    }
    fseek(fd, BLOCKSIZ, SEEK_SET);
    fwrite(&filsys, 1, sizeof(struct filsys), fd);
    fclose(fd);
    printf("\nGood Bye. See You Next Time. Please turn off the switch\n");
    exit(0);
}
