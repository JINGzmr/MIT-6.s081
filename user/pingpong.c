#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"

// #include <sys/types.h> //加入头文件来消除代码中的举报 是不符合规范的！

int main(int argc, int*argv[]) {
    // pid_t pid; pit_t类型需要上面的头文件才不会报错，因此，用int来代替
    int p[2][2];
    pipe(p[0]);  // 子进程to父进程
    pipe(p[1]);  // 父进程to子进程
    int pid;
    pid = fork();

    if (pid < 0) {
        printf("Fork failed");
        exit(1);  // 参数为1，表示非正常运行导致的退出程序
    }

    if (pid == 0) {  // 子进程
        write(p[0][1], "a", 1);
        char buf[1];
        sleep(1);
        read(p[1][0], buf, 1);
        if (!strcmp(buf,"b")) {
            printf("%d: received ping\n",pid);
        }
    } else {  // 父进程
        write(p[1][1], "b", 1);
        char buf[1];
        sleep(2);
        read(p[0][0],buf,1);
        if (!strcmp(buf,"a")) {
            printf("%d: received pong\n",getpid());
        }
    }
    exit(0);
}
