#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"

const int BUF_SIZE = 512;

void find(const char *path, const char *const pattern) {
    char *buf = malloc(BUF_SIZE * sizeof(char));
    if (buf == 0) {
        fprintf(2, "find: malloc failed\n");
        return;
    }

    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, O_RDONLY)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        if (close(fd) < 0) {
            fprintf(2, "find: close failed\n");
            exit(1);
        }
        return;
    }

    switch (st.type) {
        case T_FILE:;
            const char *filename = path;
            char *del;
            while ((del = strchr(filename, '/')) != 0) {
                filename = del + 1;
            }
            if (strcmp(filename, pattern) == 0) {
                fprintf(1, "%s\n", path);
            }
            break;
        case T_DIR:
            strcpy(buf, path);
            char *p = buf + strlen(buf);
            *p = '/';
            p++;
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0) {
                    continue;
                }
                if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
                    continue;
                }
                if (strlen(path) + 1 + strlen(de.name) + 1 > BUF_SIZE) {
                    fprintf(2, "find: path too long\n");
                    break;
                }
                memmove(p, de.name, strlen(de.name) + 1);
                find(buf, pattern);
            }
            break;
    }

    if (close(fd) < 0) {
        fprintf(2, "find: close failed\n");
        exit(1);
    }
    free(buf);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(2, "Usage: find <base> <pattern>\n");
        exit(1);
    }

    find(argv[1], argv[2]);

    exit(0);
}
