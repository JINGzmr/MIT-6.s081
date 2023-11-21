#include "kernel/types.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

char *fmtname(char *path) {
    char *p;

    // 从路径的末尾开始寻找最后一个斜杠
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;  // 移动到最后一个斜杠之后的字符

    return p;  // 返回文件名
}

void find(char *path, char *file_name) {
    // 定义缓冲区和其他变量
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    // 尝试打开指定路径的目录
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // 获取打开目录的文件信息
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // 根据文件类型进行处理
    switch (st.type) {
        // 如果是文件
        case T_FILE:
            char *file_name_now = fmtname(path);
            // 检查文件名是否与搜索目标匹配
            if (!strcmp(file_name, file_name_now)) printf("%s\n", path);
            break;

        // 如果是目录
        case T_DIR:
            // 检查路径长度是否超过缓冲区大小
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                printf("ls: path too long\n");
                break;
            }
            // 构建新的路径
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';
            // 遍历目录中的所有条目
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                // 忽略"."和".."这两个特殊目录
                if (de.inum == 0 || strcmp(".", de.name) == 0 ||
                    strcmp("..", de.name) == 0)
                    continue;
                // 构建完整的文件路径
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                // 获取文件或目录的信息
                if (stat(buf, &st) < 0) {
                    printf("ls: cannot stat %s\n", buf);
                    continue;
                }
                // 递归调用find函数
                find(buf, file_name);
            }
            break;
    }
    // 关闭文件描述符
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {  // 以指定形式输入参数：find+目录+目标文件名
        printf("Parameter invalid! Usage: find <directory> <filename>\n");
        exit(1);
    }

    find(argv[1], argv[2]);
    exit(0);
}
