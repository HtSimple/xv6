#include "kernel/types.h"
#include "user/user.h"

// 递归函数：从管道读取数据，筛选并创建新的筛子进程
void primes(int fd) {
    int p, n;
    int new_pipe[2];
    int pid;

    // 读取第一个数字（该数字一定是素数）
    if (read(fd, &p, sizeof(int)) != sizeof(int)) {
        close(fd);  // 没有数据，关闭管道并退出
        return;
    }

    // 输出当前素数
    printf("prime %d\n", p);

    // 创建新管道，用于传递未被当前素数整除的数字
    if (pipe(new_pipe) < 0) {
        fprintf(2, "Error: pipe creation failed\n");
        close(fd);
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        fprintf(2, "Error: fork failed\n");
        close(fd);
        close(new_pipe[0]);
        close(new_pipe[1]);
        exit(1);
    }

    if (pid == 0) {
        // 子进程：作为新的筛子，处理剩余数字
        close(fd);          // 关闭父进程传来的管道读端
        close(new_pipe[1]); // 关闭新管道的写端（子进程只需要读）
        primes(new_pipe[0]);// 递归处理新管道中的数据
        close(new_pipe[0]); // 处理完毕后关闭读端
        exit(0);
    } else {
        // 父进程：筛选出不能被当前素数整除的数字，传给子进程
        close(new_pipe[0]); // 关闭新管道的读端（父进程只需要写）

        // 读取剩余数字，筛选后写入新管道
        while (read(fd, &n, sizeof(int)) == sizeof(int)) {
            if (n % p != 0) {  // 只保留不能被当前素数整除的数字
                if (write(new_pipe[1], &n, sizeof(int)) != sizeof(int)) {
                    fprintf(2, "Error: write to new pipe failed\n");
                    close(fd);
                    close(new_pipe[1]);
                    exit(1);
                }
            }
        }

        // 关闭所有管道，等待子进程结束
        close(fd);
        close(new_pipe[1]);
        wait(0);  // 等待子进程处理完毕
    }
}

int main(int argc, char *argv[]) {
    int p[2];  // 初始管道：父进程（主进程）到第一个筛子进程
    int pid, i;

    // 创建初始管道
    if (pipe(p) < 0) {
        fprintf(2, "Error: initial pipe creation failed\n");
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        fprintf(2, "Error: fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        // 子进程：第一个筛子进程
        close(p[1]);  // 关闭管道写端（只需要读）
        primes(p[0]); // 开始递归筛选
        close(p[0]);  // 处理完毕后关闭读端
        exit(0);
    } else {
        // 父进程：生成2~35的数字，写入初始管道
        close(p[0]);  // 关闭管道读端（只需要写）

        // 写入初始数字序列
        for (i = 2; i <= 35; i++) {
            if (write(p[1], &i, sizeof(int)) != sizeof(int)) {
                fprintf(2, "Error: write to initial pipe failed\n");
                close(p[1]);
                exit(1);
            }
        }

        // 关闭写端，通知子进程数据传输完毕
        close(p[1]);

        // 等待整个筛选流程结束（所有子进程退出）
        wait(0);
        exit(0);
    }
}