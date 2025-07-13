#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAXLINE 1024

int main(int argc, char *argv[]) {
    // 跳过 xargs 自身的选项（如 -n 1），直接找到命令名
    int cmd_start = 1;
    while (cmd_start < argc && strcmp(argv[cmd_start], "-n") == 0) {
        cmd_start += 2;  // 跳过 -n 和后面的数字参数
    }

    // 保存原命令及其参数
    char *cmd_args[MAXARG];
    int cmd_argc = argc - cmd_start;
    int i;

    // 复制原命令参数到 cmd_args
    for (i = 0; i < cmd_argc; i++) {
        cmd_args[i] = argv[cmd_start + i];
    }

    char line[MAXLINE];
    char *line_args[MAXARG];

    // 从标准输入读取行
    while (1) {
        char c;
        int len = 0;
        int eof = 0;

        // 读取一行（直到遇到换行符或EOF）
        while (1) {
            if (read(0, &c, 1) != 1) {
                eof = 1;
                break;
            }
            if (c == '\n') {
                break;
            }
            if (len < MAXLINE - 1) {
                line[len++] = c;
            }
        }
        line[len] = '\0';  // 字符串结尾

        // 如果读到EOF且没有数据，退出循环
        if (eof && len == 0) {
            break;
        }

        // 准备新的参数数组：原命令参数 + 新读取的行
        int line_argc = 0;
        
        // 复制原命令参数
        for (i = 0; i < cmd_argc; i++) {
            line_args[line_argc++] = cmd_args[i];
        }
        
        // 添加新读取的行作为最后一个参数
        line_args[line_argc++] = line;
        line_args[line_argc] = 0;  // NULL结尾

        // 创建子进程执行命令
        int pid = fork();
        if (pid < 0) {
            fprintf(2, "xargs: fork failed\n");
            exit(1);
        } else if (pid == 0) {
            // 子进程执行命令：手动构建完整路径（在命令名前加 "/"）
            char full_cmd[512];
            
            full_cmd[0] = '/';  // 命令路径以 "/" 开头
            
            // 复制命令名到路径中（从下标1开始）
            for (i = 0; cmd_args[0][i] != '\0' && i + 1 < sizeof(full_cmd) - 1; i++) {
                full_cmd[i + 1] = cmd_args[0][i];
            }
            full_cmd[i + 1] = '\0';  // 字符串结尾
            
            // 执行命令
            if (exec(full_cmd, line_args) < 0) {
                fprintf(2, "xargs: exec %s failed\n", full_cmd);
                exit(1);
            }
        } else {
            // 父进程等待子进程完成
            wait(0);
        }
    }

    exit(0);
}