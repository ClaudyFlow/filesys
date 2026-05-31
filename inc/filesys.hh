#ifndef INC_FILESYS
#define INC_FILESYS

#pragma region include::standard
#include <cstdint>
#include <cstdio>
#pragma endregion include::standard

#define BLOCKSIZ    512 ///<块大小
#define SYSOPENFILE 40///<系统打开文件上限？
#define DIRNUM      128///<文件夹数量上限
#define DIRSIZ      14///<文件夹名字长度？
#define PWDSIZ      12///<密码长度
#define PWDNUM      32//密码数量上限
#define NOFILE      20//无文件？
#define NADDR       5   // x86-64四级页表: PML4+PDPT+PD+PTE+offset
#define NHINO       128//不认识
#define USERNUM     10//最大用户数
#define DINODESIZ   32//inode磁盘版本大小
#define DINODEBLK   32//inode块数
#define FILEBLK     512//文件块数？
#define NICFREE     50
#define NICINOD     50
#define DINODESTART 2*BLOCKSIZ
#define DATASTART   (2+DINODEBLK)*BLOCKSIZ
#define DIEMPTY     00000 //空指针
#define DIFILE      01000 //文件
#define DIDIR       02000 // 目录
#define UDIREAD     00001 // 读
#define UDIWRITE    00002 //写
#define UDIEXICUTE  00004 //执行，执行不是execute吗
#define GDIREAD     00010
#define GDIWRITE    00020
#define GDIEXECUTE  00040
#define ODIREAD     00100
#define ODIWRITE    00200
#define ODIEXECUTE  00400
#define READ        1
#define WRITE       2
#define EXECUTE     3
#define DEFAULTMODE 00777
#define IUPDATE     00002
#define SUPDATE     00001
#define FREAD       00001
#define FWRITE      00002
#define FAPPEND     00004
#define DISKFULL    65535
#define SEEK_SET    0

//==============================================================================
// inode 内存版本（含链表指针）
//==============================================================================
struct inode {
    struct inode *i_forw;
    struct inode *i_back;
    char  i_flag;
    uint32_t i_ino;
    uint32_t i_count;
    uint16_t di_number;
    uint16_t di_mode;
    uint16_t di_uid;
    uint16_t di_gid;
    uint16_t di_size;
    uint64_t di_addr;  // x86-64标准布局：pml4(9)+pdpt(9)+pd(9)+pte(9)+off(12)
};

//==============================================================================
// dinode 磁盘版本（精确 32 字节，强制定义）
//==============================================================================
struct dinode {
    uint16_t di_number;   // 2  硬链接计数
    uint16_t di_mode;     // 2  类型+权限
    uint16_t di_uid;      // 2  用户ID
    uint16_t di_gid;      // 2  组ID
    uint64_t di_size;     // 8  文件大小
    uint64_t di_addr;     // 8  x86-64逻辑地址（pml4/pdpt/pd/pte/off）
    uint32_t di_ctime;    // 4  创建时间
    uint32_t di_reserved; // 4  保留
};

struct direct {
    char d_name[14];    ///< UTF-8 自分割，无需长度字段
    uint32_t d_ino; ///< inode 号（0=空）
};

struct filsys {
    uint16_t s_isize;
    uint64_t s_fsize;
    uint32_t s_nfree;
    uint16_t s_pfree;
    uint32_t s_free[NICFREE];
    uint32_t s_ninode;
    uint16_t s_pinode;
    uint32_t s_inode[NICINOD];
    uint32_t s_pgd;     // PGD 基址块号（x86-64页表根）
    uint32_t s_rinode;
    char s_fmod;
};

///密码结构体
struct pwd {
    uint16_t p_uid;///<用户id
    uint16_t p_gid;///<用户组id
    char password[PWDSIZ];///<密码，字符数组，可以限制范围
};

struct dir {
    struct direct direct[DIRNUM];
    int32_t size;
};

struct hinode {
    struct inode *i_forw;
};
//文件结构体
struct file {
    char f_flag;// 文件状态
    uint32_t f_count; //文件数量
    struct inode *f_inode; // 文件节点
    uint64_t f_offset;  // 文件偏移量？
};

struct user {
    uint16_t u_default_mode;
    uint16_t u_uid;
    uint16_t u_gid;
    uint16_t u_ofile[NOFILE];
};

extern struct hinode hinode[NHINO];
extern struct dir dir;
extern struct file sys_ofile[SYSOPENFILE];
extern struct filsys filsys;
extern struct pwd pwd[PWDNUM];
extern struct user user[USERNUM];
extern        FILE *fd;
extern struct inode *cur_path_inode;
extern int32_t user_id;
extern struct inode *iget(uint32_t dinodeid);
extern void iput(struct inode *pinode);
extern uint32_t balloc(void);
extern uint32_t bfree(uint32_t block_num);
extern struct inode *ialloc(void);
extern void ifree(uint32_t dinodeid);
extern uint32_t namei(char *name);
extern uint16_t iname(char *name);
extern uint32_t file_access(uint32_t user_id, struct inode *inode, uint16_t mode);
extern void _dir(void);
extern void fs_mkdir(char *dirname);
extern void fs_chdir(char *dirname);
extern uint16_t aopen(uint16_t uid, char *filename, uint16_t openmode);
extern void fs_creat(uint32_t user_id, char *filename, uint16_t mode);
extern uint32_t fs_read(uint16_t cfd, uint32_t user_id, char *buf, uint32_t len);
extern uint32_t fs_write(uint16_t cfd, uint32_t user_id, char *buf, uint32_t len);
extern void fs_login(uint16_t uid, char *passwd);
extern void fs_logout(uint16_t uid);
extern void fs_install(void);
extern void fs_format(void);
extern void fs_close(uint32_t user_id, uint16_t cfd);
extern void fs_halt(void);
extern uint32_t fs_translate(uint32_t pgd_blk, uint64_t di_addr, uint64_t blk_num);
extern void fs_delete(char *filename);


#endif /* INC_FILESYS */
