#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int parent_to_child[2];  // 父进程到子进程的管道
    int child_to_parent[2];  // 子进程到父进程的管道
    char buf = 'a';          // 用于传递的字节

    // 创建两个管道
    if (pipe(parent_to_child) < 0 || pipe(child_to_parent) < 0) {
        fprintf(2, "Error: pipe creation failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "Error: fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        // 子进程
        close(parent_to_child[1]);  // 关闭父到子的写端
        close(child_to_parent[0]);  // 关闭子到父的读端
        
        // 从父进程读取数据
        if (read(parent_to_child[0], &buf, 1) != 1) {
            fprintf(2, "Error: child read failed\n");
            exit(1);
        }
        
        // 打印消息
        printf("%d: received ping\n", getpid());
        
        // 向父进程发送数据
        if (write(child_to_parent[1], &buf, 1) != 1) {
            fprintf(2, "Error: child write failed\n");
            exit(1);
        }
        
        // 关闭管道
        close(parent_to_child[0]);
        close(child_to_parent[1]);
        exit(0);
    } else {
        // 父进程
        close(parent_to_child[0]);  // 关闭父到子的读端
        close(child_to_parent[1]);  // 关闭子到父的写端
        
        // 向子进程发送数据
        if (write(parent_to_child[1], &buf, 1) != 1) {
            fprintf(2, "Error: parent write failed\n");
            exit(1);
        }
        
        // 从子进程读取数据
        if (read(child_to_parent[0], &buf, 1) != 1) {
            fprintf(2, "Error: parent read failed\n");
            exit(1);
        }
        
        // 打印消息
        printf("%d: received pong\n", getpid());
        
        // 关闭管道
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        
        // 等待子进程结束
        wait(0);
        exit(0);
    }
}