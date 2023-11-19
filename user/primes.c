#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define MAX_NUM 35

void Run_Child(int divisor,
               int* left_pipefd)  // 子进程递归执行代码 ，参数为该除数
{
    int num = 0, i = 0;
    int nums[MAX_NUM] = {0};
    int right_pipefd
        [2];  // 该管道为当前进程从父进程管道读取到的数据并处理后，写给当前进程的子进程的管道
    pipe(right_pipefd);
    close(left_pipefd[1]);

    while (read(left_pipefd[0], &num,
                sizeof(num))) {  // 只要返回值不为0，则说明写入端还有东西没写完
        if (num == divisor) {  // 该数和除数相同，说明是素数
            printf("prime %d\n", num);
        } else if (num % divisor !=
                   0) {  // 如果被整除了，则直接淘汰，不进入下一个管道
            nums[i++] = num;
        }
    }
    close(left_pipefd[0]);

    if (num !=
        divisor) {  // 如果num最后的值还是和除数相同，说明此时已经将所有的数处理完毕
        int pid = fork();

        if (pid < 0) {
            printf("Fork failed");
            exit(1);
        } else if (pid == 0) {  // 子进程
            Run_Child(divisor + 1, right_pipefd);
        } else {  // 父进程
            i = 0;
            close(right_pipefd[0]);
            while (nums[i] != 0) {
                write(right_pipefd[1], nums + i, sizeof(num));
                i++;
            }
            close(right_pipefd[1]);
            wait((int*)0);
        }
    }
    exit(0);
}

int main(int argc, int** argv) {
    int pipefd[2];
    pipe(pipefd);
    int pid = fork();

    if (pid < 0) {
        printf("Fork failed");
        exit(1);
    } else if (pid == 0) {  // 子进程
        Run_Child(2, pipefd);
    } else {  // 父进程
        close(pipefd[0]);
        for (int num = 2; num < MAX_NUM; num++) {
            write(pipefd[1], &num, sizeof(num));
        }
        close(pipefd[1]);
        // 关闭写入端，此时子进程的read在读取完管道内的所有数据后，其返回值为0
        // 如果不关闭，且管道没有数据的话，阻塞模式下read会阻塞住，不返回东西；非阻塞下，返回-1
        wait((int*)0);
    }
    exit(0);
}