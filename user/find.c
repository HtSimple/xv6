#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// 提取文件名（从路径中截取最后一个 '/' 后的部分）
char* fmtname(char *path) {
    static char buf[DIRSIZ + 1];
    char *p;

    // 从路径末尾开始查找 '/'
    for(p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;  // 指向文件名的第一个字符

    // 复制文件名（最多 DIRSIZ 个字符）
    if(strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), 0, DIRSIZ - strlen(p));
    return buf;
}

// 递归查找文件
void find(char *path, char *target) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    // 打开路径对应的文件/目录
    if((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // 获取文件状态信息
    if(fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // 根据文件类型处理
    switch(st.type) {
    case T_FILE:
        // 如果是文件，检查文件名是否匹配目标
        if(strcmp(fmtname(path), target) == 0)
            printf("%s\n", path);
        break;

    case T_DIR:
        // 如果是目录，检查路径长度是否超出限制
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
            printf("find: path too long: %s\n", path);
            close(fd);
            return;
        }

        // 构建完整路径（在原路径后添加 '/'）
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        // 读取目录中的所有条目
        while(read(fd, &de, sizeof(de)) == sizeof(de)) {
            // 跳过无效条目
            if(de.inum == 0)
                continue;

            // 跳过当前目录 (.) 和父目录 (..)
            if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;

            // 拼接子路径（原路径 + '/' + 条目名称）
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;  // 确保字符串结束

            // 递归查找子路径
            find(buf, target);
        }
        break;
    }

    close(fd);  // 关闭文件描述符，释放资源
}

int main(int argc, char *argv[]) {
    // 检查命令行参数（必须提供路径和目标文件名）
    if(argc != 3) {
        fprintf(2, "Usage: find <path> <target>\n");
        exit(1);
    }

    // 从指定路径开始查找目标文件
    find(argv[1], argv[2]);
    exit(0);
}