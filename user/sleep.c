#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    // 检查命令行参数数量是否正确
    if (argc != 2) {
        fprintf(2, "Usage: sleep ticks\n");
        exit(1);
    }

    // 将参数转换为整数
    int ticks = atoi(argv[1]); 
    // 检查参数是否有效
    if (ticks < 0) {
        fprintf(2, "Error: ticks must be a non-negative integer\n");
        exit(1);
    }
    //实际使用发现sleep 10 为1s，此处ticks*10为了便于用户理解使用
    sleep(ticks*10);
    exit(0);
}