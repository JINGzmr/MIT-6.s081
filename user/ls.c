#include "kernel/types.h"  // 包含内核定义的类型
#include "kernel/fs.h"     // 包含文件系统相关的头文件
#include "kernel/stat.h"   // 包含获取文件状态的结构体定义
#include "user/user.h"     // 包含用户级函数的头文件

// 函数定义：格式化文件名
char *fmtname(char *path) {
    static char buf[DIRSIZ + 1];  // 静态字符数组，用于存储格式化后的文件名
    char *p;

    // 从路径的末尾开始寻找最后一个斜杠
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;  // 移动到最后一个斜杠之后的字符

    // 如果文件名长度大于等于DIRSIZ，则直接返回
    if (strlen(p) >= DIRSIZ) return p;
    memmove(buf, p, strlen(p));  // 将文件名移动到缓冲区
    memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));  // 用空格填充剩余部分
    return buf;  // 返回格式化后的文件名
}

// ls命令实现
void ls(char *path) {
    char buf[512], *p;  // 缓冲区和指针
    int fd;             // 文件描述符
    struct dirent de;   // 目录项结构体
    struct stat st;     // 文件状态结构体

    // 尝试打开目录，失败则输出错误信息
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }

    // 获取文件状态，失败则输出错误信息
    if (fstat(fd, &st) < 0) {
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // 根据文件类型处理
    switch (st.type) {
        case T_FILE:  // 如果是普通文件
            printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
            break;

        case T_DIR:  // 如果是目录
            // 检查路径长度是否过长
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                // 如果路径太长，打印错误信息并中断
                printf("ls: path too long\n");
                break;
            }
            strcpy(buf, path);      // 将路径复制到缓冲区
            p = buf + strlen(buf);  // 设置指针p指向缓冲区末尾
            *p++ = '/';  // 在路径末尾添加斜杠，并将指针向前移动一位

            // 循环读取目录项
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0)
                    continue;  // 如果目录项无效（inode号为0），则跳过

                memmove(p, de.name, DIRSIZ);  // 将目录项的名字复制到p指向的位置
                p[DIRSIZ] = 0;  // 确保字符串以空字符结尾

                // 获取目录项的状态信息
                if (stat(buf, &st) < 0) {
                    printf("ls: cannot stat %s\n",
                           buf);  // 如果获取失败，打印错误信息并继续
                    continue;
                }
                // 打印格式化后的文件名，文件类型，inode号和文件大小
                printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
            }
            break;
    }
    close(fd);  // 关闭目录的文件描述符
}

// main函数
int main(int argc, char *argv[]) {
    int i;

    // 如果没有指定路径，就列出当前目录
    if (argc < 2) {
        ls(".");
        exit(0);
    }
    // 遍历所有指定的路径
    for (i = 1; i < argc; i++) ls(argv[i]);
    exit(0);
}
