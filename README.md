# Nachos 文件系统工程文档

## 一、项目信息

- **路径**: `D:/Code/Project/filesysorig/`
- **语言**: C17
- **构建**: CMake + Ninja（已配置）
- **入口**: `bin/filesys.exe`
- **虚拟磁盘**: `filesystem`（运行时生成，模拟磁盘块设备）

## 二、磁盘布局

```
Block 大小: BLOCKSIZ = 512 字节

Block 0       : Boot 区（未用）
Block 1       : 超级块 (filsys 结构体)
Block 2~33    : inode 区 (DINODEBLK = 32)，每条目 DINODESIZ = 32 字节
Block 34~545  : 数据区 (FILEBLK = 512 块)

总块数 = 2 + DINODEBLK + FILEBLK = 546 块 ≈ 280KB
```

### 磁盘地址常量

| 常量        | 值                    | 含义               |
| ----------- | --------------------- | ------------------ |
| BLOCKSIZ    | 512                   | 块大小             |
| DINODESTART | BLOCKSIZ * 2 = 1024   | inode 区起始地址   |
| DATASTART   | BLOCKSIZ * 34 = 17408 | 数据区起始地址     |
| NADDR       | 6                     | inode 地址数组长度 |
| DINODESIZ   | 32                    | inode 大小         |
| DINODEBLK   | 32                    | inode 区块数       |
| FILEBLK     | 512                   | 数据区块数         |

## 三、数据结构

### 3.1 超级块 (struct filsys)

```c
struct filsys {
    unsigned short s_isize;    // inode 区大小（块数）
    unsigned short s_fsize;    // 数据区总块数
    unsigned short s_nfree;    // 空闲块数
    unsigned short s_free[NICFREE];  // 空闲块栈 (NICFREE=50)
    unsigned short s_ninode;   // 空闲 inode 数
    unsigned short s_inode[NICINOD]; // inode 栈 (NICINOD=50)
    unsigned short s_pfree;    // s_free 指针
    unsigned short s_pinode;   // s_inode 指针
    unsigned short s_rinode;   // 预留
    unsigned short s_fmod;     // 修改标志
};
```

### 3.2 inode (struct dinode) - 磁盘格式

```c
struct dinode {
    unsigned short di_number;  // 链接计数
    unsigned short di_mode;   // 文件模式（权限+类型）
    unsigned short di_uid;    // 用户 ID
    unsigned short di_gid;    // 组 ID
    unsigned long  di_size;   // 文件大小
    unsigned short di_addr[NADDR];  // 6 个块地址
};
```

### 3.3 inode (struct inode) - 内存格式（含链表指针）

```c
struct inode {
    unsigned short i_flag;    // 标志
    unsigned short i_ino;     // inode 编号
    unsigned short i_count;   // 引用计数
    unsigned long  i_size;    // 大小
    unsigned short i_mode;    // 模式
    unsigned short i_uid;     // UID
    unsigned short i_gid;     // GID
    unsigned short *i_addr;  // 块地址数组（需 malloc）
    unsigned short i_number;  // 链接数
    unsigned short di_addr[NADDR]; // 直接块地址
    struct inode *i_forw;     // 哈希链表前向
    struct inode *i_back;     // 哈希链表后向
};
```

### 3.4 目录项 (struct direct)

```c
struct direct {
    char d_name[DIRSIZ + 2];  // 文件名 (14 字节，含结束符)
    unsigned short d_ino;     // inode 编号
};
// DIRSIZ = 12
```

### 3.5 目录缓存 (struct dir)

```c
struct dir {
    int size;                  // 当前目录项数量
    struct direct direct[DIRNUM];  // 目录项数组 (DIRNUM=32)
};
```

### 3.6 用户文件表项 (struct user)

```c
struct user {
    unsigned short u_uid;      // 用户 ID
    unsigned short u_gid;      // 组 ID
    unsigned short u_default_mode; // 默认权限
    unsigned short u_ofile[NOFILE]; // 打开文件表 (NOFILE=20)
};
// USERNUM = 10
```

### 3.7 全局文件表项 (struct file)

```c
struct file {
    unsigned short f_flag;    // 文件标志
    unsigned int   f_offset;      // 文件偏移
    unsigned short f_count;   // 引用计数
    struct inode  *f_inode;   // inode 指针
};
// SYSOPENFILE = 40
```

### 3.8 inode 哈希表

```c
struct hinode {
    struct inode *i_forw;     // 哈希链表头
};
// NHINO = 128
```

### 3.9 密码表 (struct pwd)

```c
struct pwd {
    unsigned short p_uid;    // 用户 ID
    unsigned short p_gid;      // 组 ID
    char password[20];         // 密码
};
// PWDNUM = 40
```

## 四、文件列表

| 文件      | 功能                                     |
| --------- | ---------------------------------------- |
| main.c    | 入口，CLI 循环，开机动画，初始化         |
| filesys.h | 全部结构体和函数声明，含常量定义         |
| install.c | 初始化：读超级块、建哈希表、加载目录     |
| format.c  | 格式化磁盘，写超级块、inode 栈、数据块栈 |
| name.c    | namei() / iname() 目录名称解析           |
| open.c    | aopen() 文件打开（普通/追加/读）         |
| close.c   | close() 关闭文件，写回 inode             |
| creat.c   | creat() 创建文件                         |
| delete.c  | delete() 删除文件                        |
| write.c   | write() 写文件                           |
| read.c    | read() 读文件                            |
| log.c     | login() / logout() 用户登录登出          |
| dir.c     | _dir() / mkdir() / chdir() 目录操作      |
| igetput.c | iget() / iput() inode 读写和缓存管理     |
| bfree.c   | bfree() 释放数据块                       |
| alloc.c   | ialloc() / balloc() 分配 inode 和数据块  |
| halt.c    | halt() 关机（残留，含旧版菜单代码）      |

## 五、关键问题（需重写时注意）

### 5.1 namei() 返回值语义错误

**问题**：`namei(char *name)` 返回的是**目录项数组索引**（0~DIRNUM-1），不是 inode 编号。

**影响**：所有调用方（creat/open/delete/dir）把返回值直接传给 `iget()`，而 `iget()` 期望 inode 编号。

**修复方案**：改 `namei()` 返回 inode 编号，或者在调用方加一层转换。

### 5.2 creat() 的 fd 分配逻辑有漏洞

**问题**：
1. `namei()` 查不到返回 0，但 entry 0 实际是 `".."`（有效）
2. 找不到空闲 `sys_ofile[]`（i==SYSOPENFILE）时不报错
3. `user[].u_ofile[]` 找不到空闲项时直接返回 -1，没有错误提示
4. 用户文件描述符（返回值 j）和全局文件描述符（i）混淆

### 5.3 login() 函数 bug

**原 bug**：`if((uid==pwd[i].p_uid)&&(strcmp(passwd,pwd[i].password)))`
- `strcmp()` 返回 0 表示匹配，但条件里没取反 → 密码错误才登录成功

**修复**：已改为 `!strcmp()`，但 `pwd[]` 数组在 `install()` 中从未从磁盘加载，只在创建文件系统时写入。

### 5.4 install() 不加载 pwd[]

`install()` 只读超级块和目录，不读密码文件。密码表 `pwd[]` 的初始化必须由调用方在 main.c 中完成：

```c
FILE *pf = fopen("filesystem", "r+b");
fseek(pf, DATASTART + 2 * BLOCKSIZ, SEEK_SET);
fread(pwd, 1, sizeof(pwd), pf);
fclose(pf);
```

### 5.5 iget() 的链表插入 bug

**问题**：首次分配 inode 时，`newinode->i_forw->i_back = newinode;` 会解引用 NULL 指针。

**修复**：插入前判空。

### 5.6 文件权限检查 access() 实现错误

`access()` 直接用 `mode & ACCBITS` 比较，没有按用户/组/其他逐级检查。

### 5.7 _dir() 的输出格式混乱

`_dir()` 打印大量调试字符（`DIRSIZsxxxxxxxxx` 等），且混入了 `dir.direct` 的原始内容作为格式化字符串。

### 5.8 write() 的块号计算错误

`write()` 用 `cur_path_inode->di_addr[fd]` 作为块号，但没有加上 DATASTART 偏移。

## 六、用户命令（已实现）

```
login <uid> <password>   登录（如 login 2118 abcd）
logout                   登出
dir                      显示当前目录
mkdir <name>             创建子目录
chdir <name>             进入目录
create <name> [mode]     创建文件（默认 01777）
write <name> <text>      写文件（追加模式）
read <name>              读文件
delete <name>            删除文件
format                   重新格式化磁盘
user                     显示当前用户信息
help                     帮助
exit                     退出
```

## 七、测试用户

| UID  | GID | 密码 |
| ---- | --- | ---- |
| 2116 | 03  | dddd |
| 2117 | 03  | bbbb |
| 2118 | 04  | abcd |
| 2119 | 04  | cccc |
| 2220 | 05  | eeee |

## 八、重写建议

1. **统一 inode 编号概念**：`namei()` 直接返回 inode 编号，不要返回目录项索引
2. **分离 fd 和 cfd**：明确区分全局文件表索引和用户文件描述符
3. **pwd[] 在 install() 中加载**：不要让调用方负责
4. **权限检查重写**：`access()` 按 Unix 方式逐级检查 rwx
5. **输出清理**：去掉 `_dir()` 里的调试字符
6. **块地址计算统一**：所有 fseek/fread/fwrite 都用 DATASTART + 块号 × BLOCKSIZ
7. **关闭所有 FILE***：format() 创建文件后要 fclose()


























------
文件系统要有文件，创建文件自动检查是否为空
如果为空，调用dir，创建目录
所以cat要调用file和dir
为了减少耦合在头文件里不能调用
所以还是要在源文件里写
然后解决控制参数的问题

63-48   |47-39|38-30|29-21|20-12|11-0|
16位符号|偏移  |偏移  |偏移  |偏移  |页内偏移|

文件格式 utf8
0
110 10
1110 10 10
11110 10 10 10

utf8功能正常

今天需要实现文件索引和utf8储存即可

小端序系统？


## 九、编码约定

### 9.1 全部数据使用 UTF-8

文件系统内所有字符串（文件名、目录名、路径名、文本文件内容）均使用 UTF-8 编码。

**UTF-8 格式（自分割哈夫曼编码）：**

| 字符字节数 | 首字节格式       | 续字节格式       | 示例                    |
| --------- | ---------------- | ---------------- | ---------------------- |
| 1 字节     | 0xxxxxxx         | —                | 'A' = 0x41             |
| 2 字节     | 110xxxxx         | 10xxxxxx         | '中' = E4 B8 AD         |
| 3 字节     | 1110xxxx         | 10xxxxxx ×2      | '文' = E6 96 87         |
| 4 字节     | 11110xxx         | 10xxxxxx ×3      | (增补平面字符)          |

**特点：**
- 首字节连续 1 的个数决定字符字节数，从后往前扫描即可定位上一字符起始位置
- 不需要长度字段，不需要   终止符（文件名字段固定 255 字节时用   终止）
- UTF-8 是字节流编码，无端序问题（区别于 UTF-16/UTF-32）

**struct direct（目录条目）：**

```c
struct direct {
    char d_name[255];      // UTF-8 自分割，\0 终止
    unsigned int d_ino;    // 小端 inode 号
};
```

**文本文件格式（STX/ETX 边界）：**

```
[STX: 0x02] [UTF-8 内容字节流] [ETX: 0x03]
```

- 0x02 = STX (Start of Text)，标识文本开始
- 0x03 = ETX (End of Text)，标识文本结束
- 文件大小由 inode 的 di_size 记录，不依赖边界标记

### 9.2 整数端序：小端序

所有多字节整数（inode 号、块号、文件大小、地址等）统一使用小端序（Little Endian）。

**原因：** x86-64 CPU 内存布局即是小端，写入磁盘后 CPU 读取无需字节交换操作。

```c
// 示例：写 4 字节整数到磁盘（小端）
void write_le32(unsigned char *buf, unsigned int val) {
    buf[0] = (unsigned char)(val & 0xFF);
    buf[1] = (unsigned char)((val >> 8) & 0xFF);
    buf[2] = (unsigned char)((val >> 16) & 0xFF);
    buf[3] = (unsigned char)((val >> 24) & 0xFF);
}

// 读（直接使用，无需转换）
unsigned int read_le32(unsigned char *buf) {
    return (unsigned int)buf[0]
         | ((unsigned int)buf[1] << 8)
         | ((unsigned int)buf[2] << 16)
         | ((unsigned int)buf[3] << 24);
}
```

**受影响的整数字段（全部小端）：**

| 结构体       | 字段                    | 字节数 |
| ----------- | ---------------------- | ------ |
| dinode      | di_number, di_mode      | 2      |
| dinode      | di_uid, di_gid          | 2      |
| dinode      | di_size                 | 4      |
| dinode      | di_addr[]               | 4×10   |
| direct      | d_ino                   | 4      |
| filsys      | s_isize                 | 2      |
| filsys      | s_fsize                 | 4      |
| filsys      | s_nfree, s_pfree        | 2      |
| filsys      | s_free[]                | 4×50   |
| filsys      | s_ninode, s_pinode      | 2      |
| filsys      | s_inode[]               | 4×50   |

## 十、文件索引结构

### 10.1 64 位索引结构（SuperBlock）

SuperBlock 使用 64 位字段记录磁盘总块数，支持超大虚拟磁盘：

```c
struct filsys {
    unsigned short s_isize;    // inode 区大小（块数）
    unsigned long  s_fsize;   // 磁盘总块数（64 位，小端）
    unsigned int   s_nfree;
    unsigned short s_pfree;
    unsigned int   s_free[NICFREE];   // 空闲块栈
    unsigned int   s_ninode;
    unsigned short s_pinode;
    unsigned int   s_inode[NICINOD]; // 空闲 inode 栈
    unsigned int   s_rinode;
    char           s_fmod;
};
```

### 10.2 inode 块号寻址（16 + 9 + 9 + 9 + 9 + 12 = 64 位）

inode 使用 10 + 10 + 12 个块号条目（di_addr[] 共 32 项），最大寻址范围 48 位：

```
di_addr[0~9]    直接块           → 10 × 512 B = 5,120 B
di_addr[10~19]  一次间接块（10个间接块，每个256个块号）
                                   → 10 × 256 × 512 B ≈ 1.25 MB
di_addr[20~31]  二次间接块（12个间接块，每个256²个块号）
                                   → 12 × 256² × 512 B ≈ 32 GB
```

**位分布（64 位虚拟地址组成）：**

| 段         | 位范围     | 位数 | 说明              |
| --------- | ---------- | ---- | ---------------- |
| SIG       | [63:48]    | 16   | 16位符号位        |
| PML4      | [47:39]    | 9    | 页映射级4         |
| PDPT      | [38:30]    | 9    | 页目录指针表      |
| PD        | [29:21]    | 9    | 页目录            |
| PT        | [20:12]    | 9    | 页表              |
| Offset    | [11:0]     | 12   | 页内偏移（4KB）   |

**inode 块号与 CPU 分页无关**，是文件系统自身的块号编码方案。

### 10.3 磁盘布局（最终版）

```
Block 0       : Boot 区（未用）
Block 1       : SuperBlock (filsys)
Block 2~33    : inode 区 (32 块，每块 16 个 dinode)
Block 34~     : 数据区（目录块 + 文件数据块）

每个 dinode 大小 = 32 字节
每个数据块大小 = 512 字节
```

short 2 16 65535
int 4 32 4,294,967,295
long 8 64 18446744073709551615

16 9(512iB) 9(512iB) 9(512iB) 9(512iB) 12(4KiB)

1110 0110 1000 1000 1001 0001
0110 0010 0001 0001
6211
U+6211
