#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

const int BUF_SIZE = 512;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "Usage: xargs <command>\n");
        exit(1);
    }

    const char *cmd = argv[1];
    int xargc = argc - 1;
    char *xargv[MAXARG];
    memcpy(xargv, argv + 1, xargc * sizeof(char *));

    int parsed = 0;
    char buff[BUF_SIZE];
    char *start = buff;
    char *end = buff;
    char c;

    while (read(0, &c, 1) > 0) {
        *end = c;
        if (c == '0' || c == '\n') {
            *end = 0;

            if (xargc + parsed + 1 >= MAXARG) {
                fprintf(2, "xargs: too many arguments\n");
                exit(1);
            }

            xargv[xargc + parsed] = start;
            xargv[xargc + parsed + 1] = 0;
            parsed++;
            start = end + 1;
        }
        if (c == '\n') {
            int pid = fork();
            if (pid < 0) {
                fprintf(2, "xargs: fork failed\n");
                exit(1);
            }
            if (pid == 0) {
                // Child
                if (exec(cmd, xargv) < 0) {
                    fprintf(2, "xargs: exec failed\n");
                    exit(1);
                }
                // Unreachable
                exit(2);
            }
            // Parent
            wait(0);
            start = buff;
            end = buff;
            parsed = 0;
            xargv[xargc] = 0;
        }

        end++;
        if (end >= buff + BUF_SIZE) {
            fprintf(2, "xargs: line too long\n");
            exit(1);
        }
    }

    exit(0);
}